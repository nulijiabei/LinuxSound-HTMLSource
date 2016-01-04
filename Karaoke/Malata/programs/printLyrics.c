#include <stdio.h>

#define NUM_LINES 100
#define LINE_LEN 25

typedef enum {BEFORE_SONG, IN_SONG_AND_LINE, IN_SONG_BETWEEN} state_t;

state_t state = BEFORE_SONG;
int half = 1;

FILE *fp;
FILE *ofp;

unsigned char prev_ch, curr_ch;
unsigned char lines[NUM_LINES][LINE_LEN];
unsigned char separator[NUM_LINES][LINE_LEN];
unsigned char deltas[NUM_LINES][LINE_LEN];

unsigned char *first_lines, 
    *second_lines,
    *first_separators,
    *second_separators,
    *first_deltas,
    *second_deltas;

int max_lines;

void read1() {
    prev_ch = curr_ch;
    curr_ch = getc(fp);
    // fprintf(ofp, "%X %X\n", prev_ch, curr_ch);
}

void read2() {
  prev_ch = getc(fp);  
  curr_ch = getc(fp);
    // fprintf(ofp, "%X %X\n", prev_ch, curr_ch);
}


void printLine(unsigned char *line) {
    fprintf(ofp, "%2d: ", (line - lines[0])/LINE_LEN);
    int m = 0;
    for (m = 0; m < LINE_LEN; m++) {
	if (line[m] != 0) {
	    putc(line[m], ofp);
	} else {
	    putc(' ', ofp);
	}
    }
}

void printDeltas(unsigned char *deltas) {
    int m, max;

    for (max = LINE_LEN-1; max >= 0; max--) {
	if (deltas[max] != 0)
	    break;
    }
	       
    for (m = 0; m <= max; m++) {
	fprintf(ofp, "%2X ", deltas[m]);
    }

    for (m = max+1; m < LINE_LEN; m++) {
	fprintf(ofp, "   ");
    }	
}

void printSeparator(unsigned char *sep) {
    int m, max;

    for (max = LINE_LEN-1; max >= 0; max--) {
	if (sep[max] != 0)
	    break;
    }
	       
    for (m = 0; m <= max; m++) {
	fprintf(ofp, "%2X ", sep[m]);
    }
    
}

int endSection(unsigned char *sep) {
    // does this sep end with 00 26, a break in the music?
    int m, max;

    for (max = LINE_LEN-1; max >= 0; max--) {
	if (sep[max] != 0)
	    break;
    }
	       
    if ((max >= 1) && (sep[max] == 0x26) && (sep[max-1] == 0))
	return 1;
    return 0;
}

int main(int argc, char **argv) {
    int inSong = 0;
    // int lineNo = 0;
    int inLine = 0;

    bzero(lines, NUM_LINES*LINE_LEN* sizeof(unsigned char));
    bzero(separator, NUM_LINES*LINE_LEN* sizeof(unsigned char));
    bzero(deltas, NUM_LINES*LINE_LEN* sizeof(unsigned char));

    int lineNo = 0;
    int charNo = 0;

    if (argc == 1) {
	fp = stdin;
	ofp = stdout;
    } else if (argc == 2) {
	fp = fopen(argv[1], "r");
	ofp = stdout;
    } else {
	fp = fopen(argv[1], "r");
	ofp = fopen(argv[2], "w");
    }

    int n = 0;
    
    while ((n++ < 6000) && (half <= 2)) {
	
	switch (state) {
	case BEFORE_SONG:
	    read2();
	    if ((prev_ch == 0) && (curr_ch == '&')) {
		state = IN_SONG_AND_LINE;
	    }
	    break;
	case IN_SONG_AND_LINE:
	    read2();
	    if (curr_ch == '^') {
		state = IN_SONG_BETWEEN;
		putc('\n', ofp);

		fprintf(ofp, "%d: ", lineNo);

		charNo = 0;
		separator[lineNo][charNo++] = prev_ch;
		separator[lineNo][charNo++] = curr_ch;


	    } else {
		if (curr_ch != 0) {
		    putc(curr_ch, ofp);
		    lines[lineNo][charNo] = curr_ch;
		    deltas[lineNo][charNo] = prev_ch;
		    charNo++;
		}
	    }
	    break;
	case IN_SONG_BETWEEN:
	    read1();
	    if (curr_ch == '&') {
		state = IN_SONG_AND_LINE;

		separator[lineNo][charNo++] = curr_ch;

		charNo = 0;
		lineNo += 1;
	    }  else if (curr_ch == 0xFF) {
		if (half == 1) {
		    // the 2nd half
		    fprintf(ofp, "\n\nStarting 2nd half\n\n");
		    // discard extra
		    //getc(fp);
		    //lineNo = -1;
		    max_lines = lineNo;
		    half = 2;
		} else {
		    half = 3;
		}
	    } else {
		separator[lineNo][charNo++] = curr_ch;
	    }
	    break;
	}
    }

    fprintf(ofp, "\n\nDumping lines\n\n");
    
    first_lines = lines[0];
    second_lines = lines[0] + (max_lines+1) * LINE_LEN;

    first_deltas = deltas[0];
    second_deltas = deltas[0] + (max_lines+1) * LINE_LEN;

    first_separators = separator[0];
    second_separators = separator[0] + (max_lines+1) * LINE_LEN;

    for (n = 0; n < max_lines; n++) {
	fprintf(ofp, "%2d: ", n);
	/*
	if (lines[n][0] == 0) {
	    break;
	}
	*/
	//printLine(lines[n]);
	printLine(first_lines);
	first_lines += LINE_LEN;

	fprintf(ofp, "\n   ");

	//printDeltas(deltas[n]);
	printDeltas(first_deltas);
	first_deltas += LINE_LEN;

	//printSeparator(separator[n]);
	printSeparator(first_separators);
	first_separators += LINE_LEN;
	putc('\n', ofp);

	if (endSection(first_separators - LINE_LEN)) {
	    fprintf(ofp, "Break occurring in first line\n");
	    //continue;
	}

	fprintf(ofp, "%2d: ", n + max_lines + 1);
	//printLine(lines[n + max_lines + 1]);
	printLine(second_lines);
	second_lines += LINE_LEN;


	fprintf(ofp, "\n   ");
	//printDeltas(deltas[n + max_lines + 1]);
	printDeltas(second_deltas);
	second_deltas += LINE_LEN;

	//printSeparator(separator[n + max_lines + 1]);
	printSeparator(second_separators);
	second_separators += LINE_LEN;

	if (endSection(second_separators - LINE_LEN)) {
	    fprintf(ofp, "Break occurring in second line\n");
	    //continue;
	}

	/*
	for (m = 0; m < LINE_LEN/2; m++) {
	    fprintf(ofp, "%2X ", separator[n][m]);
	}
	*/
	putc('\n', ofp);
    }
    

    fclose(ofp);
    exit(0);
}

