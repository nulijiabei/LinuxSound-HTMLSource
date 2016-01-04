package alsa;

import javax.sound.midi.spi.MidiDeviceProvider;
import javax.sound.midi.MidiDevice;

public class AlsaMidiDeviceProvider extends MidiDeviceProvider {

    private static final String JNI_LIBRARY_NAME = new String("tuxguitar-alsa-jni");

    protected final static MidiDevice.Info alsainfo = AlsaDevice.info;
    
    private static MidiDevice.Info[] alsainfos = {alsainfo};
    
    static {
	//System.loadLibrary(JNI_LIBRARY_NAME);
    }
    
    public MidiDevice getDevice(MidiDevice.Info info) {
	if (info == alsainfo) {
	    return new AlsaDevice();
	}
	return null;
    }

    public MidiDevice.Info[] getDeviceInfo() {
	return alsainfos;
    }

    public boolean isDeviceSupported(MidiDevice.Info info) {
	if (info == alsainfo) {
	    return true;
	}
	return false;
    }
}
