#include "ControlEditorPanel.h"

#include "Level.h"
#include "Actors/Player.h"
#include "Components/CubeComp.h"
#include "Components/LightComponent.h"
#include "Components/SphereComp.h"
#include "Components/ParticleSubUVComp.h"
#include "Components/TextBillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "ImGUI/imgui_internal.h"
#include "LevelEditor/SLevelEditor.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "UnrealEd/EditorViewportClient.h"
#include "PropertyEditor/ShowFlags.h"

void ControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);
    
    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    float PanelPosX = 1.0f;
    float PanelPosY = 1.0f;

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    
    /* Render Start */
        
    ImGui::Begin("Control Panel", nullptr, PanelFlags);
    
    if (GEngineLoop.GetWorldType() == EWorldType::PIE)
    {
        TogglePIEMode();
        ImGui::End();
        return;
    }
    CreateMenuButton(IconSize, IconFont);
    
    ImGui::SameLine();
    
    CreateFlagButton();
    
    ImGui::SameLine();

    CreateModifyButton(IconSize, IconFont);

    ImGui::SameLine();

    TogglePIEMode();
    ImGui::SameLine();

    /* Get Window Content Region */
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    /* Move Cursor X Position */
    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));
    
    ImGui::PushFont(IconFont);
    ImGui::PopFont();

    
    ImGui::End();
}

void ControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();
    
    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 170), ImGuiCond_Always);
        
        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if (ImGui::MenuItem("New Scene"))
        {
            // TODO: New Scene
        }

        if (ImGui::MenuItem("Load Scene"))
        {
            char const * lFilterPatterns[1]={"*.scene"};
            const char* FileName =  tinyfd_openFileDialog("Open Scene File", "", 1, lFilterPatterns,"Scene(.scene) file", 0);

            if (FileName == nullptr)
            {
                tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                ImGui::End();
                return;
            }

            // TODO: Load Scene
        }

        ImGui::Separator();
        
        if (ImGui::MenuItem("Save Scene"))
        {
            char const * lFilterPatterns[1]={"*.scene"};
            const char* FileName =  tinyfd_saveFileDialog("Save Scene File", "", 1, lFilterPatterns,"Scene(.scene) file");

            if (FileName == nullptr)
            {
                ImGui::End();
                return;
            }

            // TODO: Save Scene

            tinyfd_messageBox("알림", "저장되었습니다.", "ok", "info", 1);
        }

        ImGui::Separator();
        
        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const * lFilterPatterns[1]={"*.obj"};
                const char* FileName =  tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns,"Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << std::endl;

                    if (FManagerOBJ::CreateStaticMesh(FileName) == nullptr)
                    {
                        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                    }
                }
            }
            
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("프로그램 종료");   
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("프로그램 종료", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("정말 프로그램을 종료하시겠습니까?");
            ImGui::Separator();

            float ContentWidth = ImGui::GetWindowContentRegionMax().x;

            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));
            
            if (ImGui::Button("OK", ImVec2(80, 0))) { PostQuitMessage(0); }

            ImGui::SameLine();
            
            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }
        
        ImGui::End();
    }
}

