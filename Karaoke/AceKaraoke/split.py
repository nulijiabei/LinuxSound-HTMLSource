import re

num = 0

def sanitize(s):
    #s = str.replace("\'", "\\\'")
    #s = re.sub("'", "\\\'", s)
    return s

# 1st 2 form dir, alst 3 form file 
dirSplit = re.compile("(..)(...)")
def copySong(num, title, artist):
    result = dirSplit.match(num)
    dir = result.group(1)
    file = result.group(2)

    title = sanitize(title)
    artist = sanitize(artist)

    command = "avconv -i /mnt/" + dir + "/" + file + ".avi" +\
              " -s 480x360 \"/home/httpd/html/KARAOKE/KARAOKE/Ace/ACE-" +\
              dir + file + " - " + artist + " - " + title + ".mpg\""
    print command

# number + ' ' + title + ' ' + artist
songSplit = re.compile("([0-9]{5} )(.+)  +(.*)")

# number + ' ' + title
halfSongSplit = re.compile("([0-9]{5} )(.+)")

def breakSong(line):
    line = line.rstrip()
    result = songSplit.match(line)
    if result:
        #print "num " + result.group(1) + " title " + result.group(2) +\
        #       " aritst " + result.group(3)
        num = result.group(1)
        title = result.group(2).strip().rstrip()
        artist = result.group(3).strip().rstrip()
        #print "num " + num + " title " + title +\
        #    " aritst " + artist
        copySong(num, title, artist)
    else:
        result = halfSongSplit.match(line)
        if result:
            # print "num " + result.group(1) + " title " + result.group(2)
            num = result.group(1)
            title = result.group(2).strip().rstrip()
            artist = ""
            copySong(num, title, artist)
        else:
            print "No match " + line

# split into two based on the 5 digit number pattern
lineSplit = re.compile("([0-9]{5}.*)([0-9]{5}.*)")
with open("aceout.txt", "r") as f:
    for line in f:
        num += 1
        #print line + " ..."
        result = lineSplit.match(line)
        if result:
            first = result.group(1)
            breakSong(first)
            second = result.group(2)
            breakSong(second)
        #print num
