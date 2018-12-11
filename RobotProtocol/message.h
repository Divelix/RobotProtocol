#pragma once

//------------------------------ HEADER ----------------------------
enum EPriority { LOW, MIDDLE, HIGH, CRITICAL };
enum EConfirm { NOT_CONFIRM_NO, NOT_CONFIRM_YES, CONFIRM_NO, CONFIRM_YES };
enum ETypeMsg {
	//1) Navig request, answer (18 floats)
	NAVIG_QUERY, NAVIG_DATA,

	//2) Target _request_, command (12 floats)
	TARGET_QUERY, TARGET_SET,

	//3) Сервисные сообщения
	// a) IS_ALIVE (struct {char flag, int code})
	IS_ALIVE,

	//4) TELECONTROL
	// a) вектор линейной скорости, вектор вращательной скорости
	TELE_LIN_VEL, TELE_ROT_VEL,

	//5) Trajectory generation mode
	// a) точки
	// b) ломаная
	// c) кривая (сплайн)
	GEN_DOTS, GEN_BROKEN, GEN_CURVE,

	//6) START, STOP, PAUSE
	START, STOP, PAUSE

};

struct SAddress {
	unsigned char node, comp, instance;

	bool operator== (const SAddress* another) {
		const void* thisBuff1 = this;
		const void* anotherBuff2 = another;
		int compare = memcmp(thisBuff1, anotherBuff2, sizeof(thisBuff1));
		if (compare == 0) {
			return true;
		}
		return false;
	}

	friend std::ostream& operator<< (std::ostream& stream, const SAddress& addr) {
		return stream << "node = " << (int)addr.node << "; comp= " << (int)addr.comp << "; instance = " << (int)addr.instance << std::endl;
	}
};

//вычисление контрольной суммы
unsigned char CRC8(char *pcBlock, unsigned int len) {
	unsigned char crc = 0xFF;
	unsigned int i;

	while (len--) {
		crc ^= *pcBlock++;
		for (i = 0; i < 8; i++)
			crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
	}
	return crc;
}

//------------------------------DATA---------------------------------
// базовый класс для работы с данными в сообщении
class ACBaseType {
public:
	virtual int pack(char* buff) { return 0; }
	virtual void unpack(char* buff) {}
	virtual void toStringStream(std::ostream& stream) {};

	virtual ACBaseType* getCopy() const { return new ACBaseType(); }
};

#pragma pack(push, 1)
// Структуры для данных в сообщении
struct SNavig {
	float x, y, z, roll, pitch, yaw;
	float x1, y1, z1, roll1, pitch1, yaw1;
	float x2, y2, z2, roll2, pitch2, yaw2;
};

struct STarget {
	float x, y, z, roll, pitch, yaw;
	float x1, y1, z1, roll1, pitch1, yaw1;
};

struct STele {
	float v1, v2, v3;
};
#pragma pack(pop)

// Клаcс для работы с данными навигации
class CNavigData : public ACBaseType {
public:
	SNavig navig;

	int pack(char* buff) {
		int size = sizeof(SNavig);
		memcpy(buff, &navig, size);
		return size;
	}

	void unpack(char* buff) {
		memcpy(&navig, buff, sizeof(SNavig));
	}

	void toStringStream(std::ostream& stream) {
		stream << "NavigData:\n"
			<< " Position: " << navig.x << ", " << navig.y << ", " << navig.z << ";\n"
			<< " Orientation: " << navig.roll << ", " << navig.pitch << ", " << navig.yaw << ";\n"
			<< " Velocity: " << navig.x1 << ", " << navig.y1 << ", " << navig.z1 << ";\n"
			<< " Rotational velocity: " << navig.roll1 << ", " << navig.pitch1 << ", " << navig.yaw1 << ";\n"
			<< " Acceleration: " << navig.x2 << ", " << navig.y2 << ", " << navig.z2 << ";\n"
			<< " Angular acceleration: " << navig.roll2 << ", " << navig.pitch2 << ", " << navig.yaw2 << ";\n";
	}
};

class CTargetData : public ACBaseType {
public:
	STarget target;

