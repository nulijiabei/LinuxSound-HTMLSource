package jan.newmarch.playmicrophone;

import java.io.FileOutputStream;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class PlayMicrophoneActivity extends Activity {
	private static final int RECORDER_BPP = 16;
	private static final int RECORD_CHANNELS = AudioFormat.CHANNEL_IN_MONO;
	private static final int PLAY_CHANNELS = AudioFormat.CHANNEL_OUT_MONO;
	private static final int AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;

	private AudioRecord recorder = null;
	private AudioTrack player = null;
	private int bufferSize = 0;
	private Thread recordingThread = null;
	private boolean isRecording = false;
	private int sampleRate;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		setButtonHandlers();
		enableButtons(false);

		sampleRate = AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_MUSIC);
		int bSize1 = AudioRecord.getMinBufferSize(sampleRate,
				RECORD_CHANNELS, AUDIO_ENCODING);
		int bSize2 = AudioTrack.getMinBufferSize(sampleRate,
				PLAY_CHANNELS, AUDIO_ENCODING);
		bufferSize =  (bSize1 > bSize2) ? bSize1 : bSize2;
		Toast toast = Toast.makeText(getApplicationContext(), 
					"Sample rate: " + sampleRate + 
						" min record buf size: " + bSize1 +
							" min play buf size: " + bSize2, 
					Toast.LENGTH_SHORT);
		toast.show();
		if (bSize1 < 0 || bSize2 < 0) {
			fatal("No microphone or no speaker");
		}
	}
	
	private void fatal(String msg) {
		/*
		Toast toast = Toast.makeText(getApplicationContext(), 
				msg, 
				Toast.LENGTH_SHORT);
		toast.show();
		*/
		System.out.println(msg);
		finish();
	}

	private void setButtonHandlers() {
		((Button) findViewById(R.id.btnStart)).setOnClickListener(btnClick);
		((Button) findViewById(R.id.btnStop)).setOnClickListener(btnClick);
	}

	private void enableButton(int id, boolean isEnable) {
		((Button) findViewById(id)).setEnabled(isEnable);
	}

	private void enableButtons(boolean isRecording) {
		enableButton(R.id.btnStart, !isRecording);
		enableButton(R.id.btnStop, isRecording);
	}

	private void startRecording() {
		try {
		
		recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
				sampleRate, RECORD_CHANNELS,
				AUDIO_ENCODING, bufferSize);
				
		player = new AudioTrack(AudioManager.STREAM_MUSIC,
				sampleRate, PLAY_CHANNELS,
				AUDIO_ENCODING, bufferSize,
				AudioTrack.MODE_STREAM);
		recorder.startRecording();
		player.play();
		} catch(Exception e) {
			fatal("Can't create recorder or player");
		}

		isRecording = true;

		recordingThread = new Thread(new Runnable() {

			// @Override
			public void run() {
				writeAudioDataToSpeaker();
			}
		}, "AudioRecorder Thread");

		recordingThread.start();
	}

	private void writeAudioDataToSpeaker() {
		byte data[] = new byte[bufferSize];

		int read = 0;

		while (isRecording) {
			read = recorder.read(data, 0, bufferSize);

			if (AudioRecord.ERROR_INVALID_OPERATION != read) {
				player.write(data, 0, read);
			}
		}			
	}

	private void stopRecording() {
		if (null != recorder) {
			isRecording = false;

			recorder.stop();
			recorder.release();

			recorder = null;
			recordingThread = null;
			
			player.stop();
			player.release();
		}
	}

	private View.OnClickListener btnClick = new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			switch (v.getId()) {
			case R.id.btnStart: {
				enableButtons(true);
				startRecording();
				break;
			}
			case R.id.btnStop: {
				enableButtons(false);
				stopRecording();
				break;
			}
			}
			Toast toast = Toast.makeText(getApplicationContext(), 
					"Btn clicked handler called", 
					Toast.LENGTH_SHORT);
			toast.show();
		}
	};

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

}
