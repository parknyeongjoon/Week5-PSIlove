#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "Level.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "UnrealClient.h"
#include "slate/Widgets/Layout/SSplitter.h"
#include "LevelEditor/SLevelEditor.h"
#include "World.h"
#include "Actors/Object/FireBall.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }
    int zDelta = 0;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            //UGraphicsDevice 객체의 OnResize 함수 호출
            if (FEngineLoop::graphicDevice.SwapChain)
            {
                FEngineLoop::graphicDevice.OnResize(hWnd);
            }
            for (int i = 0; i < 4; i++)
            {
                if (GEngineLoop.GetLevelEditor())
                {
                    if (GEngineLoop.GetLevelEditor()->GetViewports()[i])
                    {
                        GEngineLoop.GetLevelEditor()->GetViewports()[i]->ResizeViewport(FEngineLoop::graphicDevice.SwapchainDesc);
                    }
                }
            }
        }
     Console::GetInstance().OnResize(hWnd);
    // ControlPanel::GetInstance().OnResize(hWnd);
    // PropertyPanel::GetInstance().OnResize(hWnd);
    // Outliner::GetInstance().OnResize(hWnd);
    // ViewModeDropdown::GetInstance().OnResize(hWnd);
    // ShowFlags::GetInstance().OnResize(hWnd);
        if (GEngineLoop.GetUnrealEditor())
        {
            GEngineLoop.GetUnrealEditor()->OnResize(hWnd);
        }
        ViewportTypePanel::GetInstance().OnResize(hWnd);
        break;
    case WM_MOUSEWHEEL:
        if (ImGui::GetIO().WantCaptureMouse)
            return 0;
        zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // 휠 회전 값 (+120 / -120)
        if (GEngineLoop.GetLevelEditor())
        {
            if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsPerspective())
            {
                if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetIsOnRBMouseClick())
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(
                        static_cast<float>(GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() + zDelta * 0.01)
                    );
                }
                else
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->CameraMoveForward(zDelta * 0.1f);
                }
            }
            else
            {
                FEditorViewportClient::SetOthoSize(-zDelta * 0.01f);
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

FGraphicsDevice FEngineLoop::graphicDevice;
FRenderer FEngineLoop::renderer;
FResourceMgr FEngineLoop::resourceMgr;
uint32 FEngineLoop::TotalAllocationBytes = 0;
uint32 FEngineLoop::TotalAllocationCount = 0;

FEngineLoop::FEngineLoop()
    : hWnd(nullptr)
    , UIMgr(nullptr)
    , GLevel(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
{
}

int32 FEngineLoop::PreInit()
{
    return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
    /* must be initialized before window. */
    UnrealEditor = new UnrealEd();
    UnrealEditor->Initialize();

    WindowInit(hInstance);
    graphicDevice.Initialize(hWnd);
    renderer.Initialize(&graphicDevice);

    UIMgr = new UImGuiManager;
    UIMgr->Initialize(hWnd, graphicDevice.Device, graphicDevice.DeviceContext);

    resourceMgr.Initialize(&renderer, &graphicDevice);
    LevelEditor = new SLevelEditor();
    LevelEditor->Initialize();

    GWorld = FObjectFactory::ConstructObject<UWorld>();
    WorldContexts.Add({GWorld, EWorldType::Editor});
    GLevel = FObjectFactory::ConstructObject<ULevel>();
    GWorld->Level =GLevel;
    GLevel->Initialize(EWorldType::Editor);

    //TODO: 맵 세이브 생기면 삭제
    AStaticMeshActor* map = GetLevel()->SpawnActor<AStaticMeshActor>();
    map->GetStaticMeshComponent()->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Demo.obj"));
    map->SetActorLocation(FVector(0,0,1000));
    map->SetActorRotation(FVector(90,0,90));
    map->SetActorScale(FVector(5,5,5));

    AStaticMeshActor* car = GetLevel()->SpawnActor<AStaticMeshActor>();
    car->GetStaticMeshComponent()->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Dodge.obj"));
    car->SetActorLocation(FVector(0,-15000,960));
    car->AddComponent<UProjectileMovementComponent>();

    leftLight = car->AddComponent<ULightComponent>();
    leftLight->SetLocation(FVector(300,-20,10));
    leftLight->SetAttenuationRadius(300);
    leftLight->SetIntensity(80);
    rightLight = car->AddComponent<ULightComponent>();
    rightLight->SetLocation(FVector(300,20,10));
    rightLight->SetAttenuationRadius(300);
    rightLight->SetIntensity(80);
    // test 여기까지

    WorldContexts.Add({});

    return 0;
}


void FEngineLoop::Render()
{
    graphicDevice.ClearRenderTarget();
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> viewportClient = GetLevelEditor()->GetActiveViewportClient();
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetViewportClient(i);
            renderer.SetRenderObj(GLevel);
            renderer.PrepareRender(GLevel);
            renderer.Render(GetLevel(), LevelEditor->GetActiveViewportClient());
        }
        GetLevelEditor()->SetViewportClient(viewportClient);
    }
    else
    {
        renderer.SetRenderObj(GLevel);
        graphicDevice.ClearRenderTarget();
        graphicDevice.Prepare();
        renderer.PrepareRender(GLevel);
        renderer.Render(GetLevel(),LevelEditor->GetActiveViewportClient());
    }
}

