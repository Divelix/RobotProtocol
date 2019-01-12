// Manager that transfer messages with UDP
#pragma once
#include "message.h"
#include "exchange.h"
#include <vector>
#include <chrono>

struct SElementMap {
	SAddress* key;
	ACExchangeInterface* value;

	~SElementMap() {
		delete key;
		delete value;
	}
};

class CMapAddress {
private:
	SElementMap** map;
	int capacity = 16; // max number of elements in map
	int size; // current number of elements in map
public:
	CMapAddress() :	size(0) {
		map = new SElementMap*[capacity];
		memset(map, NULL, sizeof(SElementMap*) * capacity);
	}
	~CMapAddress() {
		for (int i = 0; i < size; i++) {
			delete map[i];
		}
		delete map;
	}

	void add(SAddress* addr, ACExchangeInterface* intf) {
		SElementMap* newElement = new SElementMap();
		newElement->key = addr;
		newElement->value = intf;
		map[size] = newElement;
		size++;
	}

	ACExchangeInterface* get(SAddress* key) {
		for (int i = 0; i < size; i++) {
			if (map[i]->key == key) {
				return map[i]->value;
			}
		}
		return NULL;
	}

	void replace(SAddress* key, ACExchangeInterface* newInterface) {
		for (int i = 0; i < size; i++) {
			if (map[i]->key == key) {
				map[i]->value = newInterface;
			}
		}
	}

	int getCount() {
		return size;
	}
};
/*
Manager(ip, port)
-Открыть сервер
-Создать пустой Map

routing()
-Если адреса получателя нет в Map - отправить обратно в Компонент-отправитель
-Логировать заголовок сообщения
*/
class CManager {
public:
	SAddress selfAddr;
	CUdpServer server;
	CMapAddress map;

	CManager(const char* ip, int port) {
		selfAddr = SAddress(0, 0, 0);
		server.open(ip, port);
	}

	void routing() {
		CMessage* msg = server.recieve();
		if (msg == NULL) return;
		ETypeMsg type = msg->msgType; // почекать разные сервисные сообщения
		SAddress addrFrom = msg->addrFrom;
		SAddress addrTo = msg->addrTo;

		// component registration
		if (type == REGISTRATION && map.get(&addrFrom) == NULL) {
			CUdpServerIdle* serverIdle = new CUdpServerIdle(&server);
			map.add(&addrFrom, (ACExchangeInterface*)&serverIdle);
			LOG("REGISTRATION IS DONE");
			delete msg;
		}

		// If reciver component is absent
		if (map.get(&addrTo) == NULL && addrTo.comp != 0) {
			LOG("ERROR: reciever component is not registered in manager");
			//CMessage* ERROR_MESSAGE; // make error message
			//server.send(ERROR_MESSAGE, (void*)&(server.clientAddr), sizeof(sockaddr_in));
		}

		// if the message is not for manager (system message)
		if (addrTo.comp != 0) {
			CUdpServer* componentInterface = (CUdpServer*)map.get(&addrTo);
			sockaddr_in componentAddr = componentInterface->clientAddr;
			server.send(msg, (void*)&componentAddr, sizeof(sockaddr_in));
		}
		// Sleep(100); // Millis?
		// TODO check if 100 microsec passed
	}
};

//--------Component----------
/*
ACBaseComponent
	SAddress addr - свой адрес
	SAddress addrManager - адрес менеджера
	ACExchangeIntf int - клиент (UDP)
	vector<SAdderss> partners - адреса компонентов, с которыми взаимодействует этот компонент

	virtual void(или int) messageProcessing(CMessage*) = 0 - прочитать сообщение
	virtual void baseAlgorithm(); - обобщенный алгоритм компонента (запускает messageProcessing() (если есть CMessage), запускает work() (всегда))
	virtual void work(); - работа компонента (например опрос датчиков)
	bool is_time_update(u_int millis) - прошло ли millis времени от вызова этой функции (из time.h) (не задержка потока!)
*/
class ACBaseComponent {
public:
	SAddress selfAddr;
	SAddress managerAddr;
	ACExchangeInterface* selfClient;
	std::vector<SAddress> partners;

	//virtual void init(const char* managerIp, int managerPort) = 0;
	virtual void messageProcessing(CMessage* msg) = 0;
	virtual void baseAlgorithm() {
		CMessage* msg = selfClient->recieve();
		if (msg != NULL) {
			messageProcessing(msg);
		}
		work();
	};
	virtual void work() = 0;
	bool isTimeUpdate(u_int millis) {
		//auto start = std::chrono::steady_clock::now();
		//auto end = std::chrono::steady_clock::now();
		//double elapsedTimeMillis = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}

	void die() {
		selfClient->close();
		delete selfClient;
		LOG("Component is dead now");
	}
};

/*
Реализовать компоненты :
- Система управления
- Система формирования траектории
- Навигационная система
*/
class CControlComponent : public ACBaseComponent {
	//targetPoint
	//work() {
	//	 закон управления на основе targetPoint
	//	 передача на драйверы
	//	 запрос у навигационной системы новые данные
	//}
	//	 W=1/(1+Ts), T > 0.05;

public:
	bool isRegistered = false;
	CTargetData* targetData;

	CControlComponent(CManager* manager) {
		selfAddr = SAddress(0, 1, 0);
		partners.push_back(SAddress(0, 2, 0)); // !!! check on stack deletion
		partners.push_back(SAddress(0, 3, 0));
		sockaddr_in serverAddr = manager->server.serverAddr;
		managerAddr = manager->selfAddr;
		selfClient = (ACExchangeInterface*) new CUdpClient();
		selfClient->open(inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
		// Registration message to manager
		selfClient->send(new CMessage(HIGH, CONFIRM_NO, REGISTRATION, selfAddr, managerAddr, NULL));
	}

	void messageProcessing(CMessage* msg) override {
		switch (msg->addrFrom.comp) {
		case 2:
			LOG("Data from TrajectoryComponent recieved");
		case 3:
			LOG("Data from NavigationComponent recieved");
		}
	}

	void work() override {
		// Transfer function, etc.
	}
};

//rework this cmp
class CTrajectoryComponent : ACBaseComponent {
public:
	CTargetData* targetData;

	CTrajectoryComponent(const char* managerIp, int managerPort) {
		selfAddr.node = 0;
		selfAddr.comp = 2;
		selfAddr.instance = 0;
		selfClient->open(managerIp, managerPort);
	}

	void messageProcessing(CMessage* msg) override {
		if (msg->msgType == TARGET_QUERY) {
			selfClient->send(msg);
		}
		else if (msg->msgType == TARGET_SET) {
			targetData = (CTargetData*)(msg->msgData);
		}
	}
	void work() override {
		//
		//
		targetData->toStringStream(std::cout);
	}
};

class CNavigComponent : ACBaseComponent {};