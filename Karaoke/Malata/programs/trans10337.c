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

    unsigned int ch;
    int n = 0;
    
    while (n++ < 1500) {
	ch = getc(fp);

	switch (ch) {
	case 0x95: ch = ' '; break;

	case 0xD4: ch = 'a'; break;
	case 0xD7: ch = 'b'; break;
	case 0xD6: ch = 'c'; break;

	case 0xD1: ch = 'd'; break;
	case 0xD0: ch = 'e'; break;
	case 0xD3: ch = 'f'; break;
	case 0xD2: ch = 'g'; break;

	case 0xDD: ch = 'h'; break;
	case 0xDC: ch = 'i'; break;
	case 0xDF: ch = 'j'; break;
	case 0xDE: ch = 'k'; break;

	case 0xD9: ch = 'l'; break;
	case 0xD8: ch = 'm'; break;
	case 0xDB: ch = 'n'; break;
	case 0xDA: ch = 'o'; break;

	case 0xC5: ch = 'p'; break;
	case 0xC4: ch = 'q'; break;
	case 0xC7: ch = 'r'; break;
	case 0xC6: ch = 's'; break;

	case 0xC1: ch = 't'; break;
	case 0xC0: ch = 'u'; break;
	case 0xC3: ch = 'v'; break;
	case 0xC2: ch = 'w'; break;

	    //case 0x: ch = 'x'; break;
	case 0xCC: ch = 'y'; break;
	    //case 0x: ch = 'z'; break;
 
	case 0xF4: ch = 'A'; break;
	    //case 0xF3: ch = 'B'; break;
	    //case 0x: ch = 'C'; break;

	    case 0xF1: ch = 'D'; break;
	    case 0xF0: ch = 'E'; break;
	case 0xF3: ch = 'F'; break;
	case 0xF2: ch = 'G'; break;

	    //case 0x: ch = 'H'; break;
	case 0xFC: ch = 'I'; break;
	case 0xFF: ch = 'J'; break;
	    //case 0x: ch = 'K'; break;
	    
	case 0xF9: ch = 'L'; break;
	    case 0xF8: ch = 'M'; break;
	case 0xFB: ch = 'N'; break;
	case 0xFA: ch = 'O'; break;

	    case 0xE5: ch = 'P'; break;
	case 0xE4: ch = 'Q'; break;
	case 0xE7: ch = 'R'; break;
	    case 0xE6: ch = 'S'; break;
	    
	case 0xE1: ch = 'T'; break;
	    /*
	      case 0xch = 'U'; break;
	      case 0xch = 'V'; break;
	      case 0xch = 'W'; break;

	      case 0xch = 'X'; break;
	      case 0xch = 'Y'; break;
	      case 0xch = 'Z'; break;

	      case 0x: ch = ':'; break;
	    */
	    // case 0xch = '\''; break;
	    

	default: ch = '.'; break;
	}
	putc(ch, ofp);
    }
    fclose(ofp);
    exit(0);
}
