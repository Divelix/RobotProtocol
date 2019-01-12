#pragma once
#include "message.h"
#include "exchange.h"
//--------Component----------
/*
ACBaseComponent
	SAddress addr - ���� �����
	SAddress addrManager - ����� ���������
	ACExchangeIntf int - ������ (UDP)
	vector<SAdderss> partners - ������ �����������, � �������� ��������������� ���� ���������

	virtual void(��� int) messageProcessing(CMessage*) = 0 - ��������� ���������
	virtual void baseAlgorithm(); - ���������� �������� ���������� (��������� messageProcessing() (���� ���� CMessage), ��������� work() (������))
	virtual void work(); - ������ ���������� (�������� ����� ��������)
	bool is_time_update(u_int millis) - ������ �� millis ������� �� ������ ���� ������� (�� time.h) (�� �������� ������!)
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
����������� ���������� :
- ������� ����������
- ������� ������������ ����������
- ������������� �������
*/
class CControlComponent : public ACBaseComponent {
	//targetPoint
	//work() {
	//	 ����� ���������� �� ������ targetPoint
	//	 �������� �� ��������
	//	 ������ � ������������� ������� ����� ������
	//}
	//	 W=1/(1+Ts), T > 0.05;

public:
	// NAVIG_QUERY, 
	CTargetData* targetData;

	CControlComponent(CManager* manager) {
		selfAddr = SAddress(0, 1, 0);
		partners.push_back(SAddress(0, 2, 0)); // !!! check on stack deletion
		partners.push_back(SAddress(0, 3, 0));
		sockaddr_in serverAddr = manager->server.serverAddr;
		managerAddr = manager->selfAddr;
		selfClient = (ACExchangeInterface*) new CUdpClient();
		selfClient->open(inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
		// Ask the manager to register this component
		selfClient->send(new CMessage(HIGH, CONFIRM_NO, REGISTRATION, selfAddr, managerAddr, NULL));
	}

	void messageProcessing(CMessage* msg) override {
		switch (msg->addrFrom.comp) {
		case 2:
			LOG("Data from TrajectoryComponent recieved");
			if (msg->msgType == ) {

			}
			*targetData = msg->msgData;
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
	// TARGET_SET
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