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
	char bufftype;		//0�̸� �⺻, 1�̸� ���ݷ°�ȭ
	char debufftype;	//�̸� �⺻, 1�̸� ����
	WSA_OVER_EX wsa_ex_recv;
	SOCKET  _socket;
	int		_prev_size;
	int		last_move_time;
	char moveSaveCnt;
	
	int mExp;
	void recvPacket();
	void sendPacket(void* packet, int bytes);
	void GainExp(int exp);


	//����
	int GetInteractNPC() const { return mInteractNPC; }


	//����
	void SetInteractNPC(int InteractNPC) { mInteractNPC = InteractNPC; }

private:
	void LevelUp(int remainExp);

protected:
	int mInteractNPC;	//���� ��ȭ���� NPC code -1 = ��ȭ�ϰ� ���� ���� ����

};

