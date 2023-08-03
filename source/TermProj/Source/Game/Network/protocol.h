#pragma once

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;
const int  WORLD_HEIGHT = 2000;
const int  WORLD_WIDTH = 2000;
const int  MAX_NAME_SIZE = 20;
const int  MAX_USER = 10000;
const int MAX_CHAT_SIZE = 100;
const int  MAX_NPC = 20'000;
const int MAX_CONVNPC = 10;
constexpr int NPC_ID_START = MAX_USER;
constexpr int NPC_ID_END = MAX_USER + MAX_NPC - 1;
constexpr int CONVNPC_ID_START = MAX_USER + MAX_NPC;
constexpr int CONVNPC_ID_END = CONVNPC_ID_START + MAX_CONVNPC - 1;

constexpr int NPC_PLANT = NPC_ID_START + MAX_NPC / 3;
constexpr int NPC_MONSTER = MAX_NPC / 3;
constexpr int NPC_SECTOR_MONSTER1 = NPC_PLANT + (NPC_MONSTER / 3);
constexpr int NPC_SECTOR_MONSTER2 = NPC_SECTOR_MONSTER1 + (NPC_MONSTER / 3);
constexpr int NPC_SECTOR_MONSTER3 = NPC_SECTOR_MONSTER2 + (NPC_MONSTER / 3);

//constexpr int NPC_PLANT = 140'000;
//constexpr int NPC_MONSTER = MAX_NPC / 3;
//constexpr int NPC_SECTOR_MONSTER1 = 160'000;
//constexpr int NPC_SECTOR_MONSTER2 = 180'000;
//constexpr int NPC_SECTOR_MONSTER3 = 200'000;

//constexpr int NPC_PLANT = NPC_ID_START + MAX_NPC - MAX_NPC / 100;
//constexpr int NPC_MONSTER = NPC_PLANT + ((MAX_USER + MAX_NPC - NPC_PLANT) - ((MAX_USER + MAX_NPC - NPC_PLANT) / 10));

const char OBJECT_PLAYER = 0;
const char OBJECT_PLANT = 1;
const char OBJECT_NORMALMONSTER = 2;
const char OBJECT_ANGRYMONSTER = 3;
const char OBJECT_VERYANGRYMONSTER = 4;
const char OBJECT_BOSS = 5;

const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_MOVE = 2;
const char CS_PACKET_ATTACK = 3;
const char CS_PACKET_CHAT = 4;
const char CS_PACKET_TELEPORT = 5;
const char CS_PACKET_NPC_RESPONSE = 6;
const char CS_PACKET_NPC_INTERACT = 7;

const char CS_TEST_PACKET = 120;


const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_MOVE = 2;
const char SC_PACKET_PUT_OBJECT = 3;
const char SC_PACKET_REMOVE_OBJECT = 4;
const char SC_PACKET_CHAT = 5;
const char SC_PACKET_LOGIN_FAIL = 6;
const char SC_PACKET_STATUS_CHANGE = 7;
const char SC_PACKET_LOG = 8;
const char SC_PACKET_IMGUPDATE = 9;
const char SC_PACKET_BUFFUPDATE = 10;
const char SC_PACKET_NPC_DIALOG = 11;

#pragma pack (push, 1)
struct cs_packet_login {
	unsigned char size;
	char	type;
	char	name[MAX_NAME_SIZE];
	char	pw[MAX_NAME_SIZE];
};

struct cs_packet_move {
	unsigned char size;
	char	type;
	char	direction;			// 0 : up,  1: down, 2:left, 3:right
	int		move_time;
};

struct cs_packet_attack {
	unsigned char size;
	char	type;
};

struct cs_packet_chat {
	unsigned char size;
	char	type;
	char	message[MAX_CHAT_SIZE];
};

struct cs_packet_teleport {		//�������� ��ֹ��� ���� ���� ��ǥ�� �ڷ���Ʈ ��Ų��.
	//����Ŭ�󿡼� ���� �׽�Ʈ������ ���.
	unsigned char size;
	char	type;
};

struct cs_packet_npc_interact {
	unsigned char size;
	char type;
};

struct cs_packet_npc_response {
	unsigned char size;
	char type;
	char response;
};
struct cs_packet_test {		//�������� ��ֹ��� ���� ���� ��ǥ�� �ڷ���Ʈ ��Ų��.
	//����Ŭ�󿡼� ���� �׽�Ʈ������ ���.
	unsigned char size;
	char	type;
};


//---------
struct sc_packet_login_ok {
	unsigned char size;
	char type;
	int		id;
	short	x, y;
	short	level;
	short	hp, maxhp;
	int		exp;
};

struct sc_packet_move {
	unsigned char size;
	char type;
	int		id;
	short  x, y;
	int		move_time;
};

struct sc_packet_put_object {
	unsigned char size;
	char type;
	int id;
	short x, y;
	char object_type;
	char	name[MAX_NAME_SIZE];
	char	bufftype;
	char	debufftype;
};

struct sc_packet_remove_object {
	unsigned char size;
	char type;
	int id;
};

struct sc_packet_chat {
	unsigned char size;
	char type;
	int id;
	char message[MAX_CHAT_SIZE];
};

struct sc_packet_login_fail {
	unsigned char size;
	char type;
	int reason;		//0 : �ߺ� ID , 1: ����� Full
};

struct sc_packet_status_change {
	unsigned char size;
	char type;
	short	level;
	short	hp, maxhp;
	int		exp;
};

struct sc_packet_log {
	unsigned char size;
	char type;
	char message[MAX_CHAT_SIZE];
};

struct sc_packet_imgupdate {
	unsigned char size;
	char type;
	int id;
	char imgtype;
};

struct sc_packet_buffUpdate {
	unsigned char size;
	char type;
	int id;
	char buff;
	char debuff;
};

struct sc_packet_npc_dialog {
	unsigned char size;
	char type;
	char dlgtype;	//1 normal 2 select
	char msg[200];
	int npc_id;
};
#pragma pack(pop)
