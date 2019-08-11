
FILES=main.cpp beebit.cpp bee_util.cpp tracking/centroid_tracker.cpp net/beenet.cpp
LIBS=-lopencv_core -lopencv_videoio -lopencv_dnn -lopencv_imgproc -lopencv_highgui -lopencv_tracking -I/usr/local/include/opencv4
DEPENDENCIES=yolov3.weights config.cfg
OUTPUT=beetrack

build:
	g++ $(FILES) $(LIBS) -o $(OUTPUT) -g

# The install target downloads all the weights and config files needed
deps: build
	mkdir dnn
	wget https://pjreddie.com/media/files/yolov3-tiny.weights -O dnn/yolov3.weights
	wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3-tiny.cfg -O dnn/config.cfg

.PHONY : clean
clean:
	rm -r beetrack ./dnn beebit.cfg
