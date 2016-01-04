/*
 *	SimpleMidiPlayer.java
 *
 *	This file is part of jsresources.org
 */

/*
 *      AdjustableMidiPlayer.java
 *      can change the speed and pitch of the MIDI player
 *      Copyright (c) 2016 Jan Newmarch
 */

/*
 * Copyright (c) 1999 - 2001 by Matthias Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
  |<---            this code is formatted to fit into 80 columns             --->|
*/

import java.io.File;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.MetaEventListener;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.Sequence;
import javax.sound.midi.Sequencer;
import javax.sound.midi.Synthesizer;
import javax.sound.midi.Receiver;
import javax.sound.midi.Transmitter;
import javax.sound.midi.MidiChannel;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.MidiMessage;

public class AdjustableMidiPlayer
{
    private Sequencer	sm_sequencer = null;
    private Synthesizer	sm_synthesizer = null;
    private PitchReceiver pitchReceiver = null;


    public static void main(String[] args)
    {

	if (args.length == 0 || args[0].equals("-h"))
	    {
		printUsageAndExit();
	    }


	String	strFilename = args[0];
	File	midiFile = new File(strFilename);

	AdjustableMidiPlayer amp = new AdjustableMidiPlayer();
	amp.play(midiFile);
    }

    public void play(File midiFile) {
	Sequence	sequence = null;
	try
	    {
		sequence = MidiSystem.getSequence(midiFile);
	    }
	catch (InvalidMidiDataException e)
	    {
		/*
		 *	In case of an exception, we dump the exception
		 *	including the stack trace to the console.
		 *	Then, we exit the program.
		 */
		e.printStackTrace();
		System.exit(1);
	    }
	catch (IOException e)
	    {

		e.printStackTrace();
		System.exit(1);
	    }

	try
	    {
		sm_sequencer = MidiSystem.getSequencer(false);
	    }
	catch (MidiUnavailableException e)
	    {
		e.printStackTrace();
		System.exit(1);
	    }

	if (sm_sequencer == null)
	    {
		out("SimpleMidiPlayer.main(): can't get a Sequencer");
		System.exit(1);
	    }

	sm_sequencer.addMetaEventListener(new MetaEventListener()
	    {
		public void meta(MetaMessage event)
		{
		    if (event.getType() == 47)
			{
			    sm_sequencer.close();
			    if (sm_synthesizer != null)
				{
				    sm_synthesizer.close();
				}
			    System.exit(0);
			}
		}
	    });

	try
	    {
		sm_sequencer.open();
	    }
	catch (MidiUnavailableException e)
	    {
		e.printStackTrace();
		System.exit(1);
	    }

	try
	    {
		sm_sequencer.setSequence(sequence);
	    }
	catch (InvalidMidiDataException e)
	    {
		e.printStackTrace();
		System.exit(1);
	    }

	Receiver	synthReceiver = null;
	if (! (sm_sequencer instanceof Synthesizer))
	    {

		try
		    {
			sm_synthesizer = MidiSystem.getSynthesizer();
			sm_synthesizer.open();
			synthReceiver = sm_synthesizer.getReceiver();
			Transmitter	seqTransmitter = sm_sequencer.getTransmitter();
			pitchReceiver = new PitchReceiver(synthReceiver);
			//seqTransmitter.setReceiver(synthReceiver);
			seqTransmitter.setReceiver(pitchReceiver);
		    }
		catch (MidiUnavailableException e)
		    {
			e.printStackTrace();
		    }
	    }


	sm_sequencer.start();

	/*
	try {
	    Thread.sleep(5000);
	} catch (InterruptedException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	}
	*/

	MidiChannel[] channels = sm_synthesizer.getChannels(); 
	System.out.println("Num channels is " + channels.length);
	for (int i = 0; i < channels.length; i++) {
	    channels[i].controlChange(7, 1);
	}

	//	Receiver synthReceiver = null;
	try {
	    synthReceiver = MidiSystem.getReceiver();
	} catch (Exception e) {
	}
	ShortMessage volMessage = new ShortMessage();
	int midiVolume = 100;
	for (Receiver rec: sm_synthesizer.getReceivers()) {
	    System.out.println("Setting vol on recveiver " + rec.toString());
	    for (int i = 0; i < channels.length; i++) {
		try {
		    // volMessage.setMessage(ShortMessage.CONTROL_CHANGE, i, 123, midiVolume);
		    volMessage.setMessage(ShortMessage.CONTROL_CHANGE, i, 7, midiVolume);
		} catch (InvalidMidiDataException e) {}
		synthReceiver.send(volMessage, -1);
		rec.send(volMessage, -1);
	    }
	}


	BufferedReader br = new BufferedReader(new
					       InputStreamReader(System.in));
	String str = null;
	System.out.println("Enter lines of text.");
	System.out.println("Enter 'stop' to quit.");
	do {
	    try {
		str = br.readLine();
		if (str.length() >= 2) {
		    byte[] bytes = str.getBytes();
		    if (bytes[0] == 27 && bytes[1] == 91) {
			if (bytes[2] == 65) {
			    // up
			    increasePitch();
			} else if (bytes[2] == 66) {
			    // down
			    decreasePitch();
			} else if (bytes[2] == 67) {
			    //right
			    faster();
			} else if (bytes[2] == 68) {
			    //left
			    slower();
			}
		    }
		}
	    } catch(java.io.IOException e) {
	    }
	} while(!str.equals("stop")); 
    }

    private void increasePitch() {
	pitchReceiver.increasePitch();
    }

    private void decreasePitch() {
	pitchReceiver.decreasePitch();
    }

    float speed = 1.0f;

    private void faster() {
	speed *= 1.2f;
	sm_sequencer.setTempoFactor(speed);
    }

    private void slower() {
	speed /= 1.2f;
	sm_sequencer.setTempoFactor(speed);
    }

    private static void printUsageAndExit()
    {
	out("SimpleMidiPlayer: usage:");
	out("\tjava SimpleMidiPlayer <midifile>");
	System.exit(1);
    }



    private static void out(String strMessage)
    {
	System.out.println(strMessage);
    }

    class PitchReceiver implements Receiver {
	private Receiver receiver;
	private int pitch = 0;
	

	public PitchReceiver(Receiver receiver) {
	    this.receiver = receiver;
	}

	public void send(MidiMessage message,
			 long timeStamp) {
	    if (message instanceof ShortMessage) {
		ShortMessage msg = (ShortMessage) message;
		if (msg.getCommand() == ShortMessage.NOTE_ON && pitch != 0) {
		    System.out.println("Note on");
		    try {
			msg.setMessage(msg.getCommand(),
				       msg.getChannel(),
				       msg.getData1() + pitch,
				       msg.getData2());
		    } catch (Exception e) {
		    }
		} else if  (msg.getCommand() == ShortMessage.NOTE_OFF && pitch != 0) {
		    System.out.println("Note off");
		    try {
			msg.setMessage(msg.getCommand(),
				       msg.getChannel(),
				       msg.getData1() - pitch,
				       msg.getData2());
		    } catch (Exception e) {
		    }
		}
	    }
	    receiver.send(message, timeStamp);
	}

	public void close() {
	    receiver.close();
	}

	public void increasePitch() {
	    pitch += 1;
	}

	public void decreasePitch() {
	    pitch -= 1;
	}

    }
}



/*** AdjustableMidiPlayer.java ***/
