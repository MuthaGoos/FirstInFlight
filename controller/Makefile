
#Source
CFILES = controller.c pid.c simulpid.c

#Rules
default: $(CFILES)
	$(CC) $(CFILES)
	mv *.o $(BUILD_DIR) 

all: default


clean:
	rm *.o 

