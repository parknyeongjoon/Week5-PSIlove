#include "TextRenderComponent.h"

#include "PropertyEditor/ShowFlags.h"

UTextRenderComponent::UTextRenderComponent()
{
    SetType(StaticClass()->GetName());
}

UTextRenderComponent::~UTextRenderComponent()
{
}

void UTextRenderComponent::InitializeComponent()
{
    UPrimitiveComponent::InitializeComponent();
}

void UTextRenderComponent::TickComponent(float DeltaTime)
{
    UPrimitiveComponent::TickComponent(DeltaTime);
}

void UTextRenderComponent::ClearText()
{
    vertexTextureArr.Empty();
}

void UTextRenderComponent::SetText(FWString _text)
{
    text = _text;
	if (_text.empty())
	{
		Console::GetInstance().AddLog(LogLevel::Warning, "Text is empty");

		vertexTextureArr.Empty();
		quad.Empty();
	    
		return;
	}

	const uint32 BitmapWidth = Texture->width;
	const uint32 BitmapHeight = Texture->height;

	const float CellWidth =  static_cast<float>(BitmapWidth) / static_cast<float>(ColumnCount);
	const float CellHeight = static_cast<float>(BitmapHeight)/ static_cast<float>(RowCount);

	const float nTexelUOffset = CellWidth / static_cast<float>(BitmapWidth) ;
	const float nTexelVOffset = CellHeight/ static_cast<float>(BitmapWidth) ;

	for (int i = 0; i < _text.size(); i++)
	{
		FVertexTexture leftUP = { -1.0f,1.0f,0.0f,0.0f,0.0f };
		FVertexTexture rightUP = { 1.0f,1.0f,0.0f,1.0f,0.0f };
		FVertexTexture leftDown = { -1.0f,-1.0f,0.0f,0.0f,1.0f };
		FVertexTexture rightDown = { 1.0f,-1.0f,0.0f,1.0f,1.0f };
		rightUP.u *= nTexelUOffset;
		leftDown.v *= nTexelVOffset;
		rightDown.u *= nTexelUOffset;
		rightDown.v *= nTexelVOffset;

		leftUP.x += quadWidth * i;
		rightUP.x += quadWidth * i;
		leftDown.x += quadWidth * i;
		rightDown.x += quadWidth * i;

		float startU = 0.0f;
		float startV = 0.0f;

		setStartUV(_text[i], startU, startV);
		leftUP.u += (nTexelUOffset * startU);
		leftUP.v += (nTexelVOffset * startV);
		rightUP.u += (nTexelUOffset * startU);
		rightUP.v += (nTexelVOffset * startV);
		leftDown.u += (nTexelUOffset * startU);
		leftDown.v += (nTexelVOffset * startV);
		rightDown.u += (nTexelUOffset * startU);
		rightDown.v += (nTexelVOffset * startV);

		vertexTextureArr.Add(leftUP);
		vertexTextureArr.Add(rightUP);
		vertexTextureArr.Add(leftDown);
		vertexTextureArr.Add(rightUP);
		vertexTextureArr.Add(rightDown);
		vertexTextureArr.Add(leftDown);
	}

	const float lastX = -1.0f + quadSize * _text.size();
	quad.Add(FVector(-1.0f,1.0f,0.0f));
	quad.Add(FVector(-1.0f,-1.0f,0.0f));
	quad.Add(FVector(lastX,1.0f,0.0f));
	quad.Add(FVector(lastX,-1.0f,0.0f));
    
    TArray<uint32> indices;
    const uint32 numLetters = static_cast<uint32>(_text.size());
    for (uint32 letter = 0; letter < numLetters; letter++)
    {
        uint32 baseIndex = letter * 6;
        // 첫 번째 삼각형
        indices.Add(baseIndex);
        indices.Add(baseIndex + 1);
        indices.Add(baseIndex + 2);
        // 두 번째 삼각형
        indices.Add(baseIndex + 3);
        indices.Add(baseIndex + 4);
        indices.Add(baseIndex + 5);
    }

    ID3D11Buffer* vertexBuffer = FEngineLoop::renderer.CreateImmutableVertexBuffer(vertexTextureArr);
    ID3D11Buffer* indexBuffer = FEngineLoop::renderer.CreateIndexBuffer(indices);
    
    FEngineLoop::renderer.AddOrSetVertexBuffer(Texture->Name, vertexBuffer, sizeof(FVertexTexture));
    FEngineLoop::renderer.AddOrSetIndexBuffer(Texture->Name, indexBuffer, sizeof(uint32));
}

void UTextRenderComponent::SetRowColumnCount(int32 InRowCount, int32 InColumnCount)
{
    RowCount = InRowCount;
    ColumnCount = InColumnCount;
}

int UTextRenderComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UTextRenderComponent::SetTexture(FWString _fileName)
{
    Texture = FEngineLoop::resourceMgr.GetTexture(_fileName);
}

void UTextRenderComponent::setStartUV(char alphabet, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU=0;
    int StartV=0;
    int offset = -1;


    if (alphabet == ' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (alphabet >= 'A' && alphabet <= 'Z') {

        StartU = 1;
        StartV = 4;
        offset = alphabet - 'A'; // 대문자 위치
    }
    else if (alphabet >= 'a' && alphabet <= 'z') {
        StartU = 1;
        StartV = 6;
        offset = (alphabet - 'a'); // 소문자는 대문자 다음 위치
    }
    else if (alphabet >= '0' && alphabet <= '9') {
        StartU = 0;
        StartV = 3;
        offset = (alphabet - '0'); // 숫자는 소문자 다음 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    int offsetV = (offset + StartU) / ColumnCount;
    int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}

void UTextRenderComponent::setStartUV(wchar_t hangul, float& outStartU, float& outStartV)
{
    //대문자만 받는중
    int StartU = 0;
    int StartV = 0;
    int offset = -1;

    if (hangul == L' ') {
        outStartU = 0;  // Space는 특별히 UV 좌표를 (0,0)으로 설정
        outStartV = 0;
        offset = 0;
        return;
    }
    else if (hangul >= L'A' && hangul <= L'Z') {

        StartU = 11;
        StartV = 0;
        offset = hangul - L'A'; // 대문자 위치
    }
    else if (hangul >= L'a' && hangul <= L'z') {
        StartU = 37;
        StartV = 0;
        offset = (hangul - L'a'); // 소문자는 대문자 다음 위치
    }
    else if (hangul >= L'0' && hangul <= L'9') {
        StartU = 1;
        StartV = 0;
        offset = (hangul - L'0'); // 숫자는 소문자 다음 위치
    }
    else if (hangul >= L'가' && hangul <= L'힣')
    {
        StartU = 63;
        StartV = 0;
        offset = hangul - L'가'; // 대문자 위치
    }

    if (offset == -1)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "Text Error");
    }

    const int offsetV = (offset + StartU) / ColumnCount;
    const int offsetU = (offset + StartU) % ColumnCount;

    outStartU = static_cast<float>(offsetU);
    outStartV = static_cast<float>(StartV + offsetV);
}