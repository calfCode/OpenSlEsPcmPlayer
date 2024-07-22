#include <jni.h>
#include <string>

extern "C"{
#include<SLES/OpenSLES.h>
#include<android/log.h>
#include<SLES/OpenSLES_Android.h>
}
#define TAG "BRUCE" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
using namespace std;
//engine interface
static SLObjectItf engineObject= nullptr;
static SLEngineItf engineEngine= nullptr;
//output mix interfaces
static SLObjectItf outputMixObject= nullptr;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb= nullptr;
//player interface
static SLObjectItf pcmPlayerObject= nullptr;
static SLPlayItf pcmPlayerplay= nullptr;
//buffer queue
static SLAndroidSimpleBufferQueueItf pcmBufferQueue= nullptr;
//pcm file
FILE *pcmFile= nullptr;
void *buffer= nullptr;
uint8_t *out_buffer= nullptr;
static const SLEnvironmentalReverbSettings reverbSettings=SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


void playerCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf,void *context){
    if(bufferQueueItf!=pcmBufferQueue){
        LOGI("SLAndroidSimpleBufferQueueItf is not equal");
        return;
    }
    while(!feof(pcmFile)){
        size_t size=fread(out_buffer,44100*2*4,1,pcmFile);
        if(out_buffer== nullptr||size==0){
            LOGI("read end %ld",size);
        }else{
            LOGI("reading %ld",size);
        }
        buffer=out_buffer;
        break;
    }
    if(buffer){
        LOGI("buffer is not null");
        SLresult result=(*pcmBufferQueue)->Enqueue(pcmBufferQueue,buffer,44100*2*4);
        if(result!=SL_RESULT_SUCCESS){
            LOGI("pcmBufferQueue error %ld",result);
        }
    }
}

jint playPcmBySL(JNIEnv *env,jobject thiz,jstring pcm_path){
    const char *pcmPath=env->GetStringUTFChars(pcm_path, nullptr);
    pcmFile=fopen(pcmPath,"r");
    env->ReleaseStringUTFChars(pcm_path,pcmPath);
    if(pcmFile== nullptr){
        LOGI("open pcmFile error");
        return -1;
    }
    out_buffer=(uint8_t *)malloc(44100*2*4);
    //创建引擎对象
    SLresult result=slCreateEngine(&engineObject,0,nullptr,0,nullptr,nullptr);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("slCreateEngine failed %ld",result);
        return -1;
    }
    //实例化引擎
    result=(*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("engine realize failed %ld",result);
        return -1;
    }
    //获取引擎接口SLEngineItf
    result=(*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("GetInterface SLEngineItf failed %ld",result);
        return -1;
    }
    //创建输出混音器
    const SLInterfaceID ids[1]={SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1]={SL_BOOLEAN_FALSE};
    result=(*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,1,ids,req);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("CreateOutputMix failed %ld",result);
        return -1;
    }
    //实例化混音器
    result=(*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("Realize outputMixObject failed %ld",result);
        return -1;
    }
    //获取混音器接口SLEnvironmentalReverbItf
    result=(*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("GetInterface SLEnvironmentalReverbItf failed %ld",result);
        return -1;
    }
    //给混音器设置环境混响属性
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,&reverbSettings);
    //设置输入 SLDataSource
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM formatPcm={
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_32,
            SL_PCMSAMPLEFORMAT_FIXED_32,
            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource slDataSource={&loc_bufq,&formatPcm};
    //设置输出SLDataSink
    SLDataLocator_OutputMix loc_outmix={SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSink audioSnk={&loc_outmix, nullptr};
    //创建音频播放器对象
    const SLInterfaceID ids2[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req2[1] = {SL_BOOLEAN_TRUE};

    result=(*engineEngine)->CreateAudioPlayer(engineEngine,&pcmPlayerObject,&slDataSource,&audioSnk,1,ids2,req2);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("CreateAudioPlayer failed %ld",result);
        return -1;
    }
    //实例化音频播放器对象
    result=(*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("Realize pcmPlayerObject failed %ld",result);
        return -1;
    }
    //获取音频播放器接口pcmPlayerplay
    result=(*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&pcmPlayerplay);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("GetInterface pcmPlayerplay failed %ld",result);
        return -1;
    }
    //获取音频播放的buffer接口SLAndroidSimpleBufferQueueItf
    result=(*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&pcmBufferQueue);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("GetInterface pcmBufferQueue failed %ld",result);
        return -1;
    }
    //注册回调RegisterCallback
    result=(*pcmBufferQueue)->RegisterCallback(pcmBufferQueue,playerCallback, nullptr);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("RegisterCallback failed %ld",result);
        return -1;
    }
    //设置播放状态为playing
    result=(*pcmPlayerplay)->SetPlayState(pcmPlayerplay,SL_PLAYSTATE_PLAYING);
    if(result!=SL_RESULT_SUCCESS){
        LOGI("SetPlayState failed %ld",result);
        return -1;
    }
    //触发回调
    playerCallback(pcmBufferQueue, nullptr);

    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_openslespcmplayer_MainActivity_play(JNIEnv *env, jobject thiz, jstring file_path) {
//    const char *filename = env->GetStringUTFChars(file_path, 0);
    playPcmBySL(env,thiz,file_path);
//    env->ReleaseStringUTFChars(file_path, filename);
    return 0;
}