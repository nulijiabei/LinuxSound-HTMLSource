

import java.io.FileInputStream;
import java.io.*;
import java.nio.charset.Charset;



class SongTitles {
    //private static int BASE = 0x5F23A;
    private static long OFFSET = 1; // must be >= 1 or blows array bound
    private static long INDEX_BASE = 0x800 + OFFSET;
    private static long TITLE_BASE = 0x5F000;
    private static int NUM = 12000;
    private static int INDEX_SIZE = 25;
    private static int TITLE_OFFSET = (int) (19 - OFFSET);
    private static int LENGTH_TITLE = (int) (25 - OFFSET);
    private static int ARTIST_OFFSET = (int) (21 - OFFSET);
    private static int SONG_INDEX = (int) (15 - OFFSET);

    public static void main(String[] args) throws Exception {
	long nread = INDEX_BASE;
	byte[] titleBytes = new byte[512];
 	FileInputStream fstream = new FileInputStream("MALATAS4.IDX");
	fstream.skip(INDEX_BASE);

	byte[][] indexes = new byte[NUM][];
	long titleStart[] = new long[NUM];
	long titleEnd[] = new long[NUM];
	for (int n = 0; n < NUM; n++) {
	    indexes[n] = new byte[INDEX_SIZE];
	    fstream.read(indexes[n]);
	    if (isNull(indexes[n]))
		break;
	    // printIndex(indexes[n]);
	    nread += INDEX_SIZE;

	    if (isNull(indexes[n]))
		break;

	    byte b1 = indexes[n][TITLE_OFFSET];
	    byte b2 = indexes[n][TITLE_OFFSET+1];
	    // corect for negative
	    int first, second;
	    //System.out.printf("%X %X\n", firstB, secondB);
	    //first = firstB >= 0 ? firstB : 256 - firstB;
	    //second = secondB >= 0 ? secondB : 256 - secondB;

	    titleStart[n] = ((b1 >= 0 ? b1 : 256 + b1) << 8) + (b2 >= 0 ? b2 : 256 + b2); //first * 256 + second;
	    if (titleStart[n] > 0xfff) {
		// System.out.println("too big");
		titleStart[n] &= 0xfff;
	    } else if (titleStart[n] < 0) {
		System.out.println("too small");
	    }

	    long end = indexes[n][LENGTH_TITLE];
	    if (end >= 0x80) {
		end -= 0x80;
		// System.out.println("End too big");
	    } else if (end < 0) {
		// System.out.println("End negative");
		end += 128;
	    }
	    titleEnd[n] = end;

	    /*
	    System.out.printf("Numbers %X %X %X %X\n", b1, b2, 
			      titleStart[n],
			      titleStart[n]+titleEnd[n]);
	    */
	}

	System.out.printf("Skip to %X\n", (TITLE_BASE - nread));
	fstream.skip(TITLE_BASE - nread);
	nread = TITLE_BASE;

	for (int n = 0; n < NUM-1; n++) {
	    // int len = (int) (titleStart[n+1]-titleStart[n]);
	    int len = (int) titleEnd[n];
	    //System.out.println("Reading " + len);
	    fstream.read(titleBytes, 0, len);

	    Charset charset = Charset.forName("gb2312");
	    String translated = new String(titleBytes, 0, len, charset);

	    printFullIndex(indexes[n]);
	    System.out.print(" SongIndex ");
	    printSongIndex(indexes[n]);
	    System.out.print(" ArtistIndex ");
	    printArtistIndex(indexes[n]);
	    System.out.println("" + n + ": " +translated);
	    //printIndex(indexes[n]);
	}

	/*
	fstream.skip(TITLE_BASE);

	byte[] bytes = new byte[NUM];
	fstream.read(bytes);

	Charset charset = Charset.forName("gb2312");
	String translated = new String(bytes, charset);

	System.out.println(translated);
	*/
    }

    private static void printFullIndex(byte[] bytes) {
	for (int n = 0; n < bytes.length; n++) {
	    System.out.printf("%02X ", bytes[n]);
	}
    }

    private static void printArtistIndex(byte[] bytes) {
	System.out.printf("%02X%02X ", 
			  bytes[ARTIST_OFFSET], 
			  bytes[ARTIST_OFFSET+1]);
    }

    private static void printSongIndex(byte[] bytes) {
	System.out.printf("%X%02X%02X ", 
			  bytes[SONG_INDEX], 
			  bytes[SONG_INDEX+1], 
			  bytes[SONG_INDEX+2]);
    }
    
    private static void print1byte(byte[] bytes, int index) {
	System.out.printf("%02X ", bytes[index]);
    }
    
    private static boolean isNull(byte[] bytes) {
	for (int n = 0; n < bytes.length; n++) {
	    if (bytes[n] != 0)
		return false;
	}
	return true;
    }
}