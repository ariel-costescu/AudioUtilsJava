#include <jni.h>
#include <iostream>
#include <string>

#include "TagLibReader.h"

#include <fileref.h>
#include <tag.h>

using namespace std;

JNIEXPORT jobjectArray JNICALL Java_TagLibReader_getTags (JNIEnv *env, jobject thisObj, jstring path) {
	const jchar *pathCStr = env->GetStringChars(path, NULL);
	TagLib::FileRef f(reinterpret_cast<const wchar_t*>(pathCStr));
	jstring jartist;
	jstring jalbum;
	jstring jtitle;
	if(!f.isNull() && f.tag()) {
      TagLib::Tag *tag = f.tag();
      jartist = env->NewStringUTF(tag->artist().toCString(true));
      jalbum = env->NewStringUTF(tag->album().toCString(true));
      jtitle = env->NewStringUTF(tag->title().toCString(true));
    }
    else {
    	return NULL;
    }
	jclass classString = env->FindClass("java/lang/String");
   	jobjectArray outJNIArray = env->NewObjectArray(3, classString, NULL);
 	env->SetObjectArrayElement(outJNIArray, 0, jartist);
 	env->SetObjectArrayElement(outJNIArray, 1, jalbum);
 	env->SetObjectArrayElement(outJNIArray, 2, jtitle);
	return outJNIArray;
}

JNIEXPORT jstring JNICALL Java_TagLibReader_getArtist (JNIEnv *env, jobject thisObj, jstring path) {
	const jchar *pathCStr = env->GetStringChars(path, NULL);
	TagLib::FileRef f(reinterpret_cast<const wchar_t*>(pathCStr));
	if(!f.isNull() && f.tag()) {
      TagLib::Tag *tag = f.tag();
      return env->NewStringUTF(tag->artist().toCString(true));
    }
    else {
    	return NULL;
    }
}

JNIEXPORT jstring JNICALL Java_TagLibReader_getAlbum (JNIEnv *env, jobject thisObj, jstring path) {
	const jchar *pathCStr = env->GetStringChars(path, NULL);
	TagLib::FileRef f(reinterpret_cast<const wchar_t*>(pathCStr));
	if(!f.isNull() && f.tag()) {
      TagLib::Tag *tag = f.tag();
	  return env->NewStringUTF(tag->album().toCString(true));    
	}
    else {
    	return NULL;
    }
}

JNIEXPORT jstring JNICALL Java_TagLibReader_getTitle (JNIEnv *env, jobject thisObj, jstring path) {
	const jchar *pathCStr = env->GetStringChars(path, NULL);
	TagLib::FileRef f(reinterpret_cast<const wchar_t*>(pathCStr));
	if(!f.isNull() && f.tag()) {
      TagLib::Tag *tag = f.tag();
      return env->NewStringUTF(tag->title().toCString(true));
    }
    else {
    	return NULL;
    }
}