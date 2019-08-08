
FILES=main.cpp beebit.cpp bee_util.cpp
LIBS=-lopencv_core -lopencv_videoio -lopencv_dnn -I/usr/local/include/opencv4
DEPENDENCIES=yolov3.weights config.cfg
OUTPUT=beetrack

build:
	g++ $(FILES) $(LIBS) -o $(OUTPUT) -g

# The install target downloads all the weights and config files needed
deps: build
	mkdir dnn
	wget https://pjreddie.com/media/files/yolov3.weights -O dnn/yolov3.weights
	wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3.cfg -O dnn/config.cfg

.PHONY : clean
clean:
	rm -r beetrack ./dnn
