#include "IRenderPass.h"

#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "BaseGizmos/GizmoBaseComponent.h"

FowardRenderPass::FowardRenderPass(ID3D11VertexShader* InVertexShader, ID3D11PixelShader* InPixelShader,
                                   ID3D11InputLayout* InInputLayout)
        : VertexShader(InVertexShader), PixelShader(InPixelShader), InputLayout(InInputLayout)
{
}

void FowardRenderPass::Setup(ID3D11Device* device)
{
    if (!VertexShader || !PixelShader || !InputLayout)
    {
        
    }
}

void FowardRenderPass::Execute(ID3D11DeviceContext* context)
{
    // context->IASetInputLayout(InputLayout);
    // context->VSSetShader(VertexShader, nullptr, 0);
    // context->PSSetShader(PixelShader, nullptr, 0);

    for (auto Renderable : Renderables)
    {
        if (Renderable->IsA<UStaticMeshComponent>())
        {
            
        }
        else if (Renderable->IsA<UBillboardComponent>())
        {
            
        }
        else if (Renderable->IsA<UGizmoBaseComponent>())
        {
            
        }
        else if (Renderable->IsA<UPrimitiveComponent>())
        {
            
        }
        else if (Renderable->IsA<ULightComponentBase>())
        {
            
        }
    }
}
