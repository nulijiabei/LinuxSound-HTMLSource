/*
 * KaraokePlayer.java
 *
 */

import javax.swing.*;

public class WMAPlayer {


    public static void main(String[] args) throws Exception {
	if (args.length != 1) {
	    System.err.println("WMAPlayer: usage: " +
			     "WMAPlayer <midifile>");
	    System.exit(1);
	}
	String	strFilename = args[0];

	MidiPlayer midiPlayer = new MidiPlayer();
	midiPlayer.playMidiFile(strFilename);
    }
}



