#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <array>
#include <chrono>
#include <mutex>
#include <concurrent_priority_queue.h>
#include "protocol.h"
extern HANDLE g_iocp;
extern SOCKET s_socket;
const int RANGE = 7;
void error_display(int err_no);
void Disconnect(int _id);
int Generate_Id();
void InitNPC();
void InitMAP();
void push_queue(int player_id);
void process_packet(int client_id, unsigned char* p);
bool is_Near(int character1_id, int character2_id);
bool is_Near_By_Range(int character1_id, int character2_id, int Range);
void send_put_object(int player_id, int putCharacter_id,int object_type);
void send_remove_object(int player_id, int removeCharacter_id);
void send_move_packet(int player_id, int moveCharacter_id);
void send_chat_packet(int player_id, int chatCharacter_id, void* msg);
void send_status_change_packet(int player_id);
void send_log_packet(int player_id, void* msg);
void send_login_fail_packet(int player_id, int reason);
void send_login_ok_packet(int player_id);
void send_imgupdate_packet(int player_id, int updateCharacter_id);
void send_buffUpdate_packet(int player_id, int updateCharacter_id);
void send_npc_packet(int player_id,int npc_id);

extern std::array<class Character* ,MAX_USER + MAX_NPC + MAX_CONVNPC> characters;
extern std::vector<int> CSection[20][20];
extern std::mutex section_lock[20][20];
extern bool mMap[2000][2000];	//0이면 길, 1이면 벽 mMap[x][y]의 형태.
extern CRITICAL_SECTION db_cs;
enum COMMAND_IOCP { CMD_ACCEPT,CMD_RECV, CMD_SEND, CMD_NPC_AI,
	CMD_SCRIPTNPC_MOVE , CMD_NPC_RESPAWN,
	CMD_PLAYER_HEAL,CMD_PLAYER_STUNNED,CMD_PLAYER_BUFFREMOVE,
	CMD_AUTO_SAVE};
class WSA_OVER_EX {
	WSAOVERLAPPED _wsaover;
	COMMAND_IOCP _cmd;
	WSABUF _wsabuf;
	unsigned char _buf[BUFSIZE];
public:
	WSA_OVER_EX() = default;
	WSA_OVER_EX(COMMAND_IOCP cmd, char bytes, void* msg);
	~WSA_OVER_EX();

	WSAOVERLAPPED& getWsaOver() { return _wsaover; }
	const COMMAND_IOCP getCmd() const { return _cmd; }
	void setCmd(const COMMAND_IOCP& cmd) { _cmd = cmd; }
	unsigned char* getBuf() { return _buf; }
	WSABUF& getWsaBuf() { return _wsabuf; }

};

struct Timer_Event {

	enum class TIMER_TYPE
	{
		TYPE_NPC_AI,
		TYPE_NPC_RESPAWN,
		TYPE_PLAYER_HEAL,
		TYPE_PLAYER_STUNNED,
		TYPE_PLAYER_BUFF,
		TYPE_AUTO_SAVE,
	};
	int npc_id;
	int player_id;
	std::chrono::system_clock::time_point exec_time;
	TIMER_TYPE type;

	constexpr bool operator < (const Timer_Event& R) const
	{
		return (exec_time > R.exec_time);
	}
};

struct LoginInfo {
	char p_name[20];
	char p_pw[20];
	short p_x;
	short p_y;
	short p_level;
	short p_hp;
	short p_maxhp;
	short p_exp;
};

extern concurrency::concurrent_priority_queue <Timer_Event> timer_queue;