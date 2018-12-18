#include <iostream>
#include "message.h"
#include "exchange.h"
#define LOG(x) std::cout << x << std::endl

#if TEST_PROTOCOL == 1 // Создание сообщения
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
#elif TEST_PROTOCOL == 2 // Запаковка/распаковка данных сообщения
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
	std::vector<CMessage> serverRcvMsgs = server.recieve();
	server.send(message, &server.clients[0], sizeof(sockaddr_in));
	server.send(message2, &server.clients[0], sizeof(sockaddr_in));
	std::vector<CMessage> clientRcvMsgs = client.recieve();
	server.close();
	client.close();
	client2.close();
	delete message, message2;
	std::cin.get();
}

#elif TEST_PROTOCOL	== 4 // Manager test
#include "manager.h"

int main() {
	SAddress addr;
	addr.node = (char)5;
	addr.comp = (char)4;
	addr.instance = (char)3;
	CUdpServer server;

	CMapAddress map;
	map.add(&addr, (ACExchangeInterface*)&server);
	ACExchangeInterface* serverFromMap = map.get(&addr);
	std::cin.get();
}

#elif TEST_PROTOCOL	== 100 // Multithreading
#include <thread>

static bool isFinished = false;

void doLoad() {
	using namespace std::literals::chrono_literals;
	std::cout << "Started thread id = " << std::this_thread::get_id() << std::endl;

	while (!isFinished) {
		std::cout << "Loading..." << std::endl;
		std::this_thread::sleep_for(500ms);
	}
}

int main() {
	std::cout << "Started thread id = " << std::this_thread::get_id() << std::endl;
	std::thread loader(doLoad);
	std::cin.get();
	isFinished = true;

	loader.join();
	std::cout << "Started thread id = " << std::this_thread::get_id() << std::endl;

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
	std::cout << duration.count() << std::endl;

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
		std::cout << "Timer took " << ms << "ms" << std::endl;
	}
};

#endif

