#include "PetalSystem.h"
#include <DirectXMath.h>
using namespace DirectX;

PetalSystem::PetalSystem(DXApplication* app, int windowWidth, int windowHeight)
    : app_(app), windowWidth_(windowWidth), windowHeight_(windowHeight) {
    std::random_device rd;
    rng_ = std::mt19937(rd());
}

void PetalSystem::Initialize(const std::vector<std::wstring>& texturePaths, int petalCount) {
    texturePaths_ = texturePaths;
    petals_.resize(petalCount);

    for (int i = 0; i < petalCount; ++i) {
        auto& p = petals_[i];
        ResetPetal(p);

        // ランダムで画像を割り当て
        int idx = rand() % texturePaths_.size();
        p.textureKey = L"petal_" + std::to_wstring(i);

        // DXApplicationに登録
        app_->InitializeTexture(
            p.textureKey,
            texturePaths_[idx],
            p.x, p.y,
            64.0f * p.scale,
            64.0f * p.scale,
            p.alpha
        );
    }
}

void PetalSystem::Update(float deltaTime) {

    if (fadingOut_) {
        fadeTimer_ += deltaTime;
        float fadeRate = 1.0f - (fadeTimer_ / fadeDuration_);
        if (fadeRate < 0.0f) fadeRate = 0.0f;

        for (auto& p : petals_) {
            p.alpha = fadeRate;
        }
        return; // フェードアウト中は移動を止めたいならここで return
    }

    for (auto& p : petals_) {
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.rotation += p.rotationSpeed * deltaTime;

        // フェードアウトや画面外でリセット
        p.alpha -= 0.05f * deltaTime;
        if (p.alpha <= 0.0f || p.y > windowHeight_ + 50) {
            ResetPetal(p);
        }
    }
}

void PetalSystem::Render() {
    for (auto& p : petals_) {
        app_->DrawTexture(
            p.textureKey,
            p.x, p.y,
            64.0f * p.scale,
            64.0f * p.scale,
            p.alpha,
            1.0f,        // brightness
            p.rotation,
            true
        );
    }
}

void PetalSystem::Clear() {
    // DXApplication 側のテクスチャを解放
    for (auto& p : petals_) {
        app_->ReleaseTexture(p.textureKey);  // ここでGPUリソースも解放
    }

    // 花びらコンテナを完全に破棄してメモリを解放
    std::vector<Petal>().swap(petals_);
}


void PetalSystem::StartFadeOut(float duration) {
    fadingOut_ = true;
    fadeDuration_ = duration;
    fadeTimer_ = 0.0f;
}

void PetalSystem::ResetPetal(Petal& p) {
    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(windowWidth_));
    std::uniform_real_distribution<float> distVx(-150.0f, 150.0f);   // 横速度の幅を広く
    std::uniform_real_distribution<float> distVy(50.0f, 200.0f);    // 落下速度の幅を広く
    std::uniform_real_distribution<float> distScale(0.3f, 1.2f);      // 大きさも少しばらつき
    std::uniform_real_distribution<float> distRotation(0.0f, XM_2PI);
    std::uniform_real_distribution<float> distRotationSpeed(-3.0f, 3.0f); // 回転速度も幅広く

    p.x = distX(rng_);
    std::uniform_real_distribution<float> distY(-windowHeight_, -10.0f); // -100〜-10ピクセル上
    p.y = distY(rng_);
    p.vx = distVx(rng_);
    p.vy = distVy(rng_);
    p.scale = distScale(rng_);
    p.alpha = 1.0f;
    p.rotation = distRotation(rng_);
    p.rotationSpeed = distRotationSpeed(rng_);
}

void PetalSystem::setWindowSize(int width, int height) {
    windowWidth_ = width;
    windowHeight_ = height;
}