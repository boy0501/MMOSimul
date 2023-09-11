#pragma once
#include "../Character.h"
class Player : public Character
{
public:
	Player();
	virtual ~Player();
	std::unordered_set   <int>  viewlist;
	std::mutex vl;
	std::atomic_bool is_Healing;
	std::chrono::system_clock::time_point attackTime;
	char bufftype;		//0이면 기본, 1이면 공격력강화
	char debufftype;	//이면 기본, 1이면 스턴
	WSA_OVER_EX wsa_ex_recv;
	SOCKET  _socket;
	int		_prev_size;
	int		last_move_time;
	char moveSaveCnt;
	
	int mExp;
	void recvPacket();
	void sendPacket(void* packet, int bytes);
	void GainExp(int exp);


	//게터
	int GetInteractNPC() const { return mInteractNPC; }


	//세터
	void SetInteractNPC(int InteractNPC) { mInteractNPC = InteractNPC; }

private:
	void LevelUp(int remainExp);

protected:
	int mInteractNPC;	//현재 대화중인 NPC code -1 = 대화하고 있지 않은 상태

};

