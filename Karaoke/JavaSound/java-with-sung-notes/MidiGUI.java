

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.sound.midi.*;
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;
//import java.awt.font.*;
//import java.text.*;
import java.io.*;
// import java.nio.charset.Charset;


public class MidiGUI extends JFrame {
    private static final int MIDI_NOTE_ON = 0x90;
    private static final int MIDI_NOTE_OFF = 0x80;

    private GridLayout mgr = new GridLayout(2,1);

    private MelodyPanel melodyPanel;

    private AttributedLyricPanel lyric1;
    private AttributedLyricPanel lyric2;
    private AttributedLyricPanel[] lyricLinePanels;
    private int whichLyricPanel = 0;

    private JPanel lyricsPanel = new JPanel();

    private Sequencer sequencer;
    private Sequence sequence;
    //private int maxNote = 0;
    //private int minNote = Integer.MAX_VALUE;
    // private Vector<String> lyrics = new Vector<String> ();;
    private Vector<LyricLine> lyricLines; // = new Vector<LyricLine> ();
    // private LyricLine nextLyricLine = new LyricLine();
    private int note;
    private int lyricLine = 0;
    private Font font = new Font("WenQuanYi Zen Hei", Font.PLAIN, 36);
    private long lastTimeStamp = 0;
    private static boolean inLyricHeader = true;
    private Vector<DurationNote> melodyNotes; // = new Vector<DurationNote> ();
    // private Vector<DurationNote> unresolvedNotes = new Vector<DurationNote> ();

    private Map<Character, String> pinyinMap; //  = new  HashMap<Character, String> ();

    private int language;

    public MidiGUI(Sequencer sequencer) {
	this.sequencer = sequencer;
	sequence = sequencer.getSequence();

	// getMaxMin();
	SequenceInformation.setSequence(sequence);
	lyricLines = SequenceInformation.getLyrics();
	melodyNotes = SequenceInformation.getMelodyNotes();

	// SequenceInformation.attachNotesToLyrics(lyricLines, melodyNotes);

	melodyPanel = new MelodyPanel(sequencer);

	pinyinMap = CharsetEncoding.loadPinyinMap();
	lyric1 = new AttributedLyricPanel(pinyinMap);
	lyric2 = new AttributedLyricPanel(pinyinMap);
	lyricLinePanels = new AttributedLyricPanel[] {
	    lyric1, lyric2};

	Debug.println("Lyrics ");

	for (LyricLine line: lyricLines) {
	    Debug.println(line.line + " " + line.startTick + " " + line.endTick +
			       " num notes " + line.notes.size());
	}
	/*
	for (String line : lyrics) {
	    System.out.println(line);
	}
	*/

	getContentPane().setLayout(mgr);
	getContentPane().add(melodyPanel);

	getContentPane().add(lyricsPanel);

	lyricsPanel.setLayout(new GridLayout(2, 1));
	lyricsPanel.add(lyric1);
	lyricsPanel.add(lyric2);

	//lyricLinePanels[(whichLyricPanel+1) % 2].setText(" ");
	lyricLinePanels[whichLyricPanel].setText(lyricLines.elementAt(lyricLine).line);
	if (lyricLine < lyricLines.size() - 1) {
	    lyricLinePanels[(whichLyricPanel+1) % 2].
		setText(lyricLines.elementAt(lyricLine+1).line);
	}
	// melodyPanel.setNotes(lyricLines.elementAt(lyricLine).notes);

	/*	
	JDialog controlsDialog = new JDialog(this);
	Controls controls = new Controls();
	controlsDialog.getContentPane().add(controls);
	controlsDialog.setSize(200, 200);
	controlsDialog.setVisible(true);
	*/

	setSize(1600, 900);
	setVisible(true);
    }

    public void setLanguage(int lang) {
	lyric1.setLanguage(lang);
	lyric2.setLanguage(lang);
    }


    /**
     * A lyric starts with a header section
     * We have to skip over that, but can pick useful
     * data out of it
     */

    /**
     * header format was
     *   #0001
     *   \@00
     *   \@NN  NN is language code
     *   \@title
     *   \@artist
     *   \@writer
     *   \@singer
     */

