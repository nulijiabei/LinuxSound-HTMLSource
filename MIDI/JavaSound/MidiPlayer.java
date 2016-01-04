

import javax.sound.midi.MidiSystem;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Track;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.SysexMessage;
import javax.sound.midi.Receiver;
import javax.sound.midi.Sequencer;
import javax.sound.midi.Transmitter;
import javax.sound.midi.MidiChannel;
import javax.sound.midi.MidiDevice;
import javax.sound.midi.Synthesizer;

import java.io.File;
import java.io.IOException;
import java.util.Vector;


public class MidiPlayer {

    public static void main(String[] args) throws Exception{
	MidiPlayer player = new MidiPlayer();
	player.playMidiFile(args[0]);
    }

    /*
    private Vector<Lyric> lyrics = new Vector<Lyric>();
    private Lyric currentLyric = null;
    private Vector<MidiEvent> melody = new Vector<MidiEvent>();
    private DisplayReceiver receiver;
    */

    public MidiPlayer() {

    }

    public  void playMidiFile(String strFilename) throws Exception {
	File	midiFile = new File(strFilename);

	/*
	 *	We try to get a Sequence object, loaded with the content
	 *	of the MIDI file.
	 */
	Sequence	sequence = null;
	try
	    {
		sequence = MidiSystem.getSequence(midiFile);
	    }
	catch (InvalidMidiDataException e)
	    {
		e.printStackTrace();
		System.exit(1);
	    }
	catch (IOException e)
	    {
		e.printStackTrace();
		System.exit(1);
	    }

	/*
	 *	And now, we output the data.
	 */
	if (sequence == null)
	    {
		out("Cannot retrieve Sequence.");
	    }
	else
	    {
		out("---------------------------------------------------------------------------");
		out("File: " + strFilename);
		out("---------------------------------------------------------------------------");
		out("Length: " + sequence.getTickLength() + " ticks");
		out("Duration: " + sequence.getMicrosecondLength() + " microseconds");
		out("---------------------------------------------------------------------------");
		float	fDivisionType = sequence.getDivisionType();
		String	strDivisionType = null;
		if (fDivisionType == Sequence.PPQ)
		    {
			strDivisionType = "PPQ";
		    }
		else if (fDivisionType == Sequence.SMPTE_24)
		    {
			strDivisionType = "SMPTE, 24 frames per second";
		    }
		else if (fDivisionType == Sequence.SMPTE_25)
		    {
			strDivisionType = "SMPTE, 25 frames per second";
		    }
		else if (fDivisionType == Sequence.SMPTE_30DROP)
		    {
			strDivisionType = "SMPTE, 29.97 frames per second";
		    }
		else if (fDivisionType == Sequence.SMPTE_30)
		    {
			strDivisionType = "SMPTE, 30 frames per second";
		    }

		out("DivisionType: " + strDivisionType);

		String	strResolutionType = null;
		if (sequence.getDivisionType() == Sequence.PPQ)
		    {
			strResolutionType = " ticks per beat";
		    }
		else
		    {
			strResolutionType = " ticks per frame";
		    }
		out("Resolution: " + sequence.getResolution() + strResolutionType);
		out("---------------------------------------------------------------------------");
		Track[]	tracks = sequence.getTracks();
		for (int nTrack = 0; nTrack < tracks.length; nTrack++)
		    {
			out("Track " + nTrack + ":");
			out("-----------------------");
			Track	track = tracks[nTrack];
			for (int nEvent = 0; nEvent < track.size(); nEvent++)
			    {
				MidiEvent	event = track.get(nEvent);
				//capture(event);
			    }
			out("---------------------------------------------------------------------------");
		    }

		playMidi(sequence);
	    }

    }

