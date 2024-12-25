//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @file open_browser.cpp
 * Opening browser utility by Saturnin 
 */
#include "open_browser.hpp"
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

void OpenBrowser(const std::string& url){
    
    #ifndef __ANDROID__

    std::ostringstream command;

    #if defined(_WIN32)
    command<<"start ";
    #elif defined(__APPLE__)
    command<<"open ";
    #else
    command<<"xdg-open ";
    #endif
    command<<url;
    system(command.str().c_str());

    #else //Android

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetMethodID(clazz, "openBrowser", "(Ljava/lang/String;)V");
    if(method_id==0){
        throw std::runtime_error("JNI: Method \"openBrowser\" not found!");
    }

    jstring param1 = env->NewStringUTF(url.c_str());
    (jstring)env->CallObjectMethod(activity, method_id, param1);
    env->DeleteLocalRef(param1);

    #endif
}