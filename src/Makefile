CC=gcc
LINK=gcc
TARGET1=main.exe
TARGET2=mainstatic.exe
OBJSMAIN=main.o file.o GPIB_prof.o measurements.o runtime.o serial.o setup.o utils.o
LIBS=-pthread
CFLAGS=-g -Wall -Wextra

all: ${TARGET1} ${TARGET2} ${TARGET3} ${TARGET4} 

${TARGET1}: ${OBJSMAIN}
	${CC} -o ${TARGET1} ${OBJSMAIN} ${LIBS}

${TARGET2}: ${OBJSMAIN}
	${CC} -static -o ${TARGET2} ${OBJSMAIN} ${LIBS}

.PHONY : clean

clean:
	del -f ${TARGET1} core*
	del -f ${TARGET2} core*
	del -f *.o core*