#include <iostream>
#define LOG(x) std::cout << x << std::endl
#include "message.h"
#include "exchange.h"
#include "manager.h"
#include "components.h"

#if TEST_PROTOCOL == 1 // Message test
int main() {
	STele tele = { 0.1f, 1.5f, 3.3f };
	CTeleData teleData;
	teleData.unpack((char*)&tele);
	teleData.toStringStream(std::cout);
	CMessage* message = new CMessage(HIGH, CONFIRM_YES, NAVIG_DATA, { 1, 2, 3 }, { 4, 5, 6 }, &teleData);
	LOG((int)message->priority);
	LOG((int)message->confirm);
	LOG((int)message->msgType);
	LOG("addrFrom: " << message->addrFrom);
	LOG("addrTo: " << message->addrTo);
	LOG("size_data = " << message->dataSize);
	/*LOG("Message: ");
	message->msgData.toStringStream(std::cout);*/
	LOG("size_msg = " << message->msgSize);
	LOG("---------------------------------------");
	CMessage* message2 = new CMessage(message->msg, message->msgSize);
	LOG((int)message2->priority);
	LOG((int)message2->confirm);
	LOG((int)message2->msgType);
	LOG("addrFrom: " << message2->addrFrom);
	LOG("addrTo: " << message2->addrTo);
	LOG("size_data = " << message2->dataSize);
	LOG("size_msg = " << message2->msgSize);

	delete message;
	delete message2;
	std::cin.get();
}
#elif TEST_PROTOCOL == 2 // Pack/unpack data
int main() {
	LOG("-------------------DATA---------------------");
	LOG(sizeof(SNavig));
	float fs[6] = { 1.0f, 1.1f, 1.1f, 5.0f, 5.1f, 5.2f };
	char* str = (char*)fs;
	//SNavig nav = { 1.0f, 1.1f, 1.1f, 5.0f, 5.1f, 5.2f };
	CNavigData data;
	data.unpack(str);
	char* ptr = new char[24];
	LOG(data);
	data.pack(ptr);
	LOG(data);

	data.toStringStream(std::cout);

	CTargetData target;
	LOG(target);


	std::cin.get();
}

#elif TEST_PROTOCOL	== 3 // UDP test
#include <thread>
using namespace std::literals::chrono_literals;

int main() {
	STele tele = { 0.1f, 1.5f, 3.3f };
	CTeleData teleData;
	teleData.unpack((char*)&tele);
	CMessage* message = new CMessage(HIGH, CONFIRM_YES, NAVIG_DATA, { 1, 2, 3 }, { 4, 5, 6 }, &teleData);
	CMessage* message2 = new CMessage(LOW, CONFIRM_YES, NAVIG_DATA, { 6, 5, 4 }, { 3, 2, 1 }, &teleData);

	CUdpServer server;
	CUdpClient client;
	CUdpClient client2;
	server.open("127.0.0.1", 50000);
	client.open("127.0.0.1", 50000);
	client2.open("127.0.0.1", 50000);
	client.send(message);
	client2.send(message2);
	std::this_thread::sleep_for(0.5s);
	CMessage* sRcvMsg1 = server.recieve();
	CMessage* sRcvMsg2 = server.recieve();
	server.send(message, &server.clientAddr, sizeof(sockaddr_in));
	CMessage* cRcvMsg = client.recieve();
	server.close();
	client.close();
	client2.close();
	delete message, message2;
	std::cin.get();
}

#elif TEST_PROTOCOL	== 4 // Map test

struct SPair {
	int* key;
	int* value;

	~SPair() {
		delete key;
		delete value;
	}
};

class CMap {
public:
	SPair** map;
	int initSize = 16;
	int size;

	CMap() : size(0) {
		map = new SPair*[initSize];
		memset(map, NULL, sizeof(SPair*) * initSize);
	}
	~CMap() {
		for (int i = 0; i < size; i++) {
			delete map[i];
		}
		memset(map, NULL, sizeof(SPair*) * initSize);
	}

	void add(int* a, int* b) {
		SPair* pair = new SPair();
		pair->key = a;
		pair->value = b;
		map[size] = pair;
		size++;
	}
};

int main() {
	/*CMap* map;
	for (int i = 0; i < 10; i++)  {
		map = new CMap();
		map->add(new int(11), new int(22));
		map->add(new int(33), new int(44));
		map->add(new int(55), new int(66));
		delete map;
	}*/

	SAddress* addr = new SAddress(1, 2, 3);
	ACExchangeInterface* server = (ACExchangeInterface*)new CUdpServer();

	CMapAddress* map = new CMapAddress();
	map->add(addr, server);
	ACExchangeInterface* serverIntf = map->get(addr);
	delete map;
}

#elif TEST_PROTOCOL	== 5 // Manager test

int main() {
	STarget target = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f };
	CTargetData targetData;
	targetData.unpack((char*)&target);
	targetData.toStringStream(std::cout);
	//CMessage* message = new CMessage(HIGH, CONFIRM_YES, TARGET_SET, { 0, 3, 0 }, { 0, 1, 0 }, &targetData);

	CManager* manager = new CManager("127.0.0.1", 50000);
	CControlComponent controlCmp(manager);
	manager->routing();

	controlCmp.die();
	manager->server.close();
	delete manager;
	std::cin.get();
}

#elif TEST_PROTOCOL	== 100 // Multithreading
#include <thread>

static bool isFinished = false;

void doLoad() {
	using namespace std::literals::chrono_literals;
	LOG("Started thread id = " << std::this_thread::get_id());

	while (!isFinished) {
		LOG("Loading...");
		std::this_thread::sleep_for(500ms);
	}
}

int main() {
	LOG("Started thread id = " << std::this_thread::get_id());
	std::thread loader(doLoad);
	std::cin.get();
	isFinished = true;

	loader.join();
	LOG("Started thread id = " << std::this_thread::get_id());

	std::cin.get();
}

#elif TEST_PROTOCOL	== 101 // Timing
#include <chrono>
#include <thread>

int main() {
	using namespace std::literals::chrono_literals;

	auto start = std::chrono::high_resolution_clock::now();
	std::this_thread::sleep_for(1s);
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<float> duration = end - start;
	LOG(duration.count());

	std::cin.get();
}

struct Timer {
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;

	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = start - end;

		float ms = duration.count() * 1000.0f;
		LOG("Timer took " << ms << "ms");
	}
};

#endif

