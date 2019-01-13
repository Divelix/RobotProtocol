#pragma once
#include "message.h"
#include "exchange.h"
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
	virtual void work() = 0;
	void baseAlgorithm() {
		CMessage* msg = selfClient->recieve();
		if (msg != NULL) {
			messageProcessing(msg);
		}
		work();
	};
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

class CControlComponent : public ACBaseComponent {
	//targetPoint
	//work() {
	//	 закон управления на основе targetPoint
	//	 передача на драйверы
	//	 запрос у навигационной системы новые данные
	//}
	//	 W=1/(1+Ts), T > 0.05;

public:
	CTargetData* targetData;
	CNavigData* navigData;

	CControlComponent(CManager* manager) {
		selfAddr = { 0, 1, 0 };
		//partners.push_back(SAddress(0, 2, 0)); // !!! check on stack deletion
		//partners.push_back(SAddress(0, 3, 0));
		sockaddr_in serverAddr = manager->server.serverAddr;
		managerAddr = manager->selfAddr;
		selfClient = (ACExchangeInterface*) new CUdpClient();
		selfClient->open(inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
		// Ask the manager to register this component
		selfClient->send(new CMessage(HIGH, CONFIRM_NO, REGISTRATION, selfAddr, managerAddr, NULL));
	}

	void messageProcessing(CMessage* msg) override {
		if (msg->addrFrom.comp == 2) {
			//LOG("Data from TrajectoryComponent recieved");
			if (msg->msgType == TARGET_SET) {
				//LOG("New target");
				targetData = (CTargetData*)msg->msgData;
			}
		} else if (msg->addrFrom.comp == 3) {
			//LOG("Data from NavigationComponent recieved");
			if (msg->msgType == NAVIG_DATA) {
				//LOG("Fresh navig data");
				navigData = (CNavigData*)msg->msgData;
			}
		}
	}

	void work() override {
		//LOG("work() of controlCmp");
		//LOG("Navig data for control signal evaluation:");
		//navigData->toStringStream(std::cout);
	}
};

//rework this cmp
class CTrajectoryComponent : public ACBaseComponent {
public:
	// TARGET_SET
	CTargetData* targetData;

	CTrajectoryComponent(CManager* manager) {
		selfAddr = { 0, 2, 0 };
		partners.push_back(SAddress(0, 1, 0)); // !!! check on stack deletion
		partners.push_back(SAddress(0, 3, 0));
		sockaddr_in serverAddr = manager->server.serverAddr;
		managerAddr = manager->selfAddr;
		selfClient = (ACExchangeInterface*) new CUdpClient();
		selfClient->open(inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
		// Ask the manager to register this component
		selfClient->send(new CMessage(HIGH, CONFIRM_NO, REGISTRATION, selfAddr, managerAddr, NULL));
	}

	void messageProcessing(CMessage* msg) override {
		return;
	}
	void work() override {
		LOG("work() of trajectoryCmp");
		STarget target = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f };
		CTargetData* targetData = new CTargetData();
		targetData->unpack((char*)&target);
		CMessage* msg = new CMessage(HIGH, CONFIRM_NO, TARGET_SET, selfAddr, partners[0], (ACBaseType*)targetData);
		selfClient->send(msg);
	}
};

class CNavigComponent : ACBaseComponent {};