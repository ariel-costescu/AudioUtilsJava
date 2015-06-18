
public class TagLibReader {
	final static String audioUtilsLocation;
	
	static {
		audioUtilsLocation = System.getProperty("AUDIO_UTILS_LOCATION", System.getenv("AUDIO_UTILS_LOCATION"));
		System.load("c:\\windows\\system32\\KERNEL32.DLL");
		System.load("c:\\windows\\system32\\MSVCRT.DLL");
		System.load("c:\\windows\\system32\\LIBZLIB.DLL");
		System.load("c:\\windows\\system32\\USER32.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBWINPTHREAD-1.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBGCC_S_SEH-1.DLL");
		System.load("c:\\mingw-w64\\x86_64-5.1.0-posix-seh-rt_v4-rev0\\mingw64\\bin\\LIBSTDC++-6.DLL");
		System.load("c:\\windows\\system32\\LIBTAG.DLL");
		System.load("c:\\windows\\system32\\LIBTAG_C.DLL");
		System.load(audioUtilsLocation + "\\src\\c++\\TagLibReaderCpp.dll");
	}

	public native String[] getTags(String path);
	public native String getArtist(String path);
	public native String getAlbum(String path);
	public native String getTitle(String path);
	
	public static void main(String[] args) {
		
		TagLibReader tl = new TagLibReader();
		String path = "C:\\Users\\Public\\Music\\Sample Music\\Kalimba.mp3";
		long start = System.nanoTime();
		String[] tags = tl.getTags(path);
		long end = System.nanoTime();
		System.out.println((end - start) + " ns");
		for (String t : tags) {
			System.out.println(t);
		}
	}
}
