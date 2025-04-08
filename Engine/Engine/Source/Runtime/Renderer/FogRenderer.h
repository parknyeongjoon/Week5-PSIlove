#pragma once

class ULevel;
class FGraphicsDevice;
class FEditorViewportClient;

class FFogRenderer
{
public:
    FFogRenderer();
    ~FFogRenderer();

    void Initialize(FGraphicsDevice* graphics);
    void Release();

    void RenderViewFog(ULevel* level, FEditorViewportClient* viewportClient);
    void InitFogConstants();
    void ExponentialPixelMain();

};

