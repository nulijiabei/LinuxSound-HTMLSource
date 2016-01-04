
public class SongInformation {


    // Public fields of each song record

    public String path;

    public String index;

    /**
     * song title in Unicode
     */
    public String title;

    /**
     * artist in Unicode
     */
    public String artist;



    public SongInformation(String path,
			   String index,
			   String title,
			   String artist) {
	this.path = path;
	this.index = index;
	this.title = title;
	this.artist = artist;
    }

    public String toString() {
	return "(" + index + ") " + artist + ": " + title;
    }
}
