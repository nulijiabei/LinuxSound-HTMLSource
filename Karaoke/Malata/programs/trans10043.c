#include <stdio.h>

int main(int argc, char **argv) {
    FILE *fp;
    FILE *ofp;

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

    char ch;
    int n = 0;
    
    while (n++ < 1500) {
	ch = getc(fp);

	switch (ch) {
	case 0x7F: ch = ' '; break;

	case 0x3E: ch = 'a'; break;
	case 0x3D: ch = 'b'; break;
	case 0x3C: ch = 'c'; break;

	case 0x3B: ch = 'd'; break;
	case 0x3A: ch = 'e'; break;
	case 0x39: ch = 'f'; break;
	case 0x38: ch = 'g'; break;

	case 0x37: ch = 'h'; break;
	case 0x36: ch = 'i'; break;
	case 0x35: ch = 'j'; break;
	case 0x34: ch = 'k'; break;

	case 0x33: ch = 'l'; break;
	case 0x32: ch = 'm'; break;
	case 0x31: ch = 'n'; break;
	case 0x30: ch = 'o'; break;

	case 0x2F: ch = 'p'; break;
	case 0x2E: ch = 'q'; break;
	case 0x2D: ch = 'r'; break;
	case 0x2C: ch = 's'; break;

	case 0x2B: ch = 't'; break;
	case 0x2A: ch = 'u'; break;
	case 0x29: ch = 'v'; break;
	case 0x28: ch = 'w'; break;

	case 0x27: ch = 'x'; break;
	case 0x26: ch = 'y'; break;
	case 0x25: ch = 'z'; break;
 
	case 0x1E: ch = 'A'; break;
	case 0x1D: ch = 'B'; break;
	case 0x1C: ch = 'C'; break;

	case 0x1B: ch = 'D'; break;
	case 0x1A: ch = 'E'; break;
	case 0x19: ch = 'F'; break;
	case 0x18: ch = 'G'; break;

	case 0x17: ch = 'H'; break;
	case 0x16: ch = 'I'; break;
	case 0x15: ch = 'J'; break;
	case 0x14: ch = 'K'; break;

	case 0x13: ch = 'L'; break;
	case 0x12: ch = 'M'; break;
	case 0x11: ch = 'N'; break;
	case 0x10: ch = 'O'; break;

	case 0x0F: ch = 'P'; break;
	case 0x0E: ch = 'Q'; break;
	case 0x0D: ch = 'R'; break;
	case 0x0C: ch = 'S'; break;
	/*
	case 0x: ch = 'T'; break;
	case 0x: ch = 'U'; break;
	case 0x: ch = 'V'; break;
	case 0x: ch = 'W'; break;

	case 0x: ch = 'X'; break;
	case 0x: ch = 'Y'; break;
	case 0x: ch = 'Z'; break;

	case 0x65: ch = ':'; break;

	case 0x: ch = '\''; break;
	    */

	// case 0x1: ch = ^; break;
	//default: ch = '.'; break;
	}
	
	putc(ch, ofp);
    }
    fclose(ofp);
    exit(0);
}