void FEngineLoop::Tick()
{
    LARGE_INTEGER frequency;
    const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER startTime, endTime;
    double elapsedTime = 1.0;

    while (bIsExit == false)
    {
        QueryPerformanceCounter(&startTime);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
            DispatchMessage(&msg);  // 메시지를 WndProc에 전달

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        switch (WorldContexts[curWorldContextIndex].worldType)
        {
        case EWorldType::Editor:
            EditorTick(elapsedTime);
            break;
        case EWorldType::PIE:
            PIETick(elapsedTime);
            break;
        }

        if (bTestInput2 == true)
        {
            TogglePIE();
            bTestInput2 = false;
        }

        do
        {
            Sleep(0);
            QueryPerformanceCounter(&endTime);
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        while (elapsedTime < targetFrameTime);
    }
}

void FEngineLoop::EditorTick(double elapsedTime)
{
    EditorInput();
    GLevel->EditorTick(elapsedTime);
    LevelEditor->Tick(elapsedTime);
    Render();
    UIMgr->BeginFrame();
    UnrealEditor->Render();

    Console::GetInstance().Draw();

    UIMgr->EndFrame();

    // Pending 처리된 오브젝트 제거
    GUObjectArray.ProcessPendingDestroyObjects();

    graphicDevice.SwapBuffer();
}

void FEngineLoop::SpawnMeteor()
{
    
}

void FEngineLoop::PIETick(double elapsedTime)
{
    accumulatedTime += elapsedTime;
    
    PIEInput();
    GLevel->PIETick(elapsedTime);
    SpawnMeteor();
    if (movementComponent != nullptr)
    {
        //카메라 세팅
        LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.SetLocation(FVector(-8,-14,30) + movementComponent->GetOwner()->GetActorLocation());
        movementComponent->GetOwner()->SetActorRotation(FVector(0,0,LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.GetRotation().z));
    }
    
    LevelEditor->Tick(elapsedTime);
    Render();

    UIMgr->BeginFrame();
    UnrealEditor->RenderPIE();

    UIMgr->EndFrame();

    // Pending 처리된 오브젝트 제거
    GUObjectArray.ProcessPendingDestroyObjects();

    graphicDevice.SwapBuffer();
}

void FEngineLoop::TogglePIE()
{
    curWorldContextIndex == 0 ? curWorldContextIndex = 1 : curWorldContextIndex = 0;
    if (curWorldContextIndex == 1)
    {
        WorldContexts[1] = {Cast<UWorld>(WorldContexts[0].World->Duplicate()), EWorldType::PIE};
        WorldContexts[1].World->Level->Initialize(EWorldType::PIE);
        uint32 NewFlag = LevelEditor->GetActiveViewportClient()->GetShowFlag() & 31;
        LevelEditor->GetActiveViewportClient()->SetShowFlag(NewFlag);
        // LevelEditor->DisableMultiViewport();
        for (auto& actor : WorldContexts[1].World->Level->GetActors())
        {
            for (auto& comp : actor->GetComponents())
            {
                if (auto* movementComp = Cast<UProjectileMovementComponent>(comp))
                {
                    movementComponent = movementComp;
                    break;
                }
            }
        }
    }
    else
    {
        uint32 NewFlag = LevelEditor->GetActiveViewportClient()->GetShowFlag() | 1;
        LevelEditor->GetActiveViewportClient()->SetShowFlag(NewFlag);
    }
    GWorld = WorldContexts[curWorldContextIndex].World;
    GLevel = GWorld->Level;
}

float FEngineLoop::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEngineLoop::EditorInput()
{
    if (GetAsyncKeyState('M') & 0x8000)
    {
        if (!bTestInput)
        {
            bTestInput = true;
            if (LevelEditor->IsMultiViewport())
            {
                LevelEditor->DisableMultiViewport();
            }
            else
                LevelEditor->EnableMultiViewport();
        }
    }
    else
    {
        bTestInput = false;
    }
}

void FEngineLoop::PIEInput() const
{
    if (movementComponent != nullptr)
    {
        if (GetAsyncKeyState('W') & 0x8000)
        {
            FVector forward = LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.GetForwardVector();
            forward.z = 0;
            forward = forward.Normalize();
            forward = forward * movementComponent->GetAcceleration();
            movementComponent->AddVelocity(forward);
        }
        if (GetAsyncKeyState('S') & 0x8000)
        {
            FVector back = LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.GetForwardVector();
            back.z = 0;
            back = back.Normalize();
            back = back * -movementComponent->GetAcceleration();
            movementComponent->AddVelocity(back);
        }
        // if (GetAsyncKeyState('D') & 0x8000)
        // {
        //     FVector right = LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.GetRightVector();
        //     right.z = 0;
        //     right = right.Normalize();
        //     right = right * movementActor->GetAcceleration();
        //     movementActor->AddVelocity(right);
        // }
        // if (GetAsyncKeyState('A') & 0x8000)
        // {
        //     FVector left = LevelEditor->GetActiveViewportClient()->ViewTransformPerspective.GetRightVector();
        //     left.z = 0;
        //     left = left.Normalize();
        //     left = left * -movementActor->GetAcceleration();
        //     movementActor->AddVelocity(left);
        // }
    }
}

void FEngineLoop::Exit()
{
    LevelEditor->Release();
    GLevel->Release();
    delete GLevel;
    UIMgr->Shutdown();
    delete UIMgr;
    resourceMgr.Release(&renderer);
    renderer.Release();
    graphicDevice.Release();
}


void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
    WCHAR WindowClass[] = L"JungleWindowClass";

    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wndclass = {0};
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = WindowClass;

    RegisterClassW(&wndclass);

    hWnd = CreateWindowExW(
        0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr
    );
}
