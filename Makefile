
CC=g++
OBJECTS=main.o beebit.o bee_util.o centroid_tracker.o beenet.o
LIBS=-lopencv_core -lopencv_videoio -lopencv_dnn -lopencv_imgproc -lopencv_highgui -lopencv_tracking -lcurl -I/usr/local/include/opencv4
DEPENDENCIES=yolov3.weights config.cfg
OUTPUT=beetrack

release : $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(OUTPUT) -O3

debug : $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(OUTPUT) -g

main.o : beebit.h
	$(CC) -c $(LIBS) main.cpp 
beebit.o : beebit.h bee_util.h net/beenet.h tracking/centroid_tracker.h tracking/trackable_object.h
	$(CC) -c $(LIBS) beebit.cpp
bee_util.o : bee_util.h
	$(CC) -c $(LIBS) bee_util.cpp
centroid_tracker.o : tracking/centroid_tracker.h
	$(CC) -c $(LIBS) tracking/centroid_tracker.cpp
beenet.o : net/beenet.h
	$(CC) -c $(LIBS) net/beenet.cpp

# The install target downloads all the weights and config files needed
deps: debug
	mkdir dnn
	wget https://pjreddie.com/media/files/yolov3-tiny.weights -O dnn/yolov3.weights
	wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3-tiny.cfg -O dnn/config.cfg

.PHONY : clean
clean:
	rm -r beetrack ./dnn beebit.cfg
	rm ./*.o
