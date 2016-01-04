package newmarch.songtable;

import java.io.BufferedInputStream;
import java.io.ObjectInputStream;
import java.net.URL;
import java.util.Locale;
import java.util.Vector;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.NavUtils;
import android.support.v4.view.ViewPager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends FragmentActivity {

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;



	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		// Create the adapter that will return a fragment for each of the three
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(
				getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);


		new RetreiveFeedTask().execute();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public void setMainSongs(Vector<SongInformation> songs) {
		MainSongTableFragment songTable = (MainSongTableFragment)
				mSectionsPagerAdapter.getItem(0);
		songTable.setSongs(songs);
	}
	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		private MainSongTableFragment mainSongTable = null;
		
		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			// getItem is called to instantiate the fragment for the given page.
			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			
			Fragment fragment;
			if (position == 0) {
				if (mainSongTable == null) {
					mainSongTable = new MainSongTableFragment();
				}
				fragment = mainSongTable;
			} else {
				fragment = new DummySectionFragment();
			}
			Bundle args = new Bundle();
			args.putInt(DummySectionFragment.ARG_SECTION_NUMBER, position + 1);
			fragment.setArguments(args);
			return fragment;
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			Locale l = Locale.getDefault();
			switch (position) {
			case 0:
				return getString(R.string.title_section1).toUpperCase(l);
			case 1:
				return getString(R.string.title_section2).toUpperCase(l);
			case 2:
				return getString(R.string.title_section3).toUpperCase(l);
			}
			return null;
		}
	}

	/**
	 * A dummy fragment representing a section of the app, but that simply
	 * displays dummy text.
	 */
	public static class DummySectionFragment extends Fragment {
		/**
		 * The fragment argument representing the section number for this
		 * fragment.
		 */
		public static final String ARG_SECTION_NUMBER = "section_number";
		Vector<SongInformation> songs;
		
		public DummySectionFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
	        LinearLayout layout;
	        //TextView artistView;
	        //Button trackView;

			View rootView = inflater.inflate(R.layout.fragment_main_dummy,
					container, false);
			TextView dummyTextView = (TextView) rootView
					.findViewById(R.id.section_label);
			dummyTextView.setText("Hello " + Integer.toString(getArguments().getInt(
					ARG_SECTION_NUMBER)));
			TextView artistView = (TextView) rootView
					.findViewById(R.id.text);
			artistView.setText("Goodbye");
			Button button = (Button) rootView
					.findViewById(R.id.button);
			
			
			/*
			try {
					System.err.println("Loading d/b");
					URL url 	= new URL("http://192.168.1.101/server/KARAOKE/SongStore");
					BufferedInputStream reader = 
                    new BufferedInputStream(url.openStream());
					ObjectInputStream is = new ObjectInputStream(reader);
					songs = (Vector<SongInformation>) is.readObject();
					reader.close();
					System.err.println("Loaded ok");
					artistView.setText("loaded songs");
			} catch(Exception e) {
				System.err.println(e.toString());
				artistView.setText(e.toString());
			}
			 */
			/*
	        layout = new LinearLayout(this.getContext());
	        layout.setOrientation(LinearLayout.VERTICAL);
	        artistInfo = new TextView(this.getContext());
	        artistInfo.setText("hello");
	        trackInfo = new TextView(this.getContext());
	        trackInfo.setText("world");
	        layout.addView(artistInfo);
	        layout.addView(trackInfo);
			 */

			return rootView;
		}
	}
	
	/**
	 * A fragment for the major song table.
	 */
	public class MainSongTableFragment extends Fragment {
		/**
		 * The fragment argument representing the section number for this
		 * fragment.
		 */
		public static final String ARG_SECTION_NUMBER = "section_number";
		public Vector<SongInformation> songs;
		ListView list = null;
		
		public MainSongTableFragment() {
		}

		public void setSongs(Vector<SongInformation> songs) {
			this.songs = songs;
			ArrayAdapter adapter = 
					new ArrayAdapter<SongInformation>(
							MainActivity.this.getApplicationContext(), 
							android.R.layout.simple_list_item_1, songs);
			Toast.makeText(getApplicationContext(), 
					"Loading songs " + songs.size(),
					Toast.LENGTH_LONG)
					.show();	
		}
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
	        LinearLayout layout;
	        //TextView artistView;
	        //Button trackView;

			View rootView = inflater.inflate(R.layout.fragment_main_song_table,
					container, false);
			TextView dummyTextView = (TextView) rootView
					.findViewById(R.id.section_label);
			dummyTextView.setText("Hello " + Integer.toString(getArguments().getInt(
					ARG_SECTION_NUMBER)));
			TextView artistView = (TextView) rootView
					.findViewById(R.id.text);
			artistView.setText("Goodbye");
			list = (ListView) rootView
					.findViewById(R.id.list);
			
			
			/*
			try {
					System.err.println("Loading d/b");
					URL url 	= new URL("http://192.168.1.101/server/KARAOKE/SongStore");
					BufferedInputStream reader = 
                    new BufferedInputStream(url.openStream());
					ObjectInputStream is = new ObjectInputStream(reader);
					songs = (Vector<SongInformation>) is.readObject();
					reader.close();
					System.err.println("Loaded ok");
					artistView.setText("loaded songs");
			} catch(Exception e) {
				System.err.println(e.toString());
				artistView.setText(e.toString());
			}
			 */
			/*
	        layout = new LinearLayout(this.getContext());
	        layout.setOrientation(LinearLayout.VERTICAL);
	        artistInfo = new TextView(this.getContext());
	        artistInfo.setText("hello");
	        trackInfo = new TextView(this.getContext());
	        trackInfo.setText("world");
	        layout.addView(artistInfo);
	        layout.addView(trackInfo);
			 */

			return rootView;
		}
	}


	class RetreiveFeedTask extends AsyncTask<String, Void, String> {

		private Exception exception;
		Vector<SongInformation> songs;

		protected String doInBackground(String... urls) {
			try{
				System.err.println("Loading d/b");
				URL url 	= new URL("http://192.168.1.101/KARAOKE/SongStore");
				BufferedInputStream reader = 
						new BufferedInputStream(url
									.openConnection()
									.getInputStream());
				ObjectInputStream is = new ObjectInputStream(reader);
				songs = (Vector<SongInformation>) is.readObject();
				reader.close();
				System.err.println("Loaded ok");
				//artistView.setText("loaded songs");
				return "Loaded ok, size " + songs.size();
			} catch(Exception e) {
				System.err.println(e.toString());
				return e.toString();
				//artistView.setText(e.toString());
			}

			/*
	    	    	try {
	    	            URL url= new URL(urls[0]);
	    	            SAXParserFactory factory =SAXParserFactory.newInstance();
	    	            SAXParser parser=factory.newSAXParser();
	    	            XMLReader xmlreader=parser.getXMLReader();
	    	            RssHandler theRSSHandler=new RssHandler();
	    	            xmlreader.setContentHandler(theRSSHandler);
	    	            InputSource is=new InputSource(url.openStream());
	    	            xmlreader.parse(is);
	    	            return theRSSHandler.getFeed();
	    	        } catch (Exception e) {
	    	            this.exception = e;
	    	            return null;
	    	        }
			 */
		}
		protected void onPostExecute(String feed) {
			// TODO: check this.exception 
			// TODO: do something with the feed
			
			Toast.makeText(getApplicationContext(),
				      feed, Toast.LENGTH_LONG)
				      .show();
			MainActivity.this.setMainSongs(songs);
			/*
			new AlertDialog.Builder(MainActivity.this)
		    .setTitle("Delete entry")
		    .setMessage(feed)
		    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
		        public void onClick(DialogInterface dialog, int which) { 
		            // continue with delete
		        }
		     })
		    .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
		        public void onClick(DialogInterface dialog, int which) { 
		            // do nothing
		        }
		     })
		     .show();
		     */
		}
	}
}