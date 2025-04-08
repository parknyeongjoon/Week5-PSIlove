Texture2D PositionBuffer : register(t0);
Texture2D NormalBuffer : register(t1);
Texture2D DiffuseBuffer : register(t2);
Texture2D MaterialBuffer : register(t3);
SamplerState SamLinear : register(s0);

// 먼저 FLighting 구조체를 정의해야 합니다
struct FLighting
{
    float3 Position; 
    float Intensity;
    float3 LightDirection;
    float AmbientFactor;
    float4 LightColor;
    float AttenuationRadius;
    float3 pad0;
};

cbuffer LightBuffer : register(b0) 
{
    FLighting Lights[100];
    float3 EyePosition;  // xyz: 카메라 위치, w: 패딩 또는 추가 정보
    float LightCount;
}

cbuffer UVBuffer : register(b1)
{
    float UOffset;
    float VOffset;
    float UTiles;
    float VTiles;
}

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET0;
};

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    // G-Buffer에서 데이터 샘플링
    float2 texCoord = float2(UOffset + input.texCoord.x * UTiles, VOffset + input.texCoord.y * VTiles);
    float4 position = PositionBuffer.Sample(SamLinear, texCoord);
    float4 normal = NormalBuffer.Sample(SamLinear, texCoord);
    float4 diffuse = DiffuseBuffer.Sample(SamLinear, texCoord);
    float4 material = MaterialBuffer.Sample(SamLinear, texCoord);

     // 유효한 픽셀인지 확인 (깊이 값이 0이면 스킵)
    if (position.w == 0.0f)
    {
        discard;
    }

    // 재질 속성 추출
    float roughness = material.r;
    float metallic = material.g;
    float specularIntensity = material.b;
    
    // 법선 벡터 정규화
    float3 N = normal.xyz;
    N = (N - 0.5) * 2;
    
    // 시점 방향 계산
    float3 worldPos = position.xyz;
    float3 V = normalize(EyePosition.xyz - worldPos);
    
    // 최종 색상 초기화
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    float3 ambientColor = float3(0.0f, 0.0f, 0.0f);
    
    // 각 광원에 대한 라이팅 계산
    for (uint i = 0; i < LightCount; i++)
    {
        // 주변광(Ambient) 계산
        //ambientColor += diffuse.rgb * Lights[i].LightColor * Lights[i].AmbientFactor;
        
        // 광원 방향 계산
        float3 lightVector = Lights[i].Position-worldPos;
        float distance = length(lightVector);
        float3 L = normalize(lightVector);
        
        // 감쇠(Attenuation) 계산
        float attenuation = 1.0f;
        if (Lights[i].AttenuationRadius > 0.0f)
        {
            float distanceSquared = distance * distance;
            float radiusSquared = Lights[i].AttenuationRadius * Lights[i].AttenuationRadius;
        
            // 선형-제곱 감쇠 조합 (보다 현실적인 빛 감쇠 표현)
            float att = 1.0f / (1.0f + 0.1f * distance + 0.01f * distanceSquared);
        
            // 감쇠 반경에 따른 클램핑
            attenuation = att * saturate(1.0f - (distanceSquared / radiusSquared));
        }
        
        // 디퓨즈(Diffuse) 계산 - Lambert 모델
        float NdotL = max(dot(N, L), 0.0f);
        float3 diffuseTerm = diffuse.rgb * Lights[i].LightColor * NdotL * Lights[i].Intensity * attenuation;
        
        // 스페큘러(Specular) 계산 - Blinn-Phong 모델
        float3 H = normalize(V + L);
        float NdotH = max(dot(N, H), 0.0f);
        
        // 거칠기에 따른 스페큘러 파워 계산
        float specPower = max(1.0f, 128.0f * (1.0f - roughness));
        
        // 스페큘러 계수 계산
        float3 specularColor = lerp(float3(0.04f, 0.04f, 0.04f), diffuse.rgb, metallic);
        float3 specularTerm = specularColor * specularIntensity * pow(NdotH, specPower) * Lights[i].Intensity * attenuation;
        
        // 최종 색상에 더하기
        finalColor += diffuseTerm + specularTerm;
    }
    
    // 주변광 더하기
    finalColor += ambientColor;
    
    finalColor += diffuse.rgb * 0.1f;
    
    // HDR 톤 매핑 (간단한 Reinhard 톤 매핑)
    finalColor = finalColor / (finalColor + 1.0f);
    
    // 감마 보정 (sRGB 공간으로 변환)
    finalColor = pow(finalColor, float3(1.0f/2.2f, 1.0f/2.2f, 1.0f/2.2f));
    
    output.color = float4(finalColor, 1.0f);
    
    return output;
}