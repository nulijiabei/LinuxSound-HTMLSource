package newmarch.songtable;

import java.io.BufferedInputStream;
import java.io.ObjectInputStream;
import java.net.URL;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Activity;
import android.os.AsyncTask;
import android.util.Log;

public class SongTable {

	private static Vector<SongInformation> allSongs;
	public static SongTable allSongsTable;

	public Vector<SongInformation> songs = new Vector<SongInformation>();

	public Vector<SongInformation> getSongs() {
		return songs;
	}

	public SongTable(Vector<SongInformation> songs) {
		this.songs = songs;
	}

	public int size() {
		return songs.size();	
	}
	
	public SongTable numberMatches(String pattern) {
		Vector<SongInformation> matchSongs = new Vector<SongInformation>();

		Pattern p = Pattern.compile(pattern, Pattern.CASE_INSENSITIVE);
		Matcher m = p.matcher("");

		for (SongInformation song : songs) {
			m.reset(song.index);
			if (m.find()) {
				matchSongs.add(song);
			}
		}

		return new SongTable(matchSongs);
	}

	public SongTable titleMatches(String pattern) {
		Vector<SongInformation> matchSongs = new Vector<SongInformation>();

		Pattern p = Pattern.compile(pattern, Pattern.CASE_INSENSITIVE);
		Matcher m = p.matcher("");

		for (SongInformation song : songs) {
			m.reset(song.title);
			if (m.find()) {
				matchSongs.add(song);
			}
		}

		return new SongTable(matchSongs);
	}

	
	public SongTable artistMatches(String artist) {
		/*
        // new MainActivity.ArtistMatchTask().execute(artist);

        Vector<SongInformation> v = new Vector<SongInformation>();
        SongInformation dummyLoadingSong = new SongInformation("!", "", "Loading...", "");
        v.add(dummyLoadingSong);
        return new SongTable(v);
    */
		
		Vector<SongInformation> matchSongs = new Vector<SongInformation>();

		Pattern p = Pattern.compile(artist, Pattern.CASE_INSENSITIVE);
		Matcher m = p.matcher("");

		for (SongInformation song : songs) {
			m.reset(song.artist);
			if (m.find()) {
				matchSongs.add(song);
			}
		}

		return new SongTable(matchSongs);
		
	}
}

class RetrieveSongTableThread implements Runnable {
	Vector<SongInformation> songs;
	private MainActivity mainActivity;

	RetrieveSongTableThread(MainActivity mainActivity) {
		this.mainActivity = mainActivity;
	}
	
	public void run() {
		String TAG = "SongTable";
		try {
			System.err.println("Loading d/b");
			Log.wtf(TAG, "Loading d/b");

			URL url = new URL("http://192.168.1.101/KARAOKE/SongStore.tiny");
			BufferedInputStream reader = new BufferedInputStream(url
					.openConnection().getInputStream());
			ObjectInputStream is = new ObjectInputStream(reader);
			songs = (Vector<SongInformation>) is.readObject();
			reader.close();
			System.err.println("Loaded ok");
			Log.wtf(TAG, "Loaded d/b ok");

			mainActivity.listView.post(new Runnable() {
				public void run() {
					
					//mainActivity.allSongs = songs;
					SongTable.allSongsTable = new SongTable(songs);
					mainActivity.setSongs(SongTable.allSongsTable);
					// mImageView.setImageBitmap(bitmap);
				}
			});

			// artistView.setText("loaded songs");
			// return "Loaded ok, size " + songs.size();
		} catch (Exception e) {
			System.err.println(e.toString());
			// return e.toString();
			// artistView.setText(e.toString());
			mainActivity.setSongs(null);
		}
	}

}

/*
class ArtistMatchTask extends AsyncTask<String, Void, SongTable> {
    Vector<SongInformation> songs = new Vector<SongInformation>();
    SongTable artistSongs;

    protected SongTable doInBackground(String... patterns) {
            Pattern p = Pattern.compile(patterns[0], Pattern.CASE_INSENSITIVE);
            Matcher m = p.matcher("");
            
            for (SongInformation song : SongTable.allSongsTable.getSongs()) {
                    m.reset(song.artist);
                    if (m.find()) {
                    	songs.add(song);
                    }
            }
            artistSongs = new SongTable(songs);
            return artistSongs;
    }

    protected void onPostExecute(SongTable artistSongs) {
            // TODO: check this.exception
            // TODO: do something with the feed

            // Toast.makeText(getApplicationContext(), feed, Toast.LENGTH_LONG)
            // .show();
            MainActivity.mainActivity.setSongs(artistSongs);
    }
}
*/

