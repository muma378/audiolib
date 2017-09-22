CC = g++
CFLAGS = -Wall -c -g -std=c++11 -I./include
LDFLAGS = 
TARGET = smart_truncate
SRCDIR = ./src/


$(TARGET): main.o audio.o common.o exceptions.o
	$(CC) main.o common.o audio.o exceptions.o -o $(TARGET) $(LDFLAGS)

main.o: $(SRCDIR)main.cpp audio.h exceptions.h
	$(CC) $(CFLAGS) $(SRCDIR)main.cpp

exceptions.o: exceptions.cpp exceptions.h
	$(CC) $(CFLAGS) $(SRCDIR)exceptions.cpp

audio.o: audio.cpp common.h audio.h exceptions.h riff.h
	$(CC) $(CFLAGS) $(SRCDIR)audio.cpp

common.o: common.cpp common.h
	$(CC) $(CFLAGS) $(SRCDIR)common.cpp	


clean:
	rm -f *.o *.c~ *.h~ *.log $(TARGET)


