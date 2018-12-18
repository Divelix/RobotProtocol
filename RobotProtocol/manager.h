// Manager that transfer messages with UDP
#pragma once
#include "message.h"
#include "exchange.h"

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
class Manager {
private:
	CUdpServer server;
	CMapAddress map;
public:
	Manager(const char* ip, int port) {
		server.open(ip, port);
	}

	void routing() {

	}
};