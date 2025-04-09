#pragma once
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "Engine/ResourceMgr.h"

class UProjectileMovementComponent;
class AActor;
class UnrealEd;
class UImGuiManager;
class ULevel;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class SLevelEditor;

class FEngineLoop
{
public:
    FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Render();
    void Tick();
    void EditorTick(double elapsedTime);
    void SpawnMeteor();
    void PIETick(double elapsedTime);
    void Exit();
    float GetAspectRatio(IDXGISwapChain* swapChain) const;
    void EditorInput();
    void BeginPIE() const;
    void PIEInput() const;
    EWorldType GetWorldType() const {return WorldContexts[curWorldContextIndex].worldType;}

private:
    void WindowInit(HINSTANCE hInstance);
    void TogglePIE();

public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;
    bool bTestInput2 = false;
    
    HWND hWnd;

private:
    TArray<FWorldContext> WorldContexts;
    UImGuiManager* UIMgr;
    ULevel* GLevel;
    UWorld* GWorld;
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    bool bIsExit = false;
    const int32 targetFPS = 60;
    double accumulatedTime = 0.0f;
    bool bTestInput = false;
    int curWorldContextIndex = 0;
    UProjectileMovementComponent* movementActor = nullptr; 

public:
    ULevel* GetLevel() const { return GLevel; }
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
};
