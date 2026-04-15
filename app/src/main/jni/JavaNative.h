//
// Created by Ascarre on 12-04-2026.
//

#pragma once

#include "Dumper.h"

int GetApiVersion(JNIEnv* env) {
    jclass build_version_class = env->FindClass("android/os/Build$VERSION");
    jfieldID sdk_int_field = env->GetStaticFieldID(build_version_class, "SDK_INT", "I");
    int sdk_int = env->GetStaticIntField(build_version_class, sdk_int_field);
    env->DeleteLocalRef(build_version_class);
    return sdk_int;
}

void setText(JNIEnv *env, jobject obj, const char *text) {
    jclass html = (*env).FindClass("android/text/Html");
    jmethodID fromHtml = (*env).GetStaticMethodID(html, "fromHtml","(Ljava/lang/String;)Landroid/text/Spanned;");

    jclass textView = (*env).FindClass("android/widget/TextView");
    jmethodID setText = (*env).GetMethodID(textView, "setText", "(Ljava/lang/CharSequence;)V");

    jstring jstr = (*env).NewStringUTF(text);
    (*env).CallVoidMethod(obj, setText, (*env).CallStaticObjectMethod(html, fromHtml, jstr));
}

void MakeToast(JNIEnv *env, jobject thiz, const char *text, int length) {
    jstring ToastText = env->NewStringUTF(text);
    jclass ToastClass = env->FindClass("android/widget/Toast");
    jmethodID ToastMethod = env->GetStaticMethodID(ToastClass, "makeText","(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    jobject ToastObject = env->CallStaticObjectMethod(ToastClass, ToastMethod, thiz, ToastText,length);
    jmethodID ToastMethodShow = env->GetMethodID(ToastClass, "show", "()V");
    env->CallVoidMethod(ToastObject, ToastMethodShow);
}

void startService(JNIEnv *env, jobject ctx, const char* className) {
    jclass native_context = env->GetObjectClass(ctx);
    jclass intentClass = env->FindClass("android/content/Intent");
    jclass actionString = env->FindClass(className);
    jmethodID newIntent = env->GetMethodID(intentClass, "<init>","(Landroid/content/Context;Ljava/lang/Class;)V");
    jobject intent = env->NewObject(intentClass, newIntent, ctx, actionString);
    jmethodID startActivityMethodId = env->GetMethodID(native_context, "startService","(Landroid/content/Intent;)Landroid/content/ComponentName;");
    env->CallObjectMethod(ctx, startActivityMethodId, intent);
}

void startActivityPermission(JNIEnv *env, jobject ctx) {
    jclass native_context = env->GetObjectClass(ctx);
    jmethodID startActivity = env->GetMethodID(native_context, "startActivity","(Landroid/content/Intent;)V");

    jmethodID pack = env->GetMethodID(native_context, "getPackageName", "()Ljava/lang/String;");
    jstring packageName = static_cast<jstring>(env->CallObjectMethod(ctx, pack));

    const char *pkg = env->GetStringUTFChars(packageName, 0);

    std::stringstream pkgg;
    pkgg << "package:";
    pkgg << pkg;
    std::string pakg = pkgg.str();

    jclass Uri = env->FindClass("android/net/Uri");
    jmethodID Parce = env->GetStaticMethodID(Uri, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    jobject UriMethod = env->CallStaticObjectMethod(Uri, Parce, env->NewStringUTF(pakg.c_str()));

    jclass intentclass = env->FindClass("android/content/Intent");
    jmethodID newIntent = env->GetMethodID(intentclass, "<init>","(Ljava/lang/String;Landroid/net/Uri;)V");
    jobject intent = env->NewObject(intentclass, newIntent, env->NewStringUTF("android.settings.action.MANAGE_OVERLAY_PERMISSION"), UriMethod);

    env->CallVoidMethod(ctx, startActivity, intent);
}

void SetDumpLocation(JNIEnv *env, jobject ctx) {
    jclass contextClass = env->GetObjectClass(ctx);
    jmethodID getExtFilesDirMethod = env->GetMethodID(contextClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
    jobject fileObj = env->CallObjectMethod(ctx, getExtFilesDirMethod, nullptr);
    if (fileObj == nullptr) return;
    jclass fileClass = env->GetObjectClass(fileObj);
    jmethodID getPathMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

    jstring pathStr = (jstring)env->CallObjectMethod(fileObj, getPathMethod);

    const char* pathChars = env->GetStringUTFChars(pathStr, nullptr);
    std::string basePath(pathChars);
    env->ReleaseStringUTFChars(pathStr, pathChars);

    Menu.DumpLocation = basePath + "/Dumped/" + Memory.GameName;

    if (!fs::exists(Menu.DumpLocation)) {
        if (!fs::create_directories(Menu.DumpLocation)) {
            std::cerr << "[!] Failed to create directory: " << Menu.DumpLocation << std::endl;
        }
    }

    env->DeleteLocalRef(fileObj);
    env->DeleteLocalRef(pathStr);
}

void CheckPermissionStartOverlay(JNIEnv *env, jobject ctx) {
    int sdkVer = GetApiVersion(env);
    if (sdkVer >= 23) {
        jclass Settings = env->FindClass("android/provider/Settings");
        jmethodID canDraw = env->GetStaticMethodID(Settings, "canDrawOverlays","(Landroid/content/Context;)Z");
        if (!env->CallStaticBooleanMethod(Settings, canDraw, ctx)) {
            MakeToast(env, ctx, "Please Give Overlay Permission to start Mod Menu", 1);
            startActivityPermission(env, ctx);

            sleep(1);
            exit(0);
        }
    }
    sleep(1);
    MakeToast(env, ctx, "Made By Ascarre", 1);
    SetDumpLocation(env, ctx);
    startService(env, ctx, "com/bizzra/dumper/FloatingService");
};