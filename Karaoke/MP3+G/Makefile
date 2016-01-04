

all:
	javac newmarch/songtable/*.java
	jar cfm SongTable.jar Manifest.txt newmarch/songtable/*.class
	make zip

#	(cd Table; make)

zip:
	zip all.zip run newmarch/songtable/*.java


#	zip all.zip run Table/*.java Table/*.c
