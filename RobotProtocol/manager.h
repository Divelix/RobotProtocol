// Manager that transfer messages with UDP
#pragma once
#include "message.h"
#include "exchange.h"
#include <windows.h>
#include <vector>
#include <chrono>

struct SElementMap {
	SAddress* key;
	ACExchangeInterface* value;
};

class CMapAddress {
private:
	SElementMap** map;
	int size;
public:
	CMapAddress() :
		size(0) {
		map = new SElementMap*[16];
		memset(map, NULL, sizeof(SElementMap) * 16);
	}
	~CMapAddress() {
		for (int i = 0; i < 16; i++) {
			delete map[i];
		}
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
private:
	CUdpServer server;
	CMapAddress map;
public:
	CManager(const char* ip, int port) {
		server.open(ip, port);
	}

	void routing() {
		CMessage* msg = server.recieve();
		SAddress addrFrom = msg->addrFrom;
		SAddress addrTo = msg->addrTo;
		if (map.get(&addrFrom) == NULL) {
			CUdpServerIdle* serverIdle = new CUdpServerIdle(&server);
			map.add(&addrFrom, (ACExchangeInterface*)&serverIdle);
		}
		// Sleep(100); // Millis
	}
};

//--------Component----------
/*
ACBaseComponent
	SAddress addr - свой адрес
	SAddress addrManager - адрес менеджера
	ACExchangeIntf int - клиент (UDP)
	vector<SAdderss> partners - адреса компонентов, с которыми взаимодействует этот компонент

	virtual void(или int) run(CMessage*) = 0 - прочитать сообщение
	virtual void start(); - обобщенный алгоритм компонента (запускает run() (если есть CMessage), запускает work() (всегда))
	virtual void work(); - работа компонента (например опрос датчиков)
	bool s_time_update(u_int millis) - прошло ли millis времени от вызова этой функции (из time.h) (не задержка потока!)
*/
class ACBaseComponent {
	SAddress selfAddr;
	SAddress managerAddr;
	ACExchangeInterface* selfClient;
	std::vector<SAddress> partners;

	virtual void run(CMessage* msg) = 0;
	virtual void start();
	virtual void work();
	bool sTimeUpdate(u_int millis);
};

/*
Реализовать компоненты :
- Система формирования траектории
- Система управления
- Навигационная система
*/

class CTrajectoryComponent : ACBaseComponent {
	CTrajectoryComponent(const char* managerIp, int managerPort) {
		
	}

	void run(CMessage* msg) {}
	void start() {}
	void work() {}
	bool sTimeUpdate(u_int millis) {
		auto start = std::chrono::steady_clock::now();

		auto end = std::chrono::steady_clock::now();
		double elapsedTimeMillis = double(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count());
	}
};
class CControlComponent : ACBaseComponent {};
class CNavigComponent : ACBaseComponent {};