void ControlEditorPanel::CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", ButtonSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        ImGui::Text("Grid Scale");
        GridScale = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetGridSize();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Grid Scale", &GridScale, 0.1f, 1.0f, 20.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
        }
        ImGui::Separator();

        ImGui::Text("Camera FOV");
        FOV = &GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewFOV;
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Fov", FOV, 0.1f, 30.0f, 120.0f, "%.1f"))
        {
            //GEngineLoop.GetWorld()->GetCamera()->SetFOV(FOV);
            
        }
        ImGui::Spacing();

        ImGui::Text("Camera Speed");
        CameraSpeed = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CamSpeed", &CameraSpeed, 0.1f, 0.198f, 192.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(CameraSpeed);
        }
        
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c8", ButtonSize))
    {
        ImGui::OpenPopup("PrimitiveControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("PrimitiveControl"))
    {
        struct Primitive {
            const char* label;
            int obj;
        };

        static const Primitive primitives[] = {
            { .label= "Cube",      .obj= OBJ_CUBE },
            { .label= "Sphere",    .obj= OBJ_SPHERE },
            { .label= "SpotLight", .obj= OBJ_SpotLight },
            { .label= "Particle",  .obj= OBJ_PARTICLE },
            { .label= "Billboard", .obj= OBJ_BILLBOARD },
            { .label= "Text",      .obj= OBJ_Text }
        };

        for (const auto& primitive : primitives)
        {
            if (ImGui::Selectable(primitive.label))
            {
                // GEngineLoop.GetWorld()->SpawnObject(static_cast<OBJECTS>(primitive.obj));
                ULevel* level = GEngineLoop.GetLevel();
                AActor* SpawnedActor = nullptr;
                switch (static_cast<OBJECTS>(primitive.obj))
                {
                case OBJ_SPHERE:
                {
                    SpawnedActor = level->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SPHERE"));
                    SpawnedActor->AddComponent<USphereComp>();
                    break;
                }
                case OBJ_CUBE:
                {
                    AStaticMeshActor* TempActor = level->SpawnActor<AStaticMeshActor>();
                    TempActor->SetActorLabel(TEXT("OBJ_CUBE"));
                    UStaticMeshComponent* MeshComp = TempActor->GetStaticMeshComponent();
                    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
                    break;
                }
                case OBJ_SpotLight:
                {
                    SpawnedActor = level->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SpotLight"));
                    auto a = SpawnedActor->AddComponent<ULightComponentBase>();
                    UBillboardComponent* BillboardComp = SpawnedActor->AddComponent<UBillboardComponent>();
                    BillboardComp->SetTexture(L"Editor/Icon/SpotLight_64x.png");
                    
                    break;
                }
                case OBJ_PARTICLE:
                {
                    SpawnedActor = level->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
                    UParticleSubUVComp* ParticleComponent = SpawnedActor->AddComponent<UParticleSubUVComp>();
                    ParticleComponent->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
                    ParticleComponent->SetRowColumnCount(6, 6);
                    ParticleComponent->SetScale(FVector(10.0f, 10.0f, 1.0f));
                    ParticleComponent->Activate();
                    break;
                }
                case OBJ_Text:
                {
                    SpawnedActor = level->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_Text"));
                    UTextRenderComponent* TextComponent = SpawnedActor->AddComponent<UTextRenderComponent>();
                    TextComponent->SetTexture(L"Assets/Texture/font.png");
                    TextComponent->SetRowColumnCount(106, 106);
                    TextComponent->SetText(L"안녕하세요 Jungle 1");
                        TextComponent->SetRotation(FVector(90.f, 0.f, 0.f));
                    break;
                }
                case OBJ_BILLBOARD:
                {
                    SpawnedActor = level->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_BILLBOARD"));
                    UBillboardComponent* BillboardComp = SpawnedActor->AddComponent<UBillboardComponent>();
                    BillboardComp->SetTexture(L"Editor/Icon/S_Actor.png");
                }
                case OBJ_TRIANGLE:
                case OBJ_CAMERA:
                case OBJ_PLAYER:
                case OBJ_END:
                    break;
                }
        
                if (SpawnedActor)
                {
                    level->SetPickedActor(SpawnedActor);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void ControlEditorPanel::CreateFlagButton() const
{
    auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    const char* ViewTypeNames[] = { "Perspective", "Top", "Bottom", "Left", "Right", "Front", "Back" };
    ELevelViewportType ActiveViewType = ActiveViewport->GetViewportType();
    FString TextViewType = ViewTypeNames[ActiveViewType];
    
    if (ImGui::Button(GetData(TextViewType), ImVec2(120, 32)))
    {
        // toggleViewState = !toggleViewState;
        ImGui::OpenPopup("ViewControl");
    }

    if (ImGui::BeginPopup("ViewControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewTypeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewportType() == i);
            if (ImGui::Selectable(ViewTypeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewportType((ELevelViewportType)i);
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    const char* ViewModeNames[] = { "Lit", "Unlit", "Wireframe" };
    FString SelectLightControl = ViewModeNames[(int)ActiveViewport->GetViewMode()];
    ImVec2 LightTextSize = ImGui::CalcTextSize(GetData(SelectLightControl));
    
    if (ImGui::Button(GetData(SelectLightControl), ImVec2(30 + LightTextSize.x, 32)))
    {
        ImGui::OpenPopup("LightControl");
    }

    if (ImGui::BeginPopup("LightControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewModeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewMode() == i);
            if (ImGui::Selectable(ViewModeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewMode((EViewModeIndex)i);
                FEngineLoop::graphicDevice.ChangeRasterizer(ActiveViewport->GetViewMode());
                FEngineLoop::renderer.ChangeViewMode(ActiveViewport->GetViewMode());
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowControl");
    }
    
    const char* items[] = { "AABB", "Primitive", "BillBoard", "UUID"};
    uint64 ActiveViewportFlags = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowControl"))
    {
        bool selected[IM_ARRAYSIZE(items)] =
        {
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_UUIDText)) != 0
        };  // 각 항목의 체크 상태 저장
        
        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }
        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(selected));
        ImGui::EndPopup();
    }
}

// code is so dirty / Please refactor
void ControlEditorPanel::CreateSRTButton(ImVec2 ButtonSize) const
{
    AEditorPlayer* Player = GEngineLoop.GetLevel()->GetEditorPlayer();

    ImVec4 ActiveColor = ImVec4(0.00f, 0.00f, 0.85f, 1.0f);
    
    ControlMode ControlMode = Player->GetControlMode();

    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9bc", ButtonSize)) // Move
    {
        Player->SetMode(CM_TRANSLATION);
    }
    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PopStyleColor();
    }
	
    ImGui::SameLine();

    if (ControlMode == CM_ROTATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9d3", ButtonSize)) // Rotate
    {
        Player->SetMode(CM_ROTATION);
    }
    if (ControlMode == CM_ROTATION)
    {
        ImGui::PopStyleColor();
    }
	
    ImGui::SameLine();

    if (ControlMode == CM_SCALE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9ab", ButtonSize)) // Scale
    {
        Player->SetMode(CM_SCALE);
    }
    if (ControlMode == CM_SCALE)
    {
        ImGui::PopStyleColor();
    }
}

uint64 ControlEditorPanel::ConvertSelectionToFlags(const bool selected[]) const
{
    uint64 flags = static_cast<uint64>(EEngineShowFlags::None);

    if (selected[0])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    if (selected[1])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    if (selected[2])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
    if (selected[3])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
    return flags;
}

void ControlEditorPanel::TogglePIEMode() const
{

    // 버튼 스타일 설정
    ImGui::PushStyleColor(ImGuiCol_Button, GEngineLoop.GetWorldType() == EWorldType::Editor ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GEngineLoop.GetWorldType() == EWorldType::Editor ? ImVec4(0.3f, 0.9f, 0.3f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    
    // 삼각형 모양의 재생 버튼 (또는 일시정지 바)
    if (ImGui::Button("##PlayButton", ImVec2(30, 30))) {
        GEngineLoop.bTestInput2 = true;
    }
    
    // 버튼 위에 아이콘 그리기
    float buttonWidth = 30;
    float buttonHeight = 30;
    ImVec2 buttonPos = ImGui::GetItemRectMin();
    
    if (GEngineLoop.GetWorldType() == EWorldType::PIE) {
        // 일시정지 아이콘 (두 개의 수직 막대)
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float pauseBarWidth = 4.0f;
        float pauseBarHeight = 15.0f;
        float spacing = 4.0f;
        
        drawList->AddRectFilled(
            ImVec2(buttonPos.x + (buttonWidth / 2) - spacing - pauseBarWidth, buttonPos.y + (buttonHeight - pauseBarHeight) / 2),
            ImVec2(buttonPos.x + (buttonWidth / 2) - spacing, buttonPos.y + (buttonHeight + pauseBarHeight) / 2),
            IM_COL32(255, 255, 255, 255)
        );
        
        drawList->AddRectFilled(
            ImVec2(buttonPos.x + (buttonWidth / 2) + spacing, buttonPos.y + (buttonHeight - pauseBarHeight) / 2),
            ImVec2(buttonPos.x + (buttonWidth / 2) + spacing + pauseBarWidth, buttonPos.y + (buttonHeight + pauseBarHeight) / 2),
            IM_COL32(255, 255, 255, 255)
        );
    } else {
        // 재생 아이콘 (삼각형)
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 trianglePoints[3];
        float triangleHeight = 15.0f;
        float triangleWidth = 12.0f;
        
        trianglePoints[0] = ImVec2(buttonPos.x + (buttonWidth - triangleWidth) / 2 + 2, buttonPos.y + (buttonHeight - triangleHeight) / 2);
        trianglePoints[1] = ImVec2(buttonPos.x + (buttonWidth - triangleWidth) / 2 + 2, buttonPos.y + (buttonHeight + triangleHeight) / 2);
        trianglePoints[2] = ImVec2(buttonPos.x + (buttonWidth + triangleWidth) / 2 + 2, buttonPos.y + buttonHeight / 2);
        
        drawList->AddTriangleFilled(
            trianglePoints[0],
            trianglePoints[1],
            trianglePoints[2],
            IM_COL32(255, 255, 255, 255)
        );
    }
    
    ImGui::PopStyleColor(2);
}

void ControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
