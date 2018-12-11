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
};