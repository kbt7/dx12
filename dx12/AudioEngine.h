#pragma once

#include <mfapi.h>
#include <mftransform.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <propvarutil.h>
#include <xaudio2.h>
#include <wrl.h>
#include <comdef.h>
#include <mfreadwrite.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <thread>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "xaudio2.lib")

using Microsoft::WRL::ComPtr;

class AudioEngine {
public:
    bool Initialize();
    void Shutdown();

    bool LoadBGM(const std::wstring& id, const std::wstring& filename); // �����o�^
    bool PlayBGM(const std::wstring& id);                                // ID�ōĐ�
    bool LoadSE(const std::wstring& id, const std::wstring& filename);
    bool PlaySE(const std::wstring& id);
    void StopBGM();
    void SetVolumeForBGM(const std::wstring& name, float volume);
    void SetVolumeForSE(const std::wstring& name, float volume);

private:
    struct AudioData {
        WAVEFORMATEX format{};
        std::vector<BYTE> pcmBuffer;
    };

    bool LoadMP3(const std::wstring& filename, AudioData& outAudio);
    bool PlayAudio(const AudioData& audio, bool loop, IXAudio2SourceVoice** outVoice, float volume = 1.0f);

private:
    Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
    IXAudio2MasteringVoice* masteringVoice_ = nullptr;

    std::unordered_map<std::wstring, AudioData> bgmMap_;  // ����BGM�ێ�
    std::unordered_map<std::wstring, AudioData> seMap_; // ���ʉ��p�̃}�b�v
    std::unordered_map<std::wstring, float> bgmVolumeMap_;
    std::unordered_map<std::wstring, float> seVolumeMap_;
    IXAudio2SourceVoice* currentBGMVoice_ = nullptr;      // ���݂̍Đ���BGM
    std::wstring currentBGMId_;                           // �����Ă�ID
};