    public  void playMidi(Sequence sequence) throws Exception {

	MidiDevice.Info[] info0 = MidiSystem.getMidiDeviceInfo();  

        //Sequencer sequencer = MidiSystem.getSequencer();
        Sequencer sequencer = (Sequencer) MidiSystem.getMidiDevice(info0[1]);

	//Debug.println("Sequencer is " + sequencer.getClass().toString());
	System.out.println("Sequencer is " + sequencer.getClass().toString());
	/*
	List<Transmitter> transmitters = sequencer.getTransmitters();
	for (Transmitter t: transmitters) {
	    System.out.println("  Trans class: " + t.getClass().toString());
	    System.out.println("  Sending to class: " + t.getReceiver().getClass().toString());
	}
	*/
        sequencer.open();
        sequencer.setSequence(sequence); 
	sequencer.open();
	sequencer.start();

	/*
	receiver = new DisplayReceiver(sequencer);
	sequencer.getTransmitter().setReceiver(receiver);
	sequencer.addMetaEventListener(receiver);
        sequencer.start();



	// speed
	System.out.println("Tempo in BPM is" + sequencer.getTempoInBPM());
	System.out.println("Tempo in MPQ is" + sequencer.getTempoInMPQ());
	System.out.println("Tempo factor is" + sequencer.getTempoFactor());


	sequencer.setTempoInMPQ(Constants.SONGKEN_TEMPO_IN_MPQ);
	System.out.println("New Tempo in MPQ is" + sequencer.getTempoInMPQ());

	// change volume
	// See http://www.coderanch.com/t/272584/java/java/MIDI-volume-control-difficulties
	MidiDevice.Info[] info = MidiSystem.getMidiDeviceInfo();  
	for (int i = 0; i < info.length; i++) { 
	    MidiDevice mdev = MidiSystem.getMidiDevice(info[i]);
	    //mdev.open();
	    System.out.println("Midi device " + info[i].getName() + 
			       " open? " +mdev.isOpen());  
	}

	// Synthesizer synthesizer = MidiSystem.getSynthesizer();  
	Synthesizer synthesizer = (Synthesizer) MidiSystem.getMidiDevice(info[0]);  
	synthesizer.open();  
	System.out.println("Default synthesizer is " +
			   synthesizer.getDeviceInfo().getName());
	Receiver synthReceiver = synthesizer.getReceiver();  
	Transmitter seqTransmitter = sequencer.getTransmitter();  
	seqTransmitter.setReceiver(synthReceiver); 
	MidiChannel[] channels = synthesizer.getChannels();  
	for (int i = 0; i < channels.length; i++) {  
	    channels[i].controlChange(7, 50); 
	}

	// http://health-7.com/Killer%20Game%20Programming%20in%20Java/Audio%20Effects%20on%20MIDI%20Sequences
	System.out.println("Syntheziser Channels: " + channels.length);
	System.out.print("Volumes: {");
	for (int i=0; i < channels.length; i++)
	    System.out.print( channels[i].getController(7) + " ");
	System.out.println("}");
	*/
    }

    /*
    public  void capture(MidiEvent event)
    {
	MidiMessage	message = event.getMessage();
	long		lTicks = event.getTick();
	
	if (message instanceof ShortMessage) {
	    /*
	      System.out.println("adding event " + 
	      ((ShortMessage) message).getChannel() +
	      " tick " + lTicks);
	    
	    if (((ShortMessage) message).getChannel() == 0) {
		melody.add(event);
	    }
	} else if (message instanceof MetaMessage) {
	    if (((MetaMessage) message).getType() == 5) {
		addLyric(lTicks, (MetaMessage) message);
	    }
	}

    }

    public  void addLyric(long tick, MetaMessage message) {
	byte[] data = message.getData();
	String str = new String(data);

	if (tick == 0) {
	    return;
	}
	if ((str.equals(" ") || str.equals("\r"))) {
	    if (currentLyric != null) {
		// end of a lyric word
		currentLyric.endTick = tick;
		lyrics.add(currentLyric);
		currentLyric = null;
	    }
	    return;
	}
	
	if (currentLyric == null) {
	    currentLyric = new Lyric();
	    currentLyric.startTick = tick;
	    currentLyric.lyric = str;
	} else {
	    currentLyric.lyric = currentLyric.lyric + str;
	}
    }
    
    public DisplayReceiver getReceiver() {
	return receiver;
    }
    */
    private static void out(String strMessage)
    {
	System.out.println(strMessage);
    }
}