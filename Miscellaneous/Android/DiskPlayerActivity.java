package jan.newmarch.DiskPlayer;

import java.io.File;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.view.Menu;

public class DiskPlayerActivity extends Activity {

    private String filePath = "/Removable/SD/Music/audio_01.ogg";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);
	setContentView(R.layout.activity_disk_player);
	try {
	    String[] array=filePath.split("/");
	    File f = null;
	    for(int t = 0; t < array.length - 1; t++)
		{
		    f = new File(f, array[t]);
		}
	    File songFile=new File(f, array[array.length-1]);
		
	    MediaPlayer player = new MediaPlayer();
	    player.setAudioStreamType(AudioManager.STREAM_MUSIC);
	    player.setDataSource(songFile.getAbsolutePath());
	    player.prepare()	;
	    player.start();
	} catch (Exception e) {
	    AlertDialog.Builder builder = new AlertDialog.Builder(this);
	    builder.setMessage(e.toString());
	    builder.create();
	    builder.show();
	}
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
	// Inflate the menu; this adds items to the action bar if it is present.
	getMenuInflater().inflate(R.menu.activity_disk_player, menu);
	return true;
    }

}