	int pack(char* buff) {
		int size = sizeof(STarget);
		memcpy(buff, &target, size);
		return size;
	}

	void unpack(char* buff) {
		memcpy(&target, buff, sizeof(STarget));
	}

	void toStringStream(std::ostream& stream) {
		stream << "NavigData:\n"
			<< " Position: " << target.x << ", " << target.y << ", " << target.z << ";\n"
			<< " Orientation: " << target.roll << ", " << target.pitch << ", " << target.yaw << ";\n"
			<< " Velocity: " << target.x1 << ", " << target.y1 << ", " << target.z1 << ";\n"
			<< " Rotational velocity: " << target.roll1 << ", " << target.pitch1 << ", " << target.yaw1 << ";\n";
	}
};

class CTeleData : public ACBaseType {
public:
	STele telemetry;

	int pack(char* buff) {
		int size = sizeof(STele);
		memcpy(buff, &telemetry, size);
		return size;
	}

	void unpack(char* buff) {
		memcpy(&telemetry, buff, sizeof(STele));
	}

	void toStringStream(std::ostream& stream) {
		stream << "TeleData:\n"
			<< " Velocity: " << telemetry.v1 << ", " << telemetry.v2 << ", " << telemetry.v3 << ";\n";
	}
};

//-------------------------------------MESSSAGE----------------------------------
#define SOH 'Z'
#define marker 5
class CMessage {
public:
	EPriority priority;
	EConfirm confirm;
	ETypeMsg msgType;

	SAddress addrFrom, addrTo;
	unsigned int dataSize;
	ACBaseType* msgData;
	char msg[262];
	unsigned int msgSize;

	// Передача
	CMessage(EPriority prior, EConfirm conf, ETypeMsg type, SAddress addrF, SAddress addrT, ACBaseType* data = nullptr) :
		priority(prior), confirm(conf), msgType(type), addrFrom(addrF), addrTo(addrT), msgData(data) {
		dataSize = msgData->pack(msg + 6);
		msgSize = dataSize + 7;
		marshal();
		msg[msgSize - 1] = CRC8(msg, msgSize - 1);
	}

	// Приём
	CMessage(char* buff, int size) {
		for (int i = 0; i < size; i++) {
			if (buff[i] == SOH && ((buff[i + 4] & 7) == marker)) {
				dataSize = buff[i + 5];
				msgSize = dataSize + 7;
				memcpy(msg, buff + i, msgSize - 1);
				if (CRC8(msg, msgSize - 1) == (unsigned char)buff[i + msgSize - 1]) {
					memcpy(msg, buff + i, msgSize);
					unmarshal();
					break;
				}
			}
		}
	}

	~CMessage() {
		std::cout << "Message destructor happend" << std::endl;
	}

	bool marshal() {
		msg[0] = SOH;
		msg[1] = (addrFrom.node << 4) + (addrFrom.comp << 1) + (addrFrom.instance >> 2);
		msg[2] = (addrFrom.instance << 6) + (addrTo.node << 2) + (addrTo.comp >> 1);
		msg[3] = (addrTo.comp << 7) + (addrTo.instance << 4) + (priority << 2) + confirm;
		msg[4] = (msgType << 3) + marker;
		msg[5] = dataSize;
		return true;
	}

	bool unmarshal() {
		if (msg[0] == SOH && (msg[4] & 7) == marker) {
			addrFrom.node = msg[1] >> 4;
			addrFrom.comp = (msg[1] >> 1) & 7;
			addrFrom.instance = (msg[1] << 2) & 4 | (msg[2] >> 6) & 3;
			addrTo.node = (msg[2] >> 2) & 15;
			addrTo.comp = (msg[2] << 1) & 6 | (msg[3] >> 7) & 1;
			addrTo.instance = (msg[3] >> 4) & 7;
			priority = (EPriority)((msg[3] >> 2) & 3);
			confirm = (EConfirm)(msg[3] & 3);
			msgType = (ETypeMsg)(msg[4] >> 3);
			dataSize = msg[5];
			return true;
		}
		return false;
	}
};