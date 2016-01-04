
import java.util.Vector;
import javax.swing.*;
import java.awt.*;
import javax.sound.midi.*;


public class MelodyPanel extends JPanel {

    long timeStamp;
    private Vector<DurationNote> notes;
    private Vector<DurationNote> sungNotes;
    private int lastNoteDrawn = -1;
    private Sequencer sequencer;
    private Sequence sequence;
    private int maxNote = 0;
    private int minNote = Integer.MAX_VALUE;
    //private Vector<DurationNote> melodyNotes = new Vector<DurationNote> ();
    private Vector<DurationNote> unresolvedNotes = new Vector<DurationNote> ();

    private String[] noteString = new String[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    private int playingNote = -1;

    public MelodyPanel(Sequencer sequencer) {
	//this.sequencer = sequencer;
	//sequence = sequencer.getSequence();

	maxNote = SequenceInformation.getMaxMelodyNote();
	minNote = SequenceInformation.getMinMelodyNote();
	Debug.println("Max: " + maxNote + " Min " + minNote);


	/*
       	for (DurationNote dnote : melodyNotes) {
	    Debug.println(dnote.toString());
	}
	*/
    }

    /**
     * @return Notes that haven't been played yet
     */
    /*
    public Vector<DurationNote> getUnplayedNotes() {
	Vector<DurationNote> unplayed = new Vector<DurationNote> ();
	for (int n = lastNoteDrawn + 1; n < notes.size(); n++) {
	    unplayed.add(notes.elementAt(n));
	}
	return unplayed;
    }



    public void drawSungNote(float note) {
	float onote = note;
	long sungNoteMidiTick = sequencer.getTickPosition();

	// Find which note is being played (if any). 
	// We do this so we can figure out how many octaves
	// to raise/lower the sung note so it looks close to
	// the note being played.

	// Alg: the note will have started and possibly finished but
	// the next note hasn't started yet. So we count down from
	// the last note till we get to a note that started before us.
	// This assumes that the song isn't leaping up/down by over an
	// octave and that the singer isn't singing in advance of the melody

	DurationNote currentNote = null;
	for (int n = notes.size() - 1; n >= 0;  n--) {
	    if (sungNoteMidiTick >= notes.elementAt(n).startTick) {
		currentNote = notes.elementAt(n);
		break;
	    }
	} 
	if (currentNote == null) {
	    return;
	}
	    
	// move note by octaves close to note that should be sung
	// using integer arith to floor the real values.
	long diff = Math.round(currentNote.note - note);
	if (diff > 0)
	    diff += 6;  // so it increases in the right direction
	else
	    diff -= 6;  // so it decreases in the right direction
	note += (diff/12) * 12;
	Debug.printf("Playiong note %d, Sung note: %f originally %f at tick %d\n",
			  currentNote.note, note, onote, sungNoteMidiTick);
	DurationNote dnote = new DurationNote(sungNoteMidiTick, Math.round(note));
	dnote.endTick = sungNoteMidiTick + 5; // 5 is a guess for now
	sungNotes.add(dnote);

	repaint();
    }
    */

    public void drawNoteOff(int note) {
	if (note < minNote || note > maxNote) {
	    return;
	}

	System.out.println("Note off played is " + note);
	playingNote = -1;
	repaint();
    }

    public void drawNoteOn(int note) {
	if (note < minNote || note > maxNote) {
	    return;
	}

	System.out.println("Note on played is " + note);
	playingNote = note;
	repaint();
	/*
	lastNoteDrawn++;
	if (lastNoteDrawn < notes.size()) {
	    DurationNote n = notes.elementAt(lastNoteDrawn);
	    if (n.note != note) {
		Debug.println("Expected " + n.note + " got " + note);
	    } else {
		Debug.println("Got expected " + n.note + 
				   " at tick " + sequencer.getTickPosition());
		repaint();
	    }
	}
	*/
    }

    /*
    public void setTimeStamp(long t){
	Debug.println("Setting timestamp for melody panel to " + t);
	timeStamp = t;
    }

    public void setNotes(Vector<DurationNote> notes) {
	this.notes = notes;
	lastNoteDrawn = -1;
	sungNotes = new Vector<DurationNote>();
	repaint();
    }

    private String noteToString(int note) {
	int noteIndex = (note % 12);
	return noteString[noteIndex];
    }
    */

    /*
    private void drawStaveLines(Graphics g, int maxNote, int minNote, 
				int ht, int width,
				int heightScaleFactor, int widthScaleFactor) {
    */
	/* For dashed lines:
	  import javax.swing.*;
	  import java.awt.*;
	  import java.awt.geom.*;
	  public class Stroke1 extends JFrame {
	  Stroke drawingStroke = new BasicStroke(3, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 0, new float[]{9}, 0);
	  Line2D line = new Line2D.Double(20, 40, 100, 40);
	  public void paint(Graphics g) {
	  super.paint(g);
	  Graphics2D g2d = (Graphics2D) g;
	  g2d.setStroke(drawingStroke);
	  g2d.draw(line);
	  }
	*/
	/*
	for (int n = minNote; n <= maxNote; n += 2) {
	    //int note = n * heightScaleFactor;
	    int y = (ht - 20) - (n - minNote) * heightScaleFactor;
	    g.drawString(noteToString(n), 0, y);
	    g.drawLine(20, y, width, y);
	}
	*/
    /*
    }
    */

    private void drawPiano(Graphics g, int width, int height) {
	System.out.println("Repaiting");
	int noteWidth = width / (108 - 21);
	for (int noteNum = 21; // A0
	     noteNum <= 108; // C8
	     noteNum++) {
	    
	    drawNote(g, noteNum, noteWidth);
	}
    }

    private void drawNote(Graphics g, int noteNum, int width) {
	if (isWhite(noteNum)) {
	    noteNum -= 21;
	    g.setColor(Color.WHITE);
	    g.fillRect(noteNum*width, 10, width, 100);
	    g.setColor(Color.BLACK);
	    g.drawRect(noteNum*width, 10, width, 100);
	} else {
	    noteNum -= 21;
	    g.setColor(Color.BLACK);
	    g.fillRect(noteNum*width, 10, width, 100);
	}
	if (playingNote != -1) {
	    g.setColor(Color.BLUE);
	    g.fillRect(playingNote*width, 10, width, 100);
	}	    
    }

    private boolean isWhite(int noteNum) {
	noteNum = noteNum % 12;
	switch (noteNum) {
	case 1:
	case 3:
	case 6:
	case 8:
	case 10:
	case 13: 
	    return false;
	default:
	    return true;
	}
    }


    @Override
    public void paintComponent(Graphics g) {
	/*
	if (notes == null || notes.size() == 0) {
	    return;
	}
	*/
	int ht = getHeight();
	int width = getWidth();

	drawPiano(g, width, ht);

	/*
	Debug.println("Repainting meldy panel");
	DurationNote firstNote = notes.elementAt(0);
	DurationNote lastNote = notes.elementAt(notes.size() - 1);

	int noteWidth = (int) (lastNote.endTick - firstNote.startTick);
	int heightScaleFactor = ht/(maxNote - minNote + 5);
	int widthScaleFactor = (width - 20) / noteWidth;
	if (widthScaleFactor > 4) {
	    // don't stretch things too much
	    widthScaleFactor = 4;
	}

	int offset = (width - noteWidth * widthScaleFactor) / 2;

	g.clearRect(0, 0, width, ht);

	drawStaveLines(g, maxNote, minNote, ht, width, 
		       heightScaleFactor, widthScaleFactor);

	// Draw the notes ofthe melody
	for (int n = 0; n < notes.size(); n++) {
	    DurationNote note = notes.elementAt(n);
	    //for (DurationNote note: notes) {
	    int x1 = ((int) (note.startTick - firstNote.startTick)) * widthScaleFactor;
	    int x2 = ((int) (note.endTick - firstNote.startTick)) * widthScaleFactor;
	    int y1 = (ht - 20) - (note.note - minNote) * heightScaleFactor;
	    //g.drawLine(x1, y1, x2, y1);
	    if (n <=  lastNoteDrawn) {
		renderNote(g, x1 + offset, y1, (x2-x1), heightScaleFactor, Color.RED);
	    } else {
		renderNote(g, x1 + offset, y1, (x2-x1), heightScaleFactor, Color.GREEN);
	    }
	    Debug.println("Drawing note (melody) " + note.note + 
			       " at (" + x1 + "," + y1 + ") from " +
			       note.startTick + " to " + note.endTick);
	}

	// Draw the notes sung
	for (int n = 0; n < sungNotes.size(); n++) {
	    DurationNote note = sungNotes.elementAt(n);
	    Debug.println("Sung notes in list " + note);
	    int y = (ht - 20) - (int) ((note.note - minNote) *  heightScaleFactor);
	    int x1 = ((int) (note.startTick - firstNote.startTick)) * widthScaleFactor;
	    int x2 = ((int) (note.endTick - firstNote.startTick)) * widthScaleFactor;
	    renderNote(g, x1+offset, y, 5, heightScaleFactor, Color.BLUE);
	    Debug.printf("Drawing note (sung) %d at (%d,%d)\n", note.note, x1, y);
	}
	*/	
    }

    /*
    private void renderNote(Graphics g, int x, int y, int duration, 
			    int heightScaleFactor, Color color) {
	g.setColor(color);
	g.fillRect(x, y - heightScaleFactor/2, duration, heightScaleFactor);
	g.setColor(Color.BLACK);
	g.drawRect(x, y - heightScaleFactor/2, duration, heightScaleFactor);
    }
    */
}