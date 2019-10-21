# Make up to C++17 standard for variant types.
OBJECTS=main.o beebit.o bee_util.o centroid_tracker.o beenet.o daemon.o
LIBS= -std=c++14 -lopencv_core -lopencv_videoio -lopencv_dnn -lopencv_imgproc -lopencv_highgui -lpthread -lcurl -I/usr/local/include
DEPENDENCIES=yolov3.weights config.cfg
OUTPUT=beetrack

all: pc

raspi : LIBS += -lraspicam -lraspicam_cv -DRASPI=1 -I/opt/opencv-4.1.1-arm/include/opencv4
raspi : $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $(OUTPUT)

pc : LIBS += -lopencv_tracking -I/usr/local/include/opencv4
pc : $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $(OUTPUT)

main.o : daemon/daemon.h main.cpp
	$(CXX) -c $(LIBS) main.cpp 
daemon.o : daemon/daemon.h daemon/daemon.cpp util/types.h
	$(CXX) -c $(LIBS) daemon/daemon.cpp
beebit.o : beebit.h bee_util.h net/beenet.h tracking/centroid_tracker.h tracking/trackable_object.h beebit.cpp util/types.h
	$(CXX) -c $(LIBS) beebit.cpp
bee_util.o : bee_util.h bee_util.cpp
	$(CXX) -c $(LIBS) bee_util.cpp
centroid_tracker.o : tracking/centroid_tracker.h tracking/centroid_tracker.cpp
	$(CXX) -c $(LIBS) tracking/centroid_tracker.cpp
beenet.o : net/beenet.h net/beenet.cpp
	$(CXX) -c $(LIBS) net/beenet.cpp

install: raspi
	bash ./scripts/install_raspi.sh

uninstall:
	bash ./scripts/uninstall_raspi.sh

depsPi:
	bash ./scripts/install_pi_dependencies.sh
	mkdir dnn
	wget https://pjreddie.com/media/files/yolov3-tiny.weights -O dnn/yolov3.weights
	wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3-tiny.cfg -O dnn/config.cfg

# The install target downloads all the weights and config files needed
deps: pc
	sh ./scripts/install_opencv.sh
	mkdir dnn
	wget https://pjreddie.com/media/files/yolov3-tiny.weights -O dnn/yolov3.weights
	wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3-tiny.cfg -O dnn/config.cfg

.PHONY : clean
clean:
	rm -r beetrack ./dnn beebit.cfg
	rm ./*.o
