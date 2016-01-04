package newmarch.songtable;

import java.io.BufferedInputStream;
import android.widget.TextView.OnEditorActionListener;

import java.io.BufferedWriter;
import java.io.ObjectInputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URL;
import java.util.ArrayList;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import newmarch.songselector.R;

import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.annotation.TargetApi;
import android.app.Activity;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


@TargetApi(Build.VERSION_CODES.HONEYCOMB)
public class MainActivity extends Activity {

	public static MainActivity mainActivity;
	
	public enum Selection {
		ARTIST,
		TITLE,
		SONGID
	}
	
	SongTable songTable;
	ListView listView;
	EditText songIDText, titleText, artistText;
	//Selection searchSelectionType;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		// kludge to make this activity available everywhere
		// e.g. in AsyncTask's
		mainActivity = this;

		TextView textView1 = (TextView) findViewById(R.id.songidlabel);
		TextView textView2 = (TextView) findViewById(R.id.titlelabel);
		TextView textView3 = (TextView) findViewById(R.id.artistlabel);
		
		songIDText = (EditText) findViewById(R.id.songid);
		songIDText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
			      //verifies what key is pressed, in our case verifies if the DONE key is pressed
			      if (actionId == EditorInfo.IME_ACTION_DONE) {
						String songID = songIDText.getText().toString();

						if (songID.equals(""))
							return false;

						/*
						//Vector<SongInformation> songs = numberMatches(songID);
						SongTable songs = songTable.numberMatches(songID);
						setSongs(songs);
						*/
						
						//searchSelectionType = Selection.SONGID;
						new SelectionMatchTask(songID, Selection.SONGID).execute();
			      }
				return false;
			}
		});
		
		
		songIDText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			public void onFocusChange(View v, boolean hasFocus) {
			    if(hasFocus){
			        songIDText.setText("");
			    }
			    if (artistText.getText().toString().equals("") &&
			    		titleText.getText().toString().equals("")) {
			    	setSongs(SongTable.allSongsTable);
			    }
			   }
			});
		
		songIDText.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				songIDText.setText("");
			    if (artistText.getText().toString().equals("") &&
			    		titleText.getText().toString().equals("")) {
			    	setSongs(SongTable.allSongsTable);
			    }
			}	
		});

		
		titleText = (EditText) findViewById(R.id.title);
		titleText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
			      //verifies what key is pressed, in our case verifies if the DONE key is pressed
			      if (actionId == EditorInfo.IME_ACTION_DONE) {
						String title = titleText.getText().toString();

						if (title.equals(""))
							return false;

						/*
						SongTable songs = songTable.titleMatches(title);
						setSongs(songs);
						*/
						
						//searchSelectionType = Selection.TITLE;
						new SelectionMatchTask(title, Selection.TITLE).execute();
			      }
				return false;
			}
		});
		
		
		titleText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			public void onFocusChange(View v, boolean hasFocus) {
			    if(hasFocus){
			        titleText.setText("");
			    }
			    if (songIDText.getText().toString().equals("") &&
			    		artistText.getText().toString().equals("")) {
			    	setSongs(SongTable.allSongsTable);
			    }
			   }
			});
		
		titleText.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				titleText.setText("");
			    if (songIDText.getText().toString().equals("") &&
			    		artistText.getText().toString().equals("")) {
			    	setSongs(SongTable.allSongsTable);
			    }
			}	
		});

		//
		
		artistText = (EditText) findViewById(R.id.artist);
		artistText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
			      //verifies what key is pressed, in our case verifies if the DONE key is pressed
			    Log.v("EDITOR", "Action: " + actionId);  
			    //Log.v("EDITOR", "Key event " + event.toString());
				if (actionId == EditorInfo.IME_ACTION_DONE || 
						actionId == EditorInfo.IME_ACTION_PREVIOUS) {
						String artist = artistText.getText().toString();

						if (artist.equals(""))
							return false;

						//searchSelectionType = Selection.ARTIST;
						new SelectionMatchTask(artist, Selection.ARTIST).execute();
			      }
				return false;
			}
		});
		
		artistText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			public void onFocusChange(View v, boolean hasFocus) {
			    if(hasFocus){
			        artistText.setText("");
			    
			        if (songIDText.getText().equals("") &&
			        		titleText.getText().equals("")) {
			        	setSongs(SongTable.allSongsTable);
			        }
			    }
			   }
			});
		artistText.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				artistText.setText("");
			    if (songIDText.getText().toString().equals("") &&
			    		titleText.getText().toString().equals("")) {
			    	setSongs(SongTable.allSongsTable);
			    }
			}	
		});
		
		listView = (ListView) findViewById(R.id.listview);

		listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> parent, final View view,
					int position, long id) {
				SongInformation item = (SongInformation) parent.getItemAtPosition(position);
				
				Toast.makeText(MainActivity.this.getApplicationContext(),
						"Selected " + item.toString(), Toast.LENGTH_LONG)
						.show();
				 sendToPlayer(item);
			}
		});

		String[] values = new String[] { "Loading..." };

		final ArrayList<String> list = new ArrayList<String>();
		for (int i = 0; i < values.length; ++i) {
			list.add(values[i]);
		}
		final ArrayAdapter adapter = new ArrayAdapter(this,
				android.R.layout.simple_list_item_single_choice, list);
		listView.setAdapter(adapter);
		listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

		new Thread(new RetrieveSongTableThread(this)).start();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		//getMenuInflater().inflate(R.menu.main, menu);
		//menu.setGroupVisible(1, true);
		
		// See http://stackoverflow.com/questions/17301113/dynamically-add-action-item-in-action-bar
		MenuItem jan = menu.add(1, 1, 1, "Jan");
		jan.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS | 
					MenuItem.SHOW_AS_ACTION_WITH_TEXT);
		MenuItem linda = menu.add(1, 2, 1, "Linda");
		linda.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS | 
					MenuItem.SHOW_AS_ACTION_WITH_TEXT);

		return true;
	}

	//See http://www.vogella.com/tutorials/AndroidActionBar/article.html
	@Override
	  public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	    case 1:
	      Toast.makeText(this, "Jan", Toast.LENGTH_SHORT)
	          .show();
	      break;
	    case 2:
	      Toast.makeText(this, "Linda", Toast.LENGTH_SHORT)
	          .show();
	      break;

	    default:
	      break;
	    }

	    return true;
	  } 
	
	public void setSongs(SongTable songTable) {
		this.songTable = songTable;
		
		Vector<SongInformation> songs = songTable.getSongs();
	
		ArrayAdapter adapter = new ArrayAdapter<SongInformation>(this,
		// MainActivity.this.getApplicationContext(),
				android.R.layout.simple_list_item_1, songs);
		/*
		Toast.makeText(getApplicationContext(),
				"Loading songs " + songs.size(), Toast.LENGTH_LONG).show();
		*/
		listView.setAdapter(adapter);
		listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
	}
	
	public void setLoading() {
		/* put in a dummy song, but don't override current song table */
    	Vector<SongInformation> v = new Vector<SongInformation>();
        SongInformation dummyLoadingSong = new SongInformation("!", "", "Loading...", "");
        v.add(dummyLoadingSong);

		ArrayAdapter adapter = new ArrayAdapter<SongInformation>(this,
				android.R.layout.simple_list_item_1, v);
		
		listView.setAdapter(adapter);
		listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
	}

	/*
	public Vector<SongInformation> numberMatches(String pattern) {
		Vector<SongInformation> matchSongs = new Vector<SongInformation>();

		for (SongInformation song : SongTable.allSongsTable.getSongs()) {
			if (song.numberMatch(pattern)) {
				matchSongs.add(song);
			}
		}
		return matchSongs;
	}

	public Vector<SongInformation> titleMatches(String pattern) {
		Vector<SongInformation> matchSongs = new Vector<SongInformation>();

		for (SongInformation song : SongTable.allSongsTable.getSongs()) {
			if (song.titleMatch(pattern)) {
				matchSongs.add(song);
			}
		}
		return matchSongs;
	}
	*/
	public void sendToPlayer(SongInformation song) {
		Toast.makeText(MainActivity.this,
				"ending to player " + song.path, Toast.LENGTH_LONG).show();
		new SendToPlayerTask(song.path).execute();
	}
	
	class SendToPlayerTask extends AsyncTask<String, Void, String> {

		private Exception exception;
		private String songPath;
		public static final String SERVERIP = "192.168.1.110"; 
		public static final int SERVERPORT = 13000;
		private PrintWriter out;
		String TAG = "SongTable";
		
		public SendToPlayerTask(String path) {
			this.songPath = path;
			Toast.makeText(MainActivity.this,
    				"Initing task with  " + path, Toast.LENGTH_LONG).show();
		}
		
		protected String doInBackground(String... urls) {		
			try {
				InetAddress serverAddr = InetAddress.getByName(SERVERIP);
				Socket socket = new Socket(serverAddr, SERVERPORT);
				 
	            //send the message to the server
	            out = new PrintWriter(
	            		new BufferedWriter(
	            				new OutputStreamWriter(socket.getOutputStream())), 
	            										true);
	  
	            Log.v(TAG, "Writing to server  " + songPath);
	            
				out.println(songPath);
				socket.close();
				
			} catch (Exception e) {
				System.err.println(e.toString());

				Log.v(TAG, e.toString());
				return e.toString();
			}
			return "";
		}

		protected void onPostExecute(String feed) {
			// TODO: check this.exception
			// TODO: do something with the feed
		}
	}
	

	class SelectionMatchTask extends AsyncTask<String, Void, SongTable> {
	    SongTable songs;
	    String selection;
	    Selection selectionType;
        
	    SelectionMatchTask(String selection, Selection selectionType) {
	    	this.selection = selection;
	    	this.selectionType = selectionType;
	    }
	    
	    protected void onPreExecute() {
	    	
	    	/*
	    	Vector<SongInformation> v = new Vector<SongInformation>();
	        SongInformation dummyLoadingSong = new SongInformation("!", "", "Loading...", "");
	        v.add(dummyLoadingSong);
	        // MainActivity.this.setSongs(new SongTable(v));
	        */
	    	
	    	setLoading();
	    	
	        Toast t = Toast.makeText(MainActivity.this.getApplicationContext(),
					"Loading wait...", Toast.LENGTH_LONG);
	        t.setGravity(Gravity.CENTER, 0, 0);
			t.show();
					
	    }
	    
	    protected SongTable doInBackground(String... patterns) {
	    	/*
	    	Vector<SongInformation> v = new Vector<SongInformation>();
	        SongInformation dummyLoadingSong = new SongInformation("!", "", "Loading...", "");
	        v.add(dummyLoadingSong);
	        MainActivity.this.setSongs(new SongTable(v));
	        */
	    	
	    	switch (selectionType) {
	    	case ARTIST:
	    		songs = songTable.artistMatches(selection);
	    		break;
	    	case TITLE:
	    		songs = songTable.titleMatches(selection);
	    		break;
	    	case SONGID:
	    		songs = songTable.numberMatches(selection);
	    		break;
	    	}
	    		
	    	//artist = patterns[0];
	    	
	    	return songs;
	    }

	    protected void onPostExecute(SongTable artistSongs) {
	    	 Toast t = Toast.makeText(MainActivity.this.getApplicationContext(),
							"Setting " +
							songs.size() + 
							" songs for " + selection, Toast.LENGTH_LONG);
		        t.setGravity(Gravity.CENTER, 0, 0);
				t.show();
            MainActivity.this.setSongs(songs);
	    }
	}
}
