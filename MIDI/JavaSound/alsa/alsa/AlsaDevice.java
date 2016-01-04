package alsa;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.Receiver;
import javax.sound.midi.Transmitter;
import javax.sound.midi.MidiUnavailableException;

import java.util.List;
import java.util.ArrayList;

public class AlsaDevice implements MidiDevice {

    protected final static MidiDevice.Info info = new Info();
    private Receiver receiver = new AlsaReceiver();

    private static class Info extends MidiDevice.Info {
	
	public Info() {
	    super("ALSA device", "Jan Newmarch", "ALSA receiver", "1.0");   
	}
    }

    public void close() {
	receiver.close();
    }

    public MidiDevice.Info getDeviceInfo() {
	return info;
    }

    public int getMaxReceivers() {
	return 1;
    }

    public int getMaxTransmitters() {
	return 0;
    }
    
    public long getMicrosecondPosition() {
	return 0;
    }

    public Receiver getReceiver() {
	return receiver;
    }

    public List<Receiver> getReceivers() {
	ArrayList<Receiver> receivers = new ArrayList<Receiver>();
	receivers.add(receiver);
	return receivers;
    }

    public Transmitter getTransmitter() throws MidiUnavailableException {
	throw new MidiUnavailableException("No transmitter available");
    }

    public List<Transmitter> getTransmitters() {
	return new ArrayList<Transmitter>();
    }

    public boolean isOpen() {
	return false;
    }

    public void open() {
	
    }
}
