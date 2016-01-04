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
	fopen(argv[1], "r");
	fopen(argv[2], "w");
    }

    char ch;
    int n = 0;
    
    while (n++ < 1500) {
	ch = getc(fp);

	switch (ch) {
	case 0x13: ch = ' '; break;

	case 0x52: ch = 'a'; break;
	case 0x51: ch = 'b'; break;
	case 0x50: ch = 'c'; break;

	case 0x57: ch = 'd'; break;
	case 0x56: ch = 'e'; break;
	case 0x55: ch = 'f'; break;
	case 0x54: ch = 'g'; break;

	case 0x5B: ch = 'h'; break;
	case 0x5A: ch = 'i'; break;
	case 0x59: ch = 'j'; break;
	case 0x58: ch = 'k'; break;

	case 0x5F: ch = 'l'; break;
	case 0x5E: ch = 'm'; break;
	case 0x5D: ch = 'n'; break;
	case 0x5C: ch = 'o'; break;

	case 0x43: ch = 'p'; break;
	case 0x42: ch = 'q'; break;
	case 0x41: ch = 'r'; break;
	case 0x40: ch = 's'; break;

	case 0x47: ch = 't'; break;
	case 0x46: ch = 'u'; break;
	case 0x45: ch = 'v'; break;
	case 0x44: ch = 'w'; break;

	case 0x4A: ch = 'y'; break;

	    // These aren't organised yet
	case 0x72: ch = 'A'; break;
	case 0x76: ch = 'E'; break;
	case 0x74: ch = 'G'; break;
	case 0x7A: ch = 'I'; break;
	case 0x7F: ch = 'L'; break;
	case 0x7E: ch = 'M'; break;
	case 0x7D: ch = 'N'; break;
	case 0x7C: ch = 'O'; break;
	case 0x61: ch = 'R'; break;
	case 0x67: ch = 'T'; break;
	case 0x64: ch = 'W'; break;
	case 0x69: ch = 'Z'; break;

	case 0x09: ch = ':'; break;

	case 0x14: ch = '\''; break;

	default: ch = '.'; break;
	}
	putc(ch, ofp);
    }
    fclose(ofp);
    exit(0);
}
