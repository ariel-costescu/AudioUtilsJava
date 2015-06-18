
public class ChromaPrint {
	final static String audioUtilsLocation;
	
	static {
		audioUtilsLocation = System.getenv("AUDIO_UTILS_LOCATION");
		System.load("c:\\windows\\system32\\AVCODEC-56.DLL");
		System.load("c:\\windows\\system32\\AVFORMAT-56.DLL");
		System.load("c:\\windows\\system32\\AVUTIL-54.DLL");
		System.load("c:\\windows\\system32\\KERNEL32.DLL");
		System.load("c:\\windows\\system32\\MSVCRT.DLL");
		System.load("c:\\windows\\system32\\SHELL32.DLL");
		System.load("C:\\chromaprint\\bin\\libchromaprint.dll");
		System.load(audioUtilsLocation + "\\src\\c\\ChromaPrint.dll");
	}

	public native String[] getFingerprint(String path);

	public static void main(String[] args) {
		ChromaPrint c = new ChromaPrint();
		String path = "C:\\Users\\Public\\Music\\Sample Music\\Kalimba.mp3";
		String[] f = c.getFingerprint(path);
		System.out.println(f[0]);
		System.out.println(f[1]);
	}
}
