
#include "daemon.h"

#include <curl/curl.h>
#include <thread>

#include "../bee_util.h"
#include "../beebit.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";

Daemon::Daemon(const std::string &endpoint)
    : m_lifetime(0)
    , m_config(readConfiguration(configLoc))
{
}

Daemon::~Daemon() {

}

void Daemon::start() {

    beebit::PeopleCounter peopleCounter(0);
	peopleCounter.setDebugWindow(true);
	//peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
	peopleCounter.begin();
}

}