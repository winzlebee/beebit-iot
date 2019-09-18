
CC=g++ -std=c++17
OBJECTS=main.o beebit.o bee_util.o centroid_tracker.o beenet.o daemon.o
LIBS=-lopencv_core -lopencv_videoio -lopencv_dnn -lopencv_imgproc -lopencv_highgui -lopencv_tracking -lpthread -lcurl -I/usr/local/include/opencv4
DEPENDENCIES=yolov3.weights config.cfg
OUTPUT=beetrack

release : $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(OUTPUT) -O3

debug : $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(OUTPUT) -g

main.o : beebit.h main.cpp
	$(CC) -c $(LIBS) main.cpp 
daemon.o : daemon/daemon.h daemon/daemon.cpp
	$(CC) -c $(LIBS) daemon/daemon.cpp
beebit.o : beebit.h bee_util.h net/beenet.h tracking/centroid_tracker.h tracking/trackable_object.h
	$(CC) -c $(LIBS) beebit.cpp
bee_util.o : bee_util.h bee_util.cpp
	$(CC) -c $(LIBS) bee_util.cpp
centroid_tracker.o : tracking/centroid_tracker.h tracking/centroid_tracker.cpp
	$(CC) -c $(LIBS) tracking/centroid_tracker.cpp
beenet.o : net/beenet.h net/beenet.cpp
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
