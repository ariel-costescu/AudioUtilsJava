package com.playax.fingerprint;

public class Echoprint {
	final static String codegenLocation;
	
	static {
		codegenLocation = System.getProperty("CODEGEN_LOCATION", System.getenv("CODEGEN_LOCATION"));
		System.load("c:\\windows\\system32\\KERNEL32.DLL");
		System.load("c:\\windows\\system32\\MSVCRT.DLL");
		System.load("c:\\windows\\system32\\USER32.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBWINPTHREAD-1.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBGCC_S_SEH-1.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBSTDC++-6.DLL");
		System.load("c:\\windows\\system32\\LIBTAG.DLL");
		System.load(codegenLocation + "\\lib\\libcodegen.dll");
	}
	
	public native String code(String fileName); 
	
	public static void main(String[] args) {
		String fileName = args[0];
		String code = new Echoprint().code(fileName);
		System.out.println(code);
	}
}
