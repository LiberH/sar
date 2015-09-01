package risksim;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class LogFile {

	private static File file;
	private static boolean inited;
	
	public static void init(String name) throws IOException {
		if (LogFile.inited) return;
		
		LogFile.file = new File(name);
		LogFile.file.createNewFile();
		LogFile.inited = true;
	}
	
	public static void write(String str) throws IOException {
		FileOutputStream fos = new FileOutputStream(LogFile.file, true);
		fos.write(str.getBytes()); 
		fos.close(); 
	}
	
}
