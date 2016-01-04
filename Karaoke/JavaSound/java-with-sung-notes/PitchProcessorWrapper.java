
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;

import be.hogent.tarsos.dsp.*;
import be.hogent.tarsos.dsp.pitch.*;
import be.hogent.tarsos.dsp.pitch.PitchProcessor.*;
import be.hogent.tarsos.dsp.util.AudioFloatConverter;

public class PitchProcessorWrapper {
    private PitchProcessor pitchProcessor;
    private int bytesProcessed = 0;
    private final AudioEvent audioEvent;
    private final AudioFloatConverter converter;
    private float[] audioFloatBuffer;
    private byte[] audioByteBuffer;

    // copied from MidiAndAu...

    
    int floatStepSize;
    int floatOverlap;
    int byteStepSize;
    int byteOverlap;

    int blocksToBeWritten = 0;
    

    public PitchProcessorWrapper(AudioInputStream audioInputStream,
				 DisplayReceiver receiver) {
	AudioFormat audioFormat = audioInputStream.getFormat();

	audioEvent = new AudioEvent(audioFormat, audioInputStream.getFrameLength());
	converter = AudioFloatConverter.getConverter(audioFormat);

	float sampleRate = audioFormat.getSampleRate();

	int floatBufferSize =  Constants.PITCH_BUFFER_SIZE;
	//floatOverlap = Constants.FRAMES_PER_BUFFER;
	//floatStepSize = floatBufferSize - floatOverlap;
	floatStepSize = Constants.FRAMES_PER_BUFFER;
	floatOverlap = floatBufferSize - floatStepSize;

	// Different from Joren's example: we have a single byte buffer that
	// we fill from the mike. We copy the buffer into the top half of the
	// float buffer each time round - we don't copy the top part of the
	// byte buffer only. So our byteOverlap is zero
	byteOverlap = 0;

	pitchProcessor = new PitchProcessor(PitchEstimationAlgorithm.YIN, 
					    sampleRate, floatBufferSize, receiver);

	audioFloatBuffer = new float[floatBufferSize];
	audioByteBuffer = new byte[floatBufferSize * audioFormat.getFrameSize()];
	//System.out.printf("Float size %d, byte size %d\n", audioFloatBuffer.length, bufferSize); 
    }

    /** 
     * Calls the pitch detector.
     *
     * We have conflicting goals:
     * - audio latency wants the buffer size to be small (< 512 bytes)
     * - detection of low notes wants the buffer size to be large (> 1024 bytes)
     * So we make the byte buffer small and re-use it on the next call to the
     * detector, giving the detector twice as much data to work on.
     * This means the detector will be behind the audio played, but visual latency
     * doesn't matter as much as audio latency
     */
    public void write(byte[] buffer, int numBytes) {

	System.arraycopy(audioFloatBuffer, floatStepSize, 
			 audioFloatBuffer, 0, floatOverlap);
	converter.toFloatArray(buffer, 
			       byteOverlap, 
			       audioFloatBuffer, 
			       //0, Constants.PITCH_BUFFER_SIZE/2);
			       floatOverlap, floatStepSize);

	/*
	audioEvent.setOverlap(floatOverlap);
	audioEvent.setFloatBuffer(audioFloatBuffer);
	audioEvent.setBytesProcessed(bytesProcessed);

	pitchProcessor.process(audioEvent);
	*/

	if (blocksToBeWritten++ == Constants.PITCH_BUFFER_SCALE) {
	    blocksToBeWritten = 0;
	    audioEvent.setOverlap(0);
	    audioEvent.setFloatBuffer(audioFloatBuffer);
	    audioEvent.setBytesProcessed(bytesProcessed);

	    pitchProcessor.process(audioEvent);
	    //bytesProcessed += numBytes;
	}
	
	bytesProcessed += numBytes;
    }
}