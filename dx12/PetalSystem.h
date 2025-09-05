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
    void Clear(); // �Ԃт��S����
    void StartFadeOut(float duration); // �t�F�[�h�A�E�g�J�n

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
        std::wstring textureKey;  // �Ԃт炲�Ƃ̃L�[
    };

    void ResetPetal(Petal& p);

    DXApplication* app_;
    int windowWidth_;
    int windowHeight_;
    std::vector<Petal> petals_;
    std::vector<std::wstring> texturePaths_;
    std::mt19937 rng_;

    // �t�F�[�h�A�E�g�p
    bool fadingOut_ = false;
    float fadeTimer_ = 0.0f;
    float fadeDuration_ = 1.0f;
};
