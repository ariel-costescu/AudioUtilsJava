#include <jni.h>
#include "ChromaPrint.h"

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>

#if defined(HAVE_SWRESAMPLE)
#include <libswresample/swresample.h>
#elif defined(HAVE_AVRESAMPLE)
#include <libavresample/avresample.h>
#endif

#include <chromaprint.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 28, 0)
#define avcodec_free_frame av_freep
#endif

int decode_audio_file(ChromaprintContext *chromaprint_ctx, const char *file_name, int max_length, int *duration)
{
	int ok = 0, remaining, length, consumed, codec_ctx_opened = 0, got_frame, stream_index;
	AVFormatContext *format_ctx = NULL;
	AVCodecContext *codec_ctx = NULL;
	AVCodec *codec = NULL;
	AVStream *stream = NULL;
	AVFrame *frame = NULL;
#if defined(HAVE_SWRESAMPLE)
	SwrContext *convert_ctx = NULL;
#elif defined(HAVE_AVRESAMPLE)
	AVAudioResampleContext *convert_ctx = NULL;
#else
	void *convert_ctx = NULL;
#endif
	int max_dst_nb_samples = 0, dst_linsize = 0;
	uint8_t *dst_data[1] = { NULL };
	uint8_t **data;
	AVPacket packet;

	if (!strcmp(file_name, "-")) {
		file_name = "pipe:0";
	}
	
	int av_return_code = avformat_open_input(&format_ctx, file_name, NULL, NULL);
	char errbuf[100];
	if (av_return_code != 0) {
		av_strerror(av_return_code, errbuf, sizeof(errbuf));
		av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't open the file [%s][%s]\n", file_name, errbuf);
		goto done;
	}

	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't find stream information in the file [%s]\n", file_name);
		goto done;
	}

	stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (stream_index < 0) {
		av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't find any audio stream in the file [%s]\n", file_name);
		goto done;
	}

	stream = format_ctx->streams[stream_index];

	codec_ctx = stream->codec;
	codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't open the codec [%s]\n", file_name);
		goto done;
	}
	codec_ctx_opened = 1;

	if (codec_ctx->channels <= 0) {
		av_log(NULL, AV_LOG_ERROR, "ERROR: no channels found in the audio stream [%s]\n", file_name);
		goto done;
	}

	if (codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
		int64_t channel_layout = codec_ctx->channel_layout;
		if (!channel_layout) {
			channel_layout = av_get_default_channel_layout(codec_ctx->channels);
		}
#if defined(HAVE_SWRESAMPLE)
		convert_ctx = swr_alloc_set_opts(NULL,
			channel_layout, AV_SAMPLE_FMT_S16, codec_ctx->sample_rate,
			channel_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
			0, NULL);
		if (!convert_ctx) {
			av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't allocate audio converter [%s]\n", file_name);
			goto done;
		}
		if (swr_init(convert_ctx) < 0) {
			av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't initialize the audio converter [%s]\n", file_name);
			goto done;
		}
#elif defined(HAVE_AVRESAMPLE)
		convert_ctx = avresample_alloc_context();
		av_opt_set_int(convert_ctx, "out_channel_layout", channel_layout, 0);
		av_opt_set_int(convert_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
		av_opt_set_int(convert_ctx, "out_sample_rate", codec_ctx->sample_rate, 0);
		av_opt_set_int(convert_ctx, "in_channel_layout", channel_layout, 0);
		av_opt_set_int(convert_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
		av_opt_set_int(convert_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
		if (!convert_ctx) {
			av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't allocate audio converter [%s]\n", file_name);
			goto done;
		}
		if (avresample_open(convert_ctx) < 0) {
			av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't initialize the audio converter [%s]\n", file_name);
			goto done;
		}
#else
		av_log(NULL, AV_LOG_ERROR, "ERROR: unsupported audio format (please build fpcalc with libswresample) [%s]\n", file_name);
		goto done;
#endif
	}

	if (stream->duration != AV_NOPTS_VALUE) {
		*duration = stream->time_base.num * stream->duration / stream->time_base.den;
	}
	else if (format_ctx->duration != AV_NOPTS_VALUE) {
		*duration = format_ctx->duration / AV_TIME_BASE;
	}
	else {
		av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't detect the audio duration [%s]\n", file_name);
		goto done;
	}

	remaining = max_length * codec_ctx->channels * codec_ctx->sample_rate;
	chromaprint_start(chromaprint_ctx, codec_ctx->sample_rate, codec_ctx->channels);

	frame = avcodec_alloc_frame();

	while (1) {
		if (av_read_frame(format_ctx, &packet) < 0) {
			break;
		}

		if (packet.stream_index == stream_index) {
			avcodec_get_frame_defaults(frame);

			got_frame = 0;
			consumed = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
			if (consumed < 0) {
				av_log(NULL, AV_LOG_WARNING, "WARNING: error decoding audio [%s]\n", file_name);
				continue;
			}

			if (got_frame) {
				data = frame->data;
				if (convert_ctx) {
					if (frame->nb_samples > max_dst_nb_samples) {
						av_freep(&dst_data[0]);
						if (av_samples_alloc(dst_data, &dst_linsize, codec_ctx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0) {
							av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't allocate audio converter buffer [%s]\n", file_name);
							goto done;
						}
						max_dst_nb_samples = frame->nb_samples;
					}
#if defined(HAVE_SWRESAMPLE)
					if (swr_convert(convert_ctx, dst_data, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples) < 0)
#elif defined(HAVE_AVRESAMPLE)
					if (avresample_convert(convert_ctx, dst_data, 0, frame->nb_samples, (uint8_t **)frame->data, 0, frame->nb_samples) < 0)
#endif
					{
						av_log(NULL, AV_LOG_ERROR, "ERROR: couldn't convert the audio [%s]\n", file_name);
						goto done;
					}
					data = dst_data;
				}
				length = MIN(remaining, frame->nb_samples * codec_ctx->channels);
				if (!chromaprint_feed(chromaprint_ctx, data[0], length)) {
					goto done;
				}

				if (max_length) {
					remaining -= length;
					if (remaining <= 0) {
						goto finish;
					}
				}
			}
		}
		av_free_packet(&packet);
	}

finish:
	if (!chromaprint_finish(chromaprint_ctx)) {
		av_log(NULL, AV_LOG_ERROR, "ERROR: fingerprint calculation failed [%s]\n", file_name);
		goto done;
	}

	ok = 1;

done:
	if (frame) {
		avcodec_free_frame(&frame);
	}
	if (dst_data[0]) {
		av_freep(&dst_data[0]);
	}
	if (convert_ctx) {
#if defined(HAVE_SWRESAMPLE)
		swr_free(&convert_ctx);
#elif defined(HAVE_AVRESAMPLE)
		avresample_free(&convert_ctx);
#endif
	}
	if (codec_ctx_opened) {
		avcodec_close(codec_ctx);
	}
	if (format_ctx) {
		avformat_close_input(&format_ctx);
	}
	return ok;
}

static jclass classInteger;
static jmethodID midIntegerInit;
static const int algo = CHROMAPRINT_ALGORITHM_DEFAULT;
static const int max_length = 120;

JNIEXPORT jobjectArray JNICALL Java_ChromaPrint_getFingerprint(JNIEnv *env, jobject thisObj, jstring path) {
	const char *pathCStr = (*env)->GetStringUTFChars(env, path, NULL);
	
	int duration = 0;
	char *fingerprint = "";
	ChromaprintContext *chromaprint_ctx = chromaprint_new(algo);
	
	if (!decode_audio_file(chromaprint_ctx, pathCStr, max_length, &duration)) {
		chromaprint_free(chromaprint_ctx);
		return NULL;
	}
	
	if (!chromaprint_get_fingerprint(chromaprint_ctx, &fingerprint)) {
		chromaprint_free(chromaprint_ctx);
		return NULL;
	}
	
	jclass classString = (*env)->FindClass(env, "java/lang/Object");
   	jobjectArray outJNIArray = (*env)->NewObjectArray(env, 2, classString, NULL);
 	(*env)->SetObjectArrayElement(env, outJNIArray, 0, (*env)->NewStringUTF(env, fingerprint));

	jclass classIntegerLocal;
	jmethodID midIntegerInitLocal;
 	if (NULL == classInteger) {
	  classIntegerLocal = (*env)->FindClass(env, "java/lang/Integer");
	  classInteger = (*env)->NewGlobalRef(env, classIntegerLocal);
    }
 	if (NULL == classInteger) return NULL;
    if (NULL == midIntegerInit) {
	  midIntegerInitLocal = (*env)->GetMethodID(env, classInteger, "<init>", "(I)V");
	  midIntegerInit = (*env)->NewGlobalRef(env, midIntegerInitLocal);
    }
    if (NULL == midIntegerInit) return NULL;
    jobject newObj = (*env)->NewObject(env, classInteger, midIntegerInit, duration);
 	(*env)->SetObjectArrayElement(env, outJNIArray, 1, newObj);

	chromaprint_free(chromaprint_ctx);
	free(fingerprint);

	return outJNIArray;
}

JNIEXPORT void JNICALL Java_ChromaPrint_init(JNIEnv *env, jobject thisObj) {
	av_register_all();
	av_log_set_level(AV_LOG_ERROR);
}
