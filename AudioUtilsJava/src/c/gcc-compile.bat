gcc -Wl,--add-stdcall-alias -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" -I"C:\chromaprint\include" -I"C:\ffmpeg\include" -L"C:\chromaprint\lib" -L"C:\ffmpeg\lib" -shared -o ChromaPrint.dll ChromaPrint.c -llibavcodec -llibavdevice -llibavfilter -llibavformat -llibavutil -llibpostproc -llibswresample -llibswscale -llibchromaprint