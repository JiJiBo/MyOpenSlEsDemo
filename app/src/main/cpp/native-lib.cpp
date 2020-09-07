#include <jni.h>
#include <string>

extern "C" {
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
}

#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"1123",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"1123",FORMAT,##__VA_ARGS__);

FILE *pFile;

SLObjectItf engineObj = NULL;
SLEngineItf engineEngine = NULL;

SLObjectItf outputObj = NULL;
SLEnvironmentalReverbItf outputItf = NULL;
SLEnvironmentalReverbSettings outputSlSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

SLObjectItf pcmObj = NULL;
SLPlayItf pcmPlay = NULL;
SLVolumeItf pcmVolume = NULL;

SLAndroidSimpleBufferQueueItf androidSimpleBufferQueue = NULL;


uint8_t *out_buffer;

void getPcmData(void **pcm) {
    while (!feof(pFile)) {
        fread(out_buffer, 44100 * 2 * 2, 1, pFile);
        if (out_buffer == NULL) {
            LOGI("%s", "read end");
            break;
        } else {
            LOGI("%s", "reading");
        }
        *pcm = out_buffer;
        break;
    }
}

void *buffer;

void pcmBufferCallback(SLAndroidSimpleBufferQueueItf buff, void *context) {
    getPcmData(&buffer);
    if (NULL != buffer) {
        SLresult result;
        result = (*androidSimpleBufferQueue)->Enqueue(buff, buffer,
                                                      44100 * 2 * 2);
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zcf_myopenslesdemo_MainActivity_load(JNIEnv *env, jobject thiz, jstring str) {
    const char *path = env->GetStringUTFChars(str, 0);
    pFile = fopen(path, "r");
    if (pFile == NULL) {
        LOGE("文件打开失败 %s", path)
        return;
    } else {
        LOGE("文件打开成功 %s", path)
    }
    out_buffer = (uint8_t *) malloc(44100 * 2 * 2);
    SLresult result;
    slCreateEngine(&engineObj, 0, 0, 0, 0, 0);

    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);

    const SLInterfaceID mid[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputObj, 1, mid, mreq);
    result = (*outputObj)->Realize(outputObj, SL_BOOLEAN_FALSE);
    result = (*outputObj)->GetInterface(outputObj, SL_IID_ENVIRONMENTALREVERB, &outputItf);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputItf)->SetEnvironmentalReverbProperties(outputItf, &outputSlSettings);
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputObj};
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataSink audioSink = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmObj, &slDataSource, &audioSink,
                                                3, ids, req);

    (*pcmObj)->Realize(pcmObj, SL_BOOLEAN_FALSE);
    (*pcmObj)->GetInterface(pcmObj, SL_IID_PLAY, &pcmPlay);
    (*pcmObj)->GetInterface(pcmObj, SL_IID_BUFFERQUEUE, &androidSimpleBufferQueue);
    (*androidSimpleBufferQueue)->RegisterCallback(androidSimpleBufferQueue, pcmBufferCallback,
                                                  NULL);

    (*pcmObj)->GetInterface(pcmObj, SL_IID_VOLUME, &pcmVolume);
    (*pcmPlay)->SetPlayState(pcmPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallback(androidSimpleBufferQueue, NULL);
    env->ReleaseStringUTFChars(str, path);
}