// Manager that transfer messages with UDP
#pragma once
#include "message.h"
#include "exchange.h"

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
		LOG("MAP DESTRUCTOR JUST HAPPEND");
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
			if (*(map[i]->key) == *key) {
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
	CMapAddress* map;

	CManager(const char* ip, int port) {
		selfAddr = { 0, 0, 0 };
		map = new CMapAddress();
		server.open(ip, port);
	}
	~CManager() {
		LOG("MANAGER DESTRUCTOR");
		delete map;
	}

	void routing() {
		CMessage* msg = server.recieve();
		if (msg == NULL) return;
		ETypeMsg type = msg->msgType; // почекать разные сервисные сообщения
		SAddress addrFrom = msg->addrFrom;
		SAddress* addrTo = new SAddress(msg->addrTo);

		// component registration
		if (type == REGISTRATION && map->get(&addrFrom) == NULL) {
			map->add(new SAddress(addrFrom), (ACExchangeInterface*)new CUdpServerIdle(server));
			LOG("REGISTRATION IS DONE");
			delete msg;
		}

		// if the message is not for manager (system message)
		if (addrTo->comp != 0) {
			if (map->get(addrTo) == NULL) {
				LOG("ERROR: wrong address or reciever component is not registered in manager");
				//CMessage* ERROR_MESSAGE; // make error message
				//server.send(ERROR_MESSAGE, (void*)&(server.clientAddr), sizeof(sockaddr_in));
			}
			CUdpServer* componentInterface = (CUdpServer*)map->get(addrTo);
			sockaddr_in componentAddr = componentInterface->clientAddr;
			server.send(msg, (void*)&componentAddr, sizeof(sockaddr_in));
		}
		return;
		// Sleep(100); // Millis?
		// TODO check if 100 microsec passed
	}
};