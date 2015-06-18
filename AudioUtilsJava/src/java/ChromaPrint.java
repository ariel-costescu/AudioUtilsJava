import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.stream.Stream;


public class ChromaPrint {
	final static String audioUtilsLocation;
	
	static {
		audioUtilsLocation = System.getProperty("AUDIO_UTILS_LOCATION", System.getenv("AUDIO_UTILS_LOCATION"));
		System.load("c:\\windows\\system32\\AVCODEC-56.DLL");
		System.load("c:\\windows\\system32\\AVFORMAT-56.DLL");
		System.load("c:\\windows\\system32\\AVUTIL-54.DLL");
		System.load("c:\\windows\\system32\\KERNEL32.DLL");
		System.load("c:\\windows\\system32\\MSVCRT.DLL");
		System.load("c:\\windows\\system32\\SHELL32.DLL");
		System.load("C:\\chromaprint\\bin\\libchromaprint.dll");
		System.load(audioUtilsLocation + "\\src\\c\\ChromaPrint.dll");
	}

	public native Object[] getFingerprint(String path);

	public static void main(String[] args) {
		ChromaPrint c = new ChromaPrint();
		for (String path : args) {
			System.out.println(path);
			Object[] f = c.getFingerprint(path);
			String fp = (String)f[0];
			Integer d = (Integer)f[1];
			String url = "http://api.acoustid.org/v2/lookup?client=ULjKruIh&meta=recordings+releases&duration=" + d + "&fingerprint=" + fp;
			URL u;
			try {
				u = new URL(url);
				InputStream in = u.openStream();
				BufferedReader r = new BufferedReader(new InputStreamReader(in));
				Stream<String> lines = r.lines();
				for (Object line : lines.toArray()) {
					System.out.println(line.toString());
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
