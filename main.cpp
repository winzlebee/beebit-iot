
#include "daemon/daemon.h"

int main(int argc, char *argv[]) {

	// Start the daemon and begin listening
	beebit::Daemon daemon("http://app.beebithive.com");
	daemon.start();

	return 0;
}