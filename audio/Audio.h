#pragma once
#include "xaudio2.h"
#pragma comment(lib,"xaudio2.lib")

#include <fstream>

#include "wrl.h"
using Microsoft::WRL::ComPtr;
#include "array"

class Audio{

public:// メンバ構造体

	struct ChunkHeader {
		char id[4];
		int32_t size;
	};

	struct RiffHeader {
		ChunkHeader chunk;
		char type[4];
	};

	struct FormatChunk {
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};

	struct SoundData {
		WAVEFORMATEX wfex;
		BYTE* pBuffer;
		unsigned int bufferSize;
	};

public:

	static Audio* GetInstance();

	void Initialize();

	SoundData SoundLoadData(const std::string &fileName);

	void PlayWave(const SoundData& soundData,bool loop);

	void Finalize();

private:

	Audio() = default;
	~Audio() = default;
	Audio(const Audio& obj) = delete;
	Audio& operator=(const Audio& obj) = delete;

	void SoundUnload(SoundData* soundData);

private:
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice *masterVoice;
	std::array<SoundData, 256> soundDatas_ = {};
};

