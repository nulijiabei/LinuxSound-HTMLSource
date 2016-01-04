/**
 * DisplayReceiver
 *
 * Acts as a Midi receiver to the default Java Midi sequencer.
 * It collects Midi events and Midi meta messages from the sequencer.
 * these are handed to a UI object for display.
 *
 * The current UI object is a MidiGUI but could be replaced.
 *
 * In addition, it acts as a handler for events from the PitchDetector
 * and passes these to the UI for display as well.
 *
 * Klunky bit: the events received from the sequencer are for immediate
 * play by this receiver, and as per the Midi spec all events have a
 * timestamp of -1.
 * But we have to match pitch events from the microphone to the 
 * midi events so the UI can draw them properly, and we can only
 * do that if we know where in the Midi sequence the
 * sequencer has reached. So (klunk) the receiver has to be passed the
 * sequencer (that it gets messages from) so that it can work out where 
 * the pitch events occur in relation to the Midi stream. 
 */

import javax.sound.midi.*;
import javax.swing.SwingUtilities;
// import be.hogent.tarsos.dsp.pitch.PitchProcessor.*;

public class DisplayReceiver implements Receiver, 
					MetaEventListener {
    private static final int TEXT = 1;
    private MidiGUI gui;
    private Sequencer sequencer;
    private int melodyChannel = 0; // true for Songken files

    public DisplayReceiver(Sequencer sequencer) {
	this.sequencer = sequencer;
	gui = new MidiGUI(sequencer);
    }

    public void close() {
    }

    /*
    public void handlePitch(float pitch, float probability, final float timeStamp,
			    float progress) {
	if (pitch >= 0 && probability >= 0.8) { 
	    // From http://dzone.com/snippets/midi-note-number-and-frequency
	    final double midiNote = 69.0 + 
		12.0 * Math.log(pitch/440.0)/Math.log(2.0);

	    Debug.println("Pitch detected: " + midiNote + " at tick " +
			       sequencer.getTickPosition());
	    SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			gui.setSungNote(timeStamp, (float) midiNote);
		    }
		});
	}
    }
    */
					    
    public void send(MidiMessage msg, long timeStamp) {
	// Note on/off messages come from the midi player
	// but not meta messages

	if (msg instanceof ShortMessage) {
	    ShortMessage smsg = (ShortMessage) msg;

		
		String strMessage = "Channel " + smsg.getChannel() + " ";
		
		switch (smsg.getCommand())
		    {
		    case 0x80:
			strMessage += "note Off " + 
			    getKeyName(smsg.getData1()) + " " + timeStamp;
			break;
			
		    case 0x90:
			strMessage += "note On " + 
			    getKeyName(smsg.getData1()) + " " + timeStamp;
			break;
		    }
		Debug.println(strMessage);
	    if (smsg.getChannel() == melodyChannel) {
		gui.setNote(timeStamp, smsg.getCommand(), smsg.getData1());
	    }
		
	}
    }
					    

    public void meta(MetaMessage msg) {
	Debug.println("Reciever got a meta message");
	if (((MetaMessage) msg).getType() == TEXT) {
	    setLyric((MetaMessage) msg);
	}
    }

    public void setLyric(MetaMessage message) {
	byte[] data = message.getData();
	String str = new String(data);
	Debug.println("Lyric +\"" + str + "\" at " + sequencer.getTickPosition());
	gui.setLyric(str);

    }

    private static String[]	sm_astrKeyNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    public static String getKeyName(int nKeyNumber)
    {
        if (nKeyNumber > 127)
            {
                return "illegal value";
            }
        else
            {
                int     nNote = nKeyNumber % 12;
                int     nOctave = nKeyNumber / 12;
                return sm_astrKeyNames[nNote] + (nOctave - 1);
            }
    }

}
