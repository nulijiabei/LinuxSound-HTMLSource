

import javax.sound.midi.MidiSystem;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Receiver;
import javax.sound.midi.Sequencer;
import javax.sound.midi.Transmitter;
import javax.sound.midi.MidiChannel;
import javax.sound.midi.MidiDevice;
import javax.sound.midi.Synthesizer;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.SysexMessage;

import java.io.File;
import java.io.IOException;

public class MidiPlayer {

    private DisplayReceiver receiver;

    public  void playMidiFile(String strFilename) throws Exception {
	File	midiFile = new File(strFilename);

	/*
	 *	We try to get a Sequence object, loaded with the content
	 *	of the MIDI file.
	 */
	Sequence	sequence = null;
	try {
	    sequence = MidiSystem.getSequence(midiFile);
	}
	catch (InvalidMidiDataException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	catch (IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	if (sequence == null) {
	    out("Cannot retrieve Sequence.");
	} else {  
	    String wmaFilename = strFilename.replace(".kar", ".wma");
	    if (new File(wmaFilename).exists()) {
		Process p  = 
		    Runtime.getRuntime().exec(new String[] {"bash", "playMplayer", wmaFilename});
		new Thread(new WaitForWMA(p)).start();
	    }
	    
	    SequenceInformation.setSequence(sequence);
	    playMidi(sequence);
	}
    }

    class WaitForWMA implements Runnable {
	Process p;
	
	WaitForWMA(Process p) {
	    this.p = p;
	}
	public void run() {
	    try {
		p.waitFor();
	    } catch(Exception e) {

	    }
	    System.exit(0);
	}
    }

    public  void playMidi(Sequence sequence) throws Exception {

        Sequencer sequencer = MidiSystem.getSequencer(true);
        sequencer.open();
        sequencer.setSequence(sequence); 

	receiver = new DisplayReceiver(sequencer);
	sequencer.addMetaEventListener(receiver);

	if (sequencer instanceof Synthesizer) {
	    Debug.println("Sequencer is also a synthesizer");
	} else {
	    Debug.println("Sequencer is not a synthesizer");
	}

        sequencer.start();
     }


    public DisplayReceiver getReceiver() {
	return receiver;
    }

    private static void out(String strMessage)
    {
	System.out.println(strMessage);
    }
}