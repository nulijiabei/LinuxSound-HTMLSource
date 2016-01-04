


import javax.swing.*;
import java.awt.*;
import java.awt.font.*;
import java.text.*;
import java.util.Map;
import sun.font.AttributeValues;

public class AttributedLyricPanel extends JPanel {

    private final int PINYIN_Y = 40;
    private final int TEXT_Y = 90;

    private String text;
    private AttributedString attrText;
    private int coloured = 0;
    private Font font = new Font(Constants.CHINESE_FONT, Font.PLAIN, 36);
    private Font smallFont = new Font(Constants.CHINESE_FONT, Font.PLAIN, 24);
    private Color red = Color.RED;
    private int language;
    private String pinyinTxt = null;

    private boolean broken = false;

    private Map<Character, String> pinyinMap = null;

    public AttributedLyricPanel(Map<Character, String> pinyinMap) {
	this.pinyinMap = pinyinMap;
    }

    public Dimension getPreferredSize() {
	return new Dimension(1000, TEXT_Y + 20);
    }

    public void setLanguage(int lang) {
	language = lang;
	Debug.printf("Lang in panel is %X\n", lang);
    }
	
    public boolean isChinese() {
	switch (language) {
	case SongInformation.CHINESE1:
	case SongInformation.CHINESE2:
	case SongInformation.CHINESE8:
	case SongInformation.CHINESE131:
	case SongInformation.TAIWANESE3:
	case SongInformation.TAIWANESE7:
	case SongInformation.CANTONESE:
	    return true;
	}
	return false;
    }

    public void setText(String txt) {

	broken = false;

	coloured = 0;
	text = txt;
	Debug.println("set text " + text);
	attrText = new AttributedString(text);
	if (text.length() == 0) {
	    return;
	}
	attrText.addAttribute(TextAttribute.FONT, font, 0, text.length());

	if (isChinese()) {
	    pinyinTxt = "";
	    for (int n = 0; n < txt.length(); n++) {
		char ch = txt.charAt(n);
		String pinyin = pinyinMap.get(ch);
		if (pinyin != null) {
		    pinyinTxt += pinyin + " ";
		} else {
		    Debug.printf("No pinyin map for character \"%c\"\n", ch);
		}
	    }
		
	}

	repaint();
    }
	
    public void colourLyric(String txt) {
	coloured += txt.length();
	if (coloured != 0) {
	    repaint();
	}
    }
	
    /**
     * Draw the string with the first part in red, rest in green.
     * String is centred
     */
	 
    @Override
    public void paintComponent(Graphics g) {
	if ((text.length() == 0) || (coloured > text.length())) {
	    return;
	}
	g.setFont(font);
	FontMetrics metrics = g.getFontMetrics();
	int strWidth = metrics.stringWidth(text);
	int panelWidth = getWidth();
	int offset = (panelWidth - strWidth) / 2;

	if (coloured != 0) {
	    try {
		attrText.addAttribute(TextAttribute.FOREGROUND, red, 
				      coloured-1, 
				      coloured);
	    } catch(Exception e) {
		AttributedCharacterIterator iter = attrText.getIterator();
		char ch = iter.first();
		while (ch != CharacterIterator.DONE) {
		    System.out.print(ch);
		    ch = iter.next();
		}
		System.out.println(" " + e.toString() + "\n   " +
				   e.getMessage() + "\n  ");
		e.printStackTrace();
	    }
	}
	g.clearRect(0, 0, getWidth(), getHeight());
	try {
	    if (! broken) {
		g.drawString(attrText.getIterator(), offset, TEXT_Y);
	    } else {
		g.drawString(text, offset, TEXT_Y); 
	    }
	} catch (Exception e) {
	    System.err.println("Attr Str exception on " + text);

	    /*
	    AttributedCharacterIterator iter = attrText.getIterator();
	    char ch = iter.first();
	    System.out.println("  Num chars " + 
			       (iter.getEndIndex() - iter.getBeginIndex()));
	    while (ch != CharacterIterator.DONE) {
		System.out.print("  Char: " + ch);

		Map attributes = iter.getAttributes();
		AttributeValues values = AttributeValues.fromMap(attributes);
		values = values.applyIMHighlight();
		int index1 = values.getUnderline();
		int index2 = values.getInputMethodUnderline();
		System.out.println(" At indices " + index1 + " " + index2);

		ch = iter.next();
	    }
	    */
	    System.out.println(" " + e.toString() + "\n   " +
				   e.getMessage() + "\n  ");
	    // e.printStackTrace();

	    // fallback
	    g.drawString(text, offset, TEXT_Y);
	    broken = true; 

	    // recover?
	    // attrText = new AttributedString(text);
	}
	
	// Draw the Pinyin if it's not zero
	if (pinyinTxt != null && pinyinTxt.length() != 0) {
	    g.setFont(smallFont);
	    metrics = g.getFontMetrics();
	    strWidth = metrics.stringWidth(pinyinTxt);
	    offset = (panelWidth - strWidth) / 2;

	    g.drawString(pinyinTxt, offset, PINYIN_Y);
	    g.setFont(font);
	}
    }
}