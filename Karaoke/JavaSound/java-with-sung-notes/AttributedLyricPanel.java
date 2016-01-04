


import javax.swing.*;
import java.awt.*;
import java.awt.font.*;
import java.text.*;
import java.util.Map;


public class AttributedLyricPanel extends JPanel {
    private String text;
    private AttributedString attrText;
    private int coloured = 0;
    private Font font = new Font(Constants.CHINESE_FONT, Font.PLAIN, 36);
    private Font smallFont = new Font(Constants.CHINESE_FONT, Font.PLAIN, 24);
    private Color red = Color.RED;
    private int language = 0x2; // incorrect hack for now
    private String pinyinTxt = null;

    private Map<Character, String> pinyinMap;

    public AttributedLyricPanel( Map<Character, String> pinyinMap) {
	this.pinyinMap = pinyinMap;
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
	coloured = 0;
	text = txt;
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
	coloured++;
	repaint();
    }
	
    /**
     * Draw the string with the first part in red, rest in green.
     * String is centred
     */
	 
    @Override
    public void paintComponent(Graphics g) {
	FontMetrics metrics = g.getFontMetrics();
	int strWidth = metrics.stringWidth(text);
	int panelWidth = getWidth();
	int offset = (panelWidth - strWidth) / 2;

	// System.out.println("Adding attr " + text.length() + " to " + coloured);
	if (coloured != 0) {
	    attrText.addAttribute(TextAttribute.FOREGROUND, red, 0, coloured);
	}
	g.clearRect(0, 0, getWidth(), getHeight());
	g.drawString(attrText.getIterator(), offset, 120); 

	// Draw the Pinyin if it's not zero
	if (pinyinTxt != null && pinyinTxt.length() != 0) {
	    g.setFont(smallFont);
	    metrics = g.getFontMetrics();
	    strWidth = metrics.stringWidth(pinyinTxt);
	    offset = (panelWidth - strWidth) / 2;

	    g.drawString(pinyinTxt, offset, 40);
	    g.setFont(font);
	}
    }
}