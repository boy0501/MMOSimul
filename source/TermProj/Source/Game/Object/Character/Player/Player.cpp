#include "Player.h"
#include "../../../../DB/DB.h"
#include <iostream>

using namespace std;
Player::Player()
	:Character(TYPE::PLAYER)
	, _prev_size(0)
	, is_Healing(false)
	, attackTime(std::chrono::system_clock::now())
	, bufftype(0)
	, debufftype(0)
	, moveSaveCnt(0)
	, mInteractNPC(-1)
{
	hp = 30;
}

Player::~Player()
{
	closesocket(_socket);
}



void Player::recvPacket()
{
	ZeroMemory(&wsa_ex_recv.getWsaOver(), sizeof(wsa_ex_recv.getWsaOver()));
	wsa_ex_recv.getWsaBuf().buf = reinterpret_cast<char*>(wsa_ex_recv.getBuf() + _prev_size);
	wsa_ex_recv.getWsaBuf().len = BUFSIZE - _prev_size;

	DWORD flags = 0;
	int ret = WSARecv(_socket, &wsa_ex_recv.getWsaBuf(), 1, 0, &flags, &wsa_ex_recv.getWsaOver(), NULL);
	if (SOCKET_ERROR == ret) {
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{

			//cout << "플레이어 리시브 save" << "player name :" << name << endl;

			error_display(err);
		}
	}
}

void Player::sendPacket(void* packet, int bytes)
{
	WSA_OVER_EX* wsa_ex = new WSA_OVER_EX(CMD_SEND, bytes, packet);
	int ret = WSASend(_socket, &wsa_ex->getWsaBuf(), 1, 0, 0, &wsa_ex->getWsaOver(), NULL);
	if (SOCKET_ERROR == ret) {
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			//cout << "플레이어 샌드 save" << "player name :" << name << endl;

			error_display(err);
		}
	}
}

void Player::GainExp(int exp)
{
	mExp += exp;

	int playerMaxExp = pow(2, level) * 100;
	if (mExp >= playerMaxExp)
		LevelUp(mExp);

	send_status_change_packet(_id);
}

void Player::LevelUp(int remainExp)
{
	while (true)
	{
		int playerMaxExp = pow(2, level) * 100;
		level += 1;
		maxhp = maxhp * 1.3 + 10;
		hp = maxhp;
		remainExp -= playerMaxExp;
		mExp = remainExp;
		char logMsg[MAX_CHAT_SIZE];
		sprintf_s(logMsg, "Level Up!");
		send_log_packet(_id, logMsg);
		//levelup!
		playerMaxExp = pow(2, level) * 100;
		if (remainExp <= playerMaxExp)
			break;
	}
}

