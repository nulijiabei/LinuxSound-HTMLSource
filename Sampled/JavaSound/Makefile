
%.class: %.java
	javac -g -deprecation  -classpath $(CLASSPATH) $<

CLASSPATH = .
CLASSES =  \
	DeviceInfo.class \
	PlayAudioFile.class \
	PlayMicrophone.class  \
	Recorder.class


all: $(CLASSES)


clean:
	rm *.class
