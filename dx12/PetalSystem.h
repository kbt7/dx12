#pragma once
#include <vector>
#include <string>
#include <random>
#include "DXApplication.h"

class PetalSystem {
public:
    PetalSystem(DXApplication* app, int windowWidth, int windowHeight);

    void Initialize(const std::vector<std::wstring>& texturePaths, int petalCount);
    void Update(float deltaTime);
    void Render();
    void Clear(); // 花びらを全消去
    void StartFadeOut(float duration); // フェードアウト開始
    void setWindowSize(int width, int height);

    bool IsFadingOut() const { return fadingOut_; }
    bool IsFinished() const { return fadingOut_ && fadeTimer_ >= fadeDuration_; }

private:
    struct Petal {
        float x, y;
        float vx, vy;
        float scale;
        float alpha;
        float rotation;
        float rotationSpeed;
        std::wstring textureKey;  // 花びらごとのキー
    };

    void ResetPetal(Petal& p);

    DXApplication* app_;
    int windowWidth_;
    int windowHeight_;
    std::vector<Petal> petals_;
    std::vector<std::wstring> texturePaths_;
    std::mt19937 rng_;

    // フェードアウト用
    bool fadingOut_ = false;
    float fadeTimer_ = 0.0f;
    float fadeDuration_ = 1.0f;
};
