import java.io.*;
import java.net.*;

public class Client{

    public static final int DAYTIMEPORT = 8189;
    
    public static void main(String[] args){

	InetAddress address = null;
	try {
	    address = InetAddress.getByName("192.168.1.102");
	} catch(UnknownHostException e) {
	    e.printStackTrace();
	    // System.exit(2);
            return;
	}

	Socket sock = null;
	try {
	    sock = new Socket(address, DAYTIMEPORT);
	} catch(IOException e) {
	    e.printStackTrace();
	    // System.exit(3);
            return;
	}

	OutputStream out = null;
	try {
	    out = sock.getOutputStream();
	} catch(IOException e) {
	    e.printStackTrace();
	    // System.exit(5);
            return;
	}

	PrintWriter writer =  new PrintWriter(new BufferedWriter(new OutputStreamWriter(out)));
	String line = "Hello 邓丽君\n";
	try {
	    /*
	    writer.print(line);
	    writer.flush();
	    writer.close();
	    */
	    byte[] bytes = line.getBytes();
	    for (int n = 0; n < bytes.size; n++) {
		System.out.print( " " + bytes[n]);
	    }
	    System.out.println();
	    DataOutputStream dout = new DataOutputStream(socket.getOutputStream());
	    dout.write(bytes);
	    dout.write('\n');
	    dout.flush();
	    dout.close();

	    sock.close();
	} catch(IOException e) {
	    e.printStackTrace();
	    // System.exit(6);
            return;
	}


	System.out.println(line);
	// System.exit(0);
        return;
    }
} // Client
