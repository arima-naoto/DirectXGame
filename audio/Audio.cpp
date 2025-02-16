#include "Audio.h"
#include "cassert"

Audio* Audio::GetInstance(){
	static Audio instance;
	return &instance;
}

void Audio::Initialize(){
	HRESULT result = S_FALSE;

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスの生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

Audio::SoundData Audio::SoundLoadData(const std::string &fileName)
{
	std::ifstream file;
	file.open(fileName.c_str(), std::ios::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	if (strncmp(riff.type, "WAVE", 4)) {
		assert(0);
	}

	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	ChunkHeader data;
	while (true) {
		file.read(reinterpret_cast<char*>(&data), sizeof(data));

		if (strncmp(data.id, "data", 4) == 0) {
			break;
		}

		file.seekg(data.size, std::ios_base::cur);
	}

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	file.close();
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	return soundData;
}

void Audio::PlayWave(const SoundData& soundData,bool loop){
	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = loop ? XAUDIO2_LOOP_INFINITE : 0;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

}

void Audio::Finalize(){
	xAudio2.Reset();

	for (int i = 0; i < soundDatas_.size(); i++) {
		SoundUnload(&soundDatas_[i]);
	}
}

void Audio::SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}


