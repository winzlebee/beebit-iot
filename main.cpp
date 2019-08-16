
#include "beebit.h"

int main(int argc, char *argv[]) {
	beebit::PeopleCounter peopleCounter(0);
	peopleCounter.setBoxes(true);
	peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
	peopleCounter.begin();

	return 0;
}