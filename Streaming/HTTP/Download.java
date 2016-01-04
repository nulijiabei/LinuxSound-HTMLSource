

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;
import java.io.IOException;
import java.io.*;

public class Download {

    public static void main(String[] args) {
	if (args.length != 1) {
	    System.err.println("Usage: java Downlaod URL");
	    System.exit(1);
	}

	URL url = null;
	try {
	    url = new URL(args[0]);
	} catch( MalformedURLException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	URLConnection connection = null;
	try {
	    connection = url.openConnection();
	} catch(IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	
	String type = connection.getContentType();
	System.out.println("Type " + type);

	InputStream is = connectionStream();
	ByteInputStream bis = new ByteInputStream();
	//while 
    }
}