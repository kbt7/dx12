#include "AudioEngine.h"

using Microsoft::WRL::ComPtr;

bool AudioEngine::Initialize() {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) return false;

    hr = XAudio2Create(&xaudio2_);
    if (FAILED(hr)) return false;

    hr = xaudio2_->CreateMasteringVoice(&masteringVoice_);
    return SUCCEEDED(hr);
}

void AudioEngine::Shutdown() {
    if (currentBGMVoice_) {
        currentBGMVoice_->Stop();
        currentBGMVoice_->DestroyVoice();
        currentBGMVoice_ = nullptr;
    }

    masteringVoice_->DestroyVoice();
    masteringVoice_ = nullptr;

    xaudio2_.Reset();
    MFShutdown();
}

bool AudioEngine::LoadMP3(const std::wstring& filename, AudioData& outAudio) {
    ComPtr<IMFSourceReader> reader;
    HRESULT hr = MFCreateSourceReaderFromURL(filename.c_str(), nullptr, &reader);
    if (FAILED(hr)) return false;

    ComPtr<IMFMediaType> audioType;
    MFCreateMediaType(&audioType);
    audioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    audioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, audioType.Get());

    ComPtr<IMFMediaType> nativeType;
    reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &nativeType);

    UINT32 bits = 16;
    UINT32 channels = MFGetAttributeUINT32(nativeType.Get(), MF_MT_AUDIO_NUM_CHANNELS, 0);
    UINT32 sampleRate = MFGetAttributeUINT32(nativeType.Get(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);

    outAudio.format.wFormatTag = WAVE_FORMAT_PCM;
    outAudio.format.nChannels = (WORD)channels;
    outAudio.format.nSamplesPerSec = sampleRate;
    outAudio.format.wBitsPerSample = (WORD)bits;
    outAudio.format.nBlockAlign = (channels * bits) / 8;
    outAudio.format.nAvgBytesPerSec = sampleRate * outAudio.format.nBlockAlign;
    outAudio.format.cbSize = 0;

    while (true) {
        ComPtr<IMFSample> sample;
        DWORD flags = 0;
        DWORD streamIndex = 0;
        LONGLONG llTimeStamp = 0;

        hr = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &sample);
        if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) break;
        if (!sample) continue;

        ComPtr<IMFMediaBuffer> buffer;
        sample->ConvertToContiguousBuffer(&buffer);

        BYTE* data = nullptr;
        DWORD maxLength = 0, currentLength = 0;
        buffer->Lock(&data, &maxLength, &currentLength);
        outAudio.pcmBuffer.insert(outAudio.pcmBuffer.end(), data, data + currentLength);
        buffer->Unlock();
    }

    return true;
}

bool AudioEngine::PlayAudio(const AudioData& audio, bool loop, IXAudio2SourceVoice** outVoice, float volume) {
    IXAudio2SourceVoice* voice = nullptr;
    HRESULT hr = xaudio2_->CreateSourceVoice(&voice, &audio.format);
    if (FAILED(hr)) return false;

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = static_cast<UINT32>(audio.pcmBuffer.size());
    buffer.pAudioData = audio.pcmBuffer.data();
    if (loop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }
    else {
        buffer.Flags = XAUDIO2_END_OF_STREAM;
    }

    hr = voice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) {
        voice->DestroyVoice();
        return false;
    }

    voice->SetVolume(volume); // ← ここ追加
    voice->Start();

    if (outVoice) {
        *outVoice = voice;
    }
    else {
        // 音が終わるまで別スレッドで待機してから破棄
        std::thread([voice]() {
            XAUDIO2_VOICE_STATE state = {};
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                voice->GetState(&state);
            } while (state.BuffersQueued > 0);
            voice->DestroyVoice();
            }).detach();
    }

    return true;
}

bool AudioEngine::LoadBGM(const std::wstring& id, const std::wstring& filename) {
    AudioData data;
    if (!LoadMP3(filename, data)) return false;

    bgmMap_[id] = std::move(data);
    return true;
}

bool AudioEngine::PlayBGM(const std::wstring& name) {
    auto it = bgmMap_.find(name);
    if (it == bgmMap_.end()) return false;

    StopBGM();

    float volume = 1.0f;
    auto vIt = bgmVolumeMap_.find(name);
    if (vIt != bgmVolumeMap_.end()) {
        volume = vIt->second;
    }

    // ★volume を引数に渡すようにする
    if (!PlayAudio(it->second, true, &currentBGMVoice_, volume)) return false;

    return true;
}

bool AudioEngine::LoadSE(const std::wstring& id, const std::wstring& filename) {
    AudioData data;
    if (!LoadMP3(filename, data)) return false;

    seMap_[id] = std::move(data);
    return true;
}

bool AudioEngine::PlaySE(const std::wstring& name) {
    auto it = seMap_.find(name);
    if (it == seMap_.end()) return false;

    float volume = 1.0f;
    auto vIt = seVolumeMap_.find(name);
    if (vIt != seVolumeMap_.end()) {
        volume = vIt->second;
    }

    return PlayAudio(it->second, false, nullptr, volume);
}

void AudioEngine::StopBGM() {
    if (currentBGMVoice_) {
        currentBGMVoice_->Stop();
        currentBGMVoice_->DestroyVoice();
        currentBGMVoice_ = nullptr;
        currentBGMId_.clear();
    }
}

void AudioEngine::SetVolumeForBGM(const std::wstring& name, float volume) {
    bgmVolumeMap_[name] = volume;
}

void AudioEngine::SetVolumeForSE(const std::wstring& name, float volume) {
    seVolumeMap_[name] = volume;
}