    /**
     * header format is
     *   \@Llanguage code
     *   \@Ttitle
     *   \@Tsinger
     */
    /*
    private int atCount = 0;
    private static final int HEADERS = 7;
    private String headerStr = "";

    public void addToHeader(String txt) {
	if (txt.equals("\r")) {
	    return;
	}
	headerStr += txt;
    }
    
    public void endHeader() {
	Debug.println("Header is " + headerStr);
	String[] headers = headerStr.split("@");
	int lang = Integer.parseInt(headers[3], 16);
	String title = CharsetEncoding.toUnicode(lang, headers[4].getBytes());
	String artist = headers[7];
	Debug.printf("Lang %X, title %s, artist %s\n", lang, title, artist);
    }
    */
    public void setLyric(String txt) {

	/*
	if (inLyricHeader) {
	    Debug.println("In lyric header");
	    if (atCount == HEADERS && txt.equals("\r\n")) {
		inLyricHeader = false;
		endHeader();
		return;
	    }
	    if (txt.equals("@")) {
		atCount++;
	    }
	    addToHeader(txt);
	    return;
	}
	*/

	if (txt.startsWith("@")) {
	    System.out.println("Header: " + txt);
	    return;
	}

	if (txt.equals("\r\n")) {
	    Debug.println("Setting current lyric line to \"" + 
			  lyricLines.elementAt(lyricLine).line + "\"");
	    Debug.println("Setting next lyric line to \"" + 
			  lyricLines.elementAt(lyricLine + 1).line + "\"");

	    // We've got the \r\n to signal end of line.
	    // But often this occurs before the end of the actual melody
	    // for that line. In attachNotesToLine the end tick for a lyric
	    // line was extended to the end of the notes.
	    // So we keep this lyric line until time is up and reset melody 
	    // and panel after then
	    long endCurrLineTick = lyricLines.elementAt(lyricLine ).endTick;
	    long delayForTicks = endCurrLineTick - sequencer.getTickPosition();
	    float microSecsPerQNote = sequencer.getTempoInMPQ();
	    float delayInMicroSecs = microSecsPerQNote * delayForTicks / 24;
	    Debug.println("Delaying melody update for microsecs " + delayInMicroSecs);
	    
	    final Vector<DurationNote> notes = lyricLines.elementAt(lyricLine + 1).notes;
	    final int thisPanel = whichLyricPanel;
	    final int nextLineForPanel = lyricLine + 2;
	    Timer timer = new Timer((int) Math.floor(delayInMicroSecs/1000),
				    new ActionListener() {
					public void actionPerformed(ActionEvent e) {
					    // melodyPanel.setNotes(notes);
					    if (nextLineForPanel >= lyricLines.size()) {
						return;
					    }
					    lyricLinePanels[thisPanel].setText(lyricLines.elementAt(nextLineForPanel).line);
		
					}
				    });
	    timer.setRepeats(false);
	    timer.start();
	    

	    // Due to weaknesses in the algorithm for attaching notes to lyrics,
	    // some notes may end up attached to the previous lyric line.
	    // These are "unplayed" and have to be stuck at the front of
	    // the next lyric line

	    /*
	    final Vector<DurationNote> notes = lyricLines.elementAt(lyricLine + 1).notes;
	    Vector<DurationNote> unplayedNotes = melody.getUnplayedNotes();
	    notes.addAll(0, unplayedNotes);
	    Debug.println("Have some unplayed notes");
	    //melody.setNotes(notes);

	    *
	     * A lyric can finish before its note. No problem unless
	     * we are at the end of a line. Then we don't want to refresh
	     * the notes (melody) panel when the lyric finishes, but delay
	     * that until the next notes are ready to be played.
	     * Chuck it into a Swing Timer to do that
	     *
	    
	    if (notes.size() > 0) {
		long nextNoteTick = notes.elementAt(0).startTick;
		long delayForTicks = nextNoteTick - sequencer.getTickPosition();
		float microSecsPerQNote = sequencer.getTempoInMPQ();
		float delayInMicroSecs = microSecsPerQNote * delayForTicks / 24;
		Debug.println("Delaying melody update for microsecs " + delayInMicroSecs);

		Timer timer = new Timer((int) Math.floor(delayInMicroSecs/1000),
				  new ActionListener() {
				      public void actionPerformed(ActionEvent e) {
					  melody.setNotes(notes);
				      }
				  });
		timer.setRepeats(false);
		timer.start();
	    }
	    
	    
	    if (lyricLine < lyricLines.size() - 2) {
		// Perhaps delay this a little bit
		lyricLinePanels[whichLyricPanel].setText(lyricLines.elementAt(lyricLine + 2).line);
	    }
	    */
	    
	    whichLyricPanel = (whichLyricPanel + 1) % 2;
	    
	    Debug.println("Setting new lyric line at tick " + 
			       sequencer.getTickPosition());
	    
	    lyricLine++;
	} else {
	    Debug.println("Playing lyric " + txt);
	    lyricLinePanels[whichLyricPanel].colourLyric(txt);
	}
    }

    public void setNote(long timeStamp, int onOff, int note) {
	Debug.printf("Setting note in gui to %d\n", note);

	if (onOff == 0x80) {
	    melodyPanel.drawNoteOff(note);
	    this.note = 0;
	} else if (onOff == 0x90) {
	    this.note = note;
	    melodyPanel.drawNoteOn(note);
	}
    }

    /*
    public void setSungNote(float timeStamp, float note) {
	// pitch detected at about 20ms intervals
	Debug.printf("Midi note sung %f  at %f\n", 
			  note, timeStamp);
	melodyPanel.drawSungNote(note);

    }
    */
}


