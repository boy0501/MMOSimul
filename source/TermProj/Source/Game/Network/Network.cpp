#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include "Network.h"
#include "../Object/Character/Character.h"
#include "../Object/Character/Player/Player.h"
#include "../Object/Character/Npc/Npc.h"
#include "../Object/Character/Npc/ScriptNpc/ScriptNpc.h"
#include "../Object/Character/Npc/ScriptNpc/AngryMonster/AngryMonster.h"
#include "../Object/Character/Npc/ScriptNpc/PlantMonster/PlantMonster.h"
#include "../Object/Character/Npc/ScriptNpc/NormalMonster/NormalMonster.h"
#include "../Object/Character/Npc/ScriptNpc/BossMonster/BossMonster.h"
#include "../../../Script/cplustolua.h"
#include "../../DB/DB.h"
#pragma comment(lib,"lua54.lib")
using namespace std;

HANDLE g_iocp;
SOCKET s_socket;
bool mMap[2000][2000];

concurrency::concurrent_priority_queue <Timer_Event> timer_queue;
std::array<Character*, MAX_USER + MAX_NPC> characters;
vector<int> CSection[20][20];
std::mutex section_lock[20][20];
CRITICAL_SECTION db_cs;

WSA_OVER_EX::WSA_OVER_EX(COMMAND_IOCP cmd, char bytes, void* msg)
	: _cmd(cmd)
{
	
	ZeroMemory(&_wsaover, sizeof(_wsaover));
	_wsabuf.buf = reinterpret_cast<char*>(_buf);
	_wsabuf.len = bytes;
	memcpy(_buf, msg, bytes);
}

WSA_OVER_EX::~WSA_OVER_EX()
{

}

void error_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	wcout << lpMsgBuf << endl;
	//while (true);
	LocalFree(lpMsgBuf);
}

void Disconnect(int _id)
{
	Player* cl = reinterpret_cast<Player*>(characters[_id]);
	cl->vl.lock();
	unordered_set <int> my_vl = cl->viewlist;
	cl->vl.unlock();
	for (auto& other : my_vl) {
		Player* target = reinterpret_cast<Player*>(characters[other]);
		if (true == target->is_Npc())
		{
			Npc* npc = reinterpret_cast<Npc*>(characters[other]);
			if(npc->target == cl->_id)
				npc->target = INVALID_TARGET;
			continue;
		}
		if (Character::STATE::ST_INGAME != target->_state)
			continue;
		target->vl.lock();
		if (0 != target->viewlist.count(_id)) {
			target->viewlist.erase(_id);
			target->vl.unlock();
			send_remove_object(other, _id);
		}
		else target->vl.unlock();
	}
	strcpy_s(cl->name, "");
	cl->state_lock.lock();
	closesocket(cl->_socket);
	cl->_state = Character::STATE::ST_FREE;
	cl->state_lock.unlock();
}

bool is_Near(int character1_id, int character2_id)
{
	if (RANGE < abs(characters[character1_id]->x - characters[character2_id]->x)) return false;
	if (RANGE < abs(characters[character1_id]->y - characters[character2_id]->y)) return false;
	return true;
}

bool is_Near_By_Range(int character1_id, int character2_id, int Range)
{
	if (Range < abs(characters[character1_id]->x - characters[character2_id]->x)) return false;
	if (Range < abs(characters[character1_id]->y - characters[character2_id]->y)) return false;
	return true;
}

int Generate_Id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		auto user = reinterpret_cast<Player*>(characters[i]);
		user->state_lock.lock();
		if (Character::STATE::ST_FREE == user->_state)
		{
			user->_state = Character::STATE::ST_ACCEPT;
			user->state_lock.unlock();
			return i;
		}
		user->state_lock.unlock();
	}
	cout << "Player is Over the MAX_USER" << endl;
	return -1;
}

void send_login_ok_packet(int player_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_login_ok packet;
	packet.id = player_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_OK;
	packet.x = characters[player_id]->x;
	packet.y = characters[player_id]->y;
	packet.exp = player->exp;
	packet.hp = player->hp;
	packet.maxhp = player->maxhp;
	packet.level = player->level;
	player->sendPacket(&packet, sizeof(packet));
}

void send_move_packet(int player_id, int moveCharacter_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_move packet;
	packet.id = moveCharacter_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_MOVE;
	packet.x = characters[moveCharacter_id]->x;
	packet.y = characters[moveCharacter_id]->y;
	if (characters[moveCharacter_id]->_type == Character::TYPE::PLAYER)
	{
		auto mover = reinterpret_cast<Player*>(characters[moveCharacter_id]);
		packet.move_time = mover->last_move_time;
	}
	player->sendPacket(&packet, sizeof(packet));
}

void send_remove_object(int player_id, int removeCharacter_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_remove_object packet;
	packet.id = removeCharacter_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_REMOVE_OBJECT;
	player->sendPacket(&packet, sizeof(packet));
}

void send_put_object(int player_id, int putCharacter_id,int object_type)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_put_object packet;
	packet.id = putCharacter_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_PUT_OBJECT;
	packet.x = characters[putCharacter_id]->x;
	packet.y = characters[putCharacter_id]->y;
	strcpy_s(packet.name, characters[putCharacter_id]->name);
	packet.object_type = object_type;
	packet.bufftype = -1;
	packet.debufftype = -1;
	if (object_type == OBJECT_PLAYER)
	{
		auto OtherPlayer = reinterpret_cast<Player*>(characters[putCharacter_id]);
		packet.bufftype = OtherPlayer->bufftype;
		packet.debufftype = OtherPlayer->debufftype;
	}
	player->sendPacket(&packet, sizeof(packet));
}

void send_chat_packet(int player_id, int chatCharacter_id, void* msg)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_chat packet;
	packet.id = chatCharacter_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_CHAT;
	strcpy_s(packet.message, 100, (char*)msg);
	player->sendPacket(&packet, sizeof(packet));
}

void send_imgupdate_packet(int player_id, int updateCharacter_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_imgupdate packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_IMGUPDATE;
	packet.id = updateCharacter_id;
	packet.imgtype = characters[updateCharacter_id]->imageType;
	player->sendPacket(&packet, sizeof(packet));

}

void send_log_packet(int player_id, void* msg)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_log packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOG;
	strcpy_s(packet.message, MAX_CHAT_SIZE, (char*)msg);
	player->sendPacket(&packet, sizeof(packet));
}

void send_status_change_packet(int player_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	//SaveStatus(player->name, player->hp, player->maxhp, player->level, player->exp, player->x, player->y);
	sc_packet_status_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_STATUS_CHANGE;
	packet.exp = player->exp;
	packet.hp = player->hp;
	packet.level = player->level;
	packet.maxhp = player->maxhp;
	player->sendPacket(&packet, sizeof(packet));
}

void send_login_fail_packet(int player_id,int reason)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	sc_packet_login_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_FAIL;
	packet.reason = reason;
	player->sendPacket(&packet, sizeof(packet));
}

void send_buffUpdate_packet(int player_id, int updateCharacter_id)
{
	auto player = reinterpret_cast<Player*>(characters[player_id]);
	auto Updater = reinterpret_cast<Player*>(characters[updateCharacter_id]);
	sc_packet_buffUpdate packet;
	packet.size = sizeof(packet);
	packet.id = updateCharacter_id;
	packet.type = SC_PACKET_BUFFUPDATE;
	packet.buff = Updater->bufftype;
	packet.debuff = Updater->debufftype;
	player->sendPacket(&packet, sizeof(packet));
}


void process_packet(int client_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	Character* character = characters[client_id];

	switch (packet_type) {
	case CS_PACKET_LOGIN: {
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
		bool LoginFailFlag = false;
		for (auto c : characters)
		{
			if (c->is_Npc()) break;
			if (strcmp(c->name, packet->name) == 0)
			{
				send_login_fail_packet(client_id, 0);
				Disconnect(client_id);
				LoginFailFlag = true;
				break;
			}
		}

		if (LoginFailFlag == true)
			break;
		strcpy_s(character->name, packet->name);

		LoginInfo p_info{};
		EnterCriticalSection(&db_cs);
		if (-1 == Login(packet->name, p_info))
		{
			LeaveCriticalSection(&db_cs);
			//로그인 실패하면 끊는게아니라, 새롭게 만들어줘야함.
			EnterCriticalSection(&db_cs);
			if (-1 == MakeCharacterAndLogin(packet->name, p_info))
			{
				LeaveCriticalSection(&db_cs);
				//여기서도 실패하면 끊어야지 뭐,,
				Disconnect(client_id);
				LoginFailFlag = true;
			}else{
				LeaveCriticalSection(&db_cs);
			}
		}
		else {

			LeaveCriticalSection(&db_cs);
		}

		if (LoginFailFlag == true)
			break;

		auto player = reinterpret_cast<Player*>(character);
		player->x = p_info.p_x;
		player->y = p_info.p_y;
		player->level = p_info.p_level;
		player->exp = p_info.p_exp;
		player->hp = p_info.p_hp;
		player->maxhp = p_info.p_maxhp;
		//-------------------------------------add
		int sx = player->x / 100;	//sectionX
		int sy = player->y / 100;	//sectionY

		section_lock[sy][sx].lock();
		CSection[sy][sx].push_back(character->_id);
		section_lock[sy][sx].unlock();

		send_login_ok_packet(client_id);

		//5분~10분사이 랜덤하게 auto save
		random_device rd;
		mt19937 rng(rd());
		uniform_int_distribution<int> timing(300, 600);
		Timer_Event AutoSaveinstq;
		AutoSaveinstq.player_id = player->_id;
		AutoSaveinstq.type = Timer_Event::TIMER_TYPE::TYPE_AUTO_SAVE;
		AutoSaveinstq.npc_id = INVALID_TARGET;
		AutoSaveinstq.exec_time = chrono::system_clock::now() + chrono::seconds(timing(rng));
		timer_queue.push(AutoSaveinstq);

		if (player->hp < player->maxhp)	//피가 닳아있다면 hp회복에 넣어줌.
		{
			Timer_Event instq;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_HEAL;
			instq.exec_time = chrono::system_clock::now() + 5000ms;
			instq.player_id = player->_id;
			instq.npc_id = INVALID_TARGET;
			player->is_Healing = true;
			timer_queue.push(instq);
		}
		player->state_lock.lock();
		player->_state = Character::STATE::ST_INGAME;
		player->state_lock.unlock();

		// 새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다
		for (auto other : characters) {
			if (other->is_Npc()) continue;
			if (other->_id == client_id) continue;
			//if npc -> continue  So, under lines other is player
			// npc면 continue했으니 이 아래부터 other는 모두 player이다.
			auto OtherPlayer = reinterpret_cast<Player*>(other);
			OtherPlayer->state_lock.lock();
			if (Character::STATE::ST_INGAME != OtherPlayer->_state) {
				OtherPlayer->state_lock.unlock();
				continue;
			}
			OtherPlayer->state_lock.unlock();

			if (false == is_Near(OtherPlayer->_id, client_id))
				continue;

			OtherPlayer->vl.lock();
			OtherPlayer->viewlist.insert(client_id);
			OtherPlayer->vl.unlock();


			send_put_object(OtherPlayer->_id, client_id, characters[client_id]->imageType);
		}

		// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
		for (auto other : characters) {
			if (other->_id == client_id) continue;
			other->state_lock.lock();
			if (Character::STATE::ST_INGAME != other->_state) {
				other->state_lock.unlock();
				continue;
			}
			other->state_lock.unlock();

			if (false == is_Near(other->_id, client_id))
				continue;

			player->vl.lock();
			player->viewlist.insert(other->_id);
			player->vl.unlock();


			send_put_object(client_id, other->_id, characters[other->_id]->imageType);
		}
		//위에서 만든 vl정보로, vl에 npc가 있다면 queue에 넣어준다.
		push_queue(client_id);
		break;
	}
	case CS_PACKET_MOVE: {
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);

		auto player = reinterpret_cast<Player*>(character);
		player->last_move_time = packet->move_time;
		if (player->debufftype == 1) 
			break;

		int x = character->x;
		int y = character->y;
		int oldsx = x/100;
		int oldsy = y/100;
		switch (packet->direction) {
		case 0:
			if (y > 0)
				if (mMap[x][y - 1] == 0)
					y--;
			break;
		case 1:
			if (y < (WORLD_HEIGHT - 1))
				if (mMap[x][y + 1] == 0)
					y++;
			break;
		case 2:
			if (x > 0)
				if (mMap[x - 1][y] == 0)
					x--;
			break;
		case 3:
			if (x < (WORLD_WIDTH - 1))
				if (mMap[x + 1][y] == 0)
					x++;
			break;
		default:
			cout << "Invalid Move!! Move id :" << client_id << endl;
			exit(-1);
		}
		character->x = x;
		character->y = y;
		int sx = x / 100;	//sectionX
		int sy = y / 100;	//sectionY

		if ((oldsy != sy) || (oldsx != sx))
		{
			section_lock[oldsy][oldsx].lock();
			CSection[oldsy][oldsx].erase(remove(CSection[oldsy][oldsx].begin(), CSection[oldsy][oldsx].end(), character->_id), CSection[oldsy][oldsx].end());
			section_lock[oldsy][oldsx].unlock();

			section_lock[sy][sx].lock();
			CSection[sy][sx].push_back(character->_id);
			section_lock[sy][sx].unlock();
		}
		unordered_set <int> nearlist;
		for (int i = max(0, sx - 1); i < min(20, sx + 1); ++i)
		{
			for (int j = max(0, sy - 1); j < min(20, sy + 1); ++j)
			{
				section_lock[j][i].lock();
				vector<int> Sector{ CSection[j][i] };
				section_lock[j][i].unlock();

				for (auto other : Sector)
				{
					if (Character::STATE::ST_INGAME != characters[other]->_state)
						continue;
					if (false == is_Near(client_id, other))
						continue;
					nearlist.insert(other);
				}				
			}
		}
		

		//for (auto other : Sector[0])
		//{
		//	if (Character::STATE::ST_INGAME != other->_state)
		//		continue;
		//	if (false == is_Near(client_id, other->_id))
		//		continue;
		//	nearlist.insert(other->_id);
		//}

		//for (auto other : characters) {
		//	if (Character::STATE::ST_INGAME != other->_state)
		//		continue;
		//	if (false == is_Near(client_id, other->_id))
		//		continue;
		//	nearlist.insert(other->_id);
		//}

		send_move_packet(character->_id, character->_id);


		player->vl.lock();
		unordered_set <int> my_vl{ player->viewlist };
		player->vl.unlock();

		// player의 시야에 새롭게 들어온 사람들에게 player의 정보를 준다.
		for (auto other : nearlist) {
			if (other == client_id)
				continue;
			if (0 == my_vl.count(other)) {
				player->vl.lock();
				player->viewlist.insert(other);
				player->vl.unlock();


				send_put_object(player->_id, other, characters[other]->imageType);


				if (characters[other]->is_Npc()) continue;

				auto OtherPlayer = reinterpret_cast<Player*>(characters[other]);
				OtherPlayer->vl.lock();
				if (0 == OtherPlayer->viewlist.count(player->_id)) {
					OtherPlayer->viewlist.insert(player->_id);
					OtherPlayer->vl.unlock();
					send_put_object(OtherPlayer->_id, player->_id, player->imageType);
				}
				else {
					OtherPlayer->vl.unlock();
					send_move_packet(OtherPlayer->_id, player->_id);
				}
			}
			// 원래 player의 시야에 있던 사람들에게 player의 정보를 준다.
			else {
				if (characters[other]->is_Npc()) continue;

				auto OtherPlayer = reinterpret_cast<Player*>(characters[other]);

				//나한텐 보이지만 상대한테 안보이는 상태일 수 있음. 
				OtherPlayer->vl.lock();
				if (0 == OtherPlayer->viewlist.count(player->_id)) {
					OtherPlayer->viewlist.insert(player->_id);
					OtherPlayer->vl.unlock();
					send_put_object(OtherPlayer->_id, player->_id, player->imageType);
				}
				else {
					OtherPlayer->vl.unlock();
					send_move_packet(OtherPlayer->_id, player->_id);
				}

			}
		}
		// 시야에서 사라진 플레이어 처리
		for (auto other_id : my_vl) {
			if (0 == nearlist.count(other_id)) {
				player->vl.lock();
				player->viewlist.erase(other_id);
				player->vl.unlock();
				send_remove_object(player->_id, other_id);

				if (characters[other_id]->is_Npc()) {
					Npc* npc = reinterpret_cast<Npc*>(characters[other_id]);
					if (npc->target == client_id)	//npc의 타겟이 나였다면
					{
						npc->target = INVALID_TARGET;
					}
					continue;
				}
				auto OtherPlayer = reinterpret_cast<Player*>(characters[other_id]);

				OtherPlayer->vl.lock();
				if (0 != OtherPlayer->viewlist.count(player->_id)) {
					OtherPlayer->viewlist.erase(player->_id);
					OtherPlayer->vl.unlock();
					send_remove_object(other_id, player->_id);
				}
				else OtherPlayer->vl.unlock();
			}
		}
		push_queue(client_id);
		break;
	}
	case CS_PACKET_ATTACK: {
		cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(p);
		auto player = reinterpret_cast<Player*>(character);

		if (player->attackTime > chrono::system_clock::now())
			break;
		player->attackTime = chrono::system_clock::now() + 1000ms;

		player->vl.lock();
		auto my_vl{ player->viewlist };
		player->vl.unlock();
		for (auto v : my_vl)
		{
			if (false == characters[v]->is_Npc()) continue;
			auto npc = reinterpret_cast<Npc*>(characters[v]);
			if (player->x == npc->x)
			{
				if (player->y - 1 <= npc->y && npc->y <= player->y + 1)
				{
					npc->state_lock.lock();
					if (npc->_state != Character::STATE::ST_INGAME) {
						npc->state_lock.unlock();
						continue; 
					}
					npc->state_lock.unlock();

					int ApplyDamage = player->level * 2;
					if (player->bufftype == 1)
						ApplyDamage *= 1.5;

					npc->hp = max(npc->hp - ApplyDamage, 0);

					char logbuf[MAX_CHAT_SIZE];
					if (player->bufftype == 1)
						sprintf_s(logbuf, MAX_CHAT_SIZE, "플레이어[%s]이/가 몬스터[%s]에게 %d만큼의 피해를 입힘(버프 추가피해 50%%증가)", player->name, npc->name, ApplyDamage);
					else
						sprintf_s(logbuf, MAX_CHAT_SIZE, "플레이어[%s]이/가 몬스터[%s]에게 %d만큼의 피해를 입힘", player->name, npc->name, ApplyDamage);

					send_log_packet(client_id, logbuf);

					npc->lua_lock.lock();
					lua_State* L = npc->L;
					lua_getglobal(L, "event_hit");
					lua_pushnumber(L, player->_id);
					if (0 != lua_pcall(L, 1, 0, 0))
					{
						cout << lua_tostring(L, -1) << endl;
						lua_pop(L, 1);
					}
					npc->target = player->_id;	//맞으면 타겟 설정.
					npc->lua_lock.unlock();

					if (npc->attack_cooltime < chrono::system_clock::now())
					{
						npc->attack_cooltime = chrono::system_clock::now() + 2000ms;
						//다른사람이 동시에 때린다면 몬스터가 두번 공격 안하게 막아줌.
						//한번 때리기 시작하면 타겟이 없어지지않는이상 계속 때리니까 
						//어차피 now + 1000ms로 다른데서 갱신이 될거라 안들어오고.
						// 플레이어한테 한대 맞고 1초뒤에 때리기 전까지 
						// 다른 플레이어한테 한대 맞고 다시 때리면 안됨.
						// Peace든 어그로든 맞으면 타게팅해주고 Chase로 들어가야하는것.
						// 시즈모드는 좀 다르긴 한데 그건 lua 코드적으로 구현되어있음.
						//if (npc->monType == MonsterType::Peace)
						{
							//cout << "때리기 타이머 넣기" << endl;
							switch (npc->monMoveType)
							{
							case MonsterMoveType::MT_Siege: {
								Timer_Event instq;
								auto timing = chrono::system_clock::now() + 1000ms;
								instq.player_id = player->_id;
								instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
								instq.npc_id = npc->_id;
								instq.exec_time = timing;
								timer_queue.push(instq);
								break;
							}
							case MonsterMoveType::MT_Move: {
								Timer_Event instq;
								auto timing = chrono::system_clock::now() + 1000ms;
								instq.player_id = player->_id;
								instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
								instq.npc_id = npc->_id;
								instq.exec_time = timing;
								timer_queue.push(instq);
								break;
							}
							}
						}

					}

				}
			}
			else if (player->y == npc->y)
			{
				if (player->x - 1 <= npc->x && npc->x <= player->x + 1)
				{
					npc->state_lock.lock();
					if (npc->_state != Character::STATE::ST_INGAME) {
						npc->state_lock.unlock();
						continue;
					}
					npc->state_lock.unlock();

					int ApplyDamage = player->level * 2;
					if (player->bufftype == 1)
						ApplyDamage *= 1.5;

					npc->hp = max(npc->hp - ApplyDamage, 0);

					char logbuf[MAX_CHAT_SIZE];
					if (player->bufftype == 1)
						sprintf_s(logbuf, MAX_CHAT_SIZE, "플레이어[%s]이/가 몬스터[%s]에게 %d만큼의 피해를 입힘(버프 추가피해 50%%증가)", player->name, npc->name, ApplyDamage);
					else
						sprintf_s(logbuf, MAX_CHAT_SIZE, "플레이어[%s]이/가 몬스터[%s]에게 %d만큼의 피해를 입힘", player->name, npc->name, ApplyDamage);

					send_log_packet(client_id, logbuf);

					npc->lua_lock.lock();
					lua_State* L = npc->L;
					lua_getglobal(L, "event_hit");
					lua_pushnumber(L, player->_id);
					if (0 != lua_pcall(L, 1, 0, 0))
					{
						cout << lua_tostring(L, -1) << endl;
						lua_pop(L, 1);
					}
					npc->target = player->_id;	//맞으면 타겟 설정.
					npc->lua_lock.unlock();
					if (npc->attack_cooltime < chrono::system_clock::now())
					{
						npc->attack_cooltime = chrono::system_clock::now() + 2000ms;
						//다른사람이 동시에 때린다면 몬스터가 두번 공격 안하게 막아줌.
						//한번 때리기 시작하면 타겟이 없어지지않는이상 계속 때리니까 
						//어차피 now + 1000ms로 다른데서 갱신이 될거라 안들어오고.
						// 플레이어한테 한대 맞고 1초뒤에 때리기 전까지 
						// 다른 플레이어한테 한대 맞고 다시 때리면 안됨.
						// 그리고 이 작업은 Peace 몬스터한테만 해당임. 어그로는 이미 타게팅돼서 쫒아오는중
						if (npc->monType == MonsterType::Peace)
						{
							//cout << "때리기 타이머 넣기" << endl;
							switch (npc->monMoveType)
							{
							case MonsterMoveType::MT_Siege: {
								Timer_Event instq;
								auto timing = chrono::system_clock::now() + 1000ms;
								instq.player_id = player->_id;
								instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
								instq.npc_id = npc->_id;
								instq.exec_time = timing;
								timer_queue.push(instq);
								break;
							}
							case MonsterMoveType::MT_Move: {
								Timer_Event instq;
								auto timing = chrono::system_clock::now() + 1000ms;
								instq.player_id = player->_id;
								instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
								instq.npc_id = npc->_id;
								instq.exec_time = timing;
								timer_queue.push(instq);
								break;
							}
							}
						}

					}
				}
			}
			//cout << "npc피"<<npc->hp << endl;

		}

		break;
	}
	case CS_PACKET_TELEPORT: {
		cs_packet_teleport* packet = reinterpret_cast<cs_packet_teleport*>(p);
		auto player = reinterpret_cast<Player*>(character);
		
		while (1) {
			int randx = rand() % 2000;
			int randy = rand() % 2000;
			if (mMap[randx][randy] == 1) continue;

			//---add
			int oldsx = player->x /100;
			int oldsy = player->y / 100;
			player->x = randx;
			player->y = randy;

			int sx = player->x / 100;	//sectionX
			int sy = player->y / 100;	//sectionY

			if ((oldsy != sy) || (oldsx != sx))
			{
				section_lock[oldsy][oldsx].lock();
				CSection[oldsy][oldsx].erase(remove(CSection[oldsy][oldsx].begin(), CSection[oldsy][oldsx].end(), player->_id), CSection[oldsy][oldsx].end());
				section_lock[oldsy][oldsx].unlock();

				section_lock[sy][sx].lock();
				CSection[sy][sx].push_back(character->_id);
				section_lock[sy][sx].unlock();
			}

			//기존에 있던 플레이어들에게 나간다고 알려준다.
			player->vl.lock();
			unordered_set<int> LeaveList = player->viewlist;
			player->viewlist.clear();
			player->vl.unlock();

			for (auto player_id : LeaveList)
			{
				if (characters[player_id]->is_Npc())
				{
					Npc* npc = reinterpret_cast<Npc*>(characters[player_id]);
					if (npc->target == client_id)
					{
						npc->target = INVALID_TARGET;	//타게팅대상이었다면 풀어준다.
					}
				}
				else {
					send_remove_object(player_id, client_id);
				}
			}


			// 새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다
			for (auto other : characters) {
				if (other->is_Npc()) break;
				if (other->_id == client_id) continue;
				//if npc -> break  So, under lines other is player
				// npc면 break했으니 이 아래부터 other는 모두 player이다.
				auto OtherPlayer = reinterpret_cast<Player*>(other);
				OtherPlayer->state_lock.lock();
				if (Character::STATE::ST_INGAME != OtherPlayer->_state) {
					OtherPlayer->state_lock.unlock();
					continue;
				}
				OtherPlayer->state_lock.unlock();

				if (false == is_Near(OtherPlayer->_id, client_id))
					continue;

				OtherPlayer->vl.lock();
				OtherPlayer->viewlist.insert(client_id);
				OtherPlayer->vl.unlock();


				send_put_object(OtherPlayer->_id, client_id, characters[client_id]->imageType);
			}

			// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
			for (auto other : characters) {
				if (other->_id == client_id) continue;
				other->state_lock.lock();
				if (Character::STATE::ST_INGAME != other->_state) {
					other->state_lock.unlock();
					continue;
				}
				other->state_lock.unlock();

				if (false == is_Near(other->_id, client_id))
					continue;

				player->vl.lock();
				player->viewlist.insert(other->_id);
				player->vl.unlock();


				send_put_object(client_id, other->_id, characters[other->_id]->imageType);
			}
			//위에서 만든 vl정보로, vl에 npc가 있다면 queue에 넣어준다.
			push_queue(client_id);

			break;
		}

		break;
	}
	}
}



void InitNPC()
{
	for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
		Npc* npc;
		if (i < NPC_PLANT)
		{
			npc = new PlantMonster("monster.lua", i);
		}
		else if (i < NPC_SECTOR_MONSTER1)
		{
			npc = new NormalMonster("Sector1Stone.lua", i);
		}
		else if (i < NPC_SECTOR_MONSTER2)
		{
			npc = new NormalMonster("Sector2Stone.lua", i);
		}
		else if (i < NPC_SECTOR_MONSTER3)
		{
			npc = new NormalMonster("Sector3Stone.lua", i);
		}
		else if(i < NPC_ID_END){
			npc = new AngryMonster("monster4.lua", i);
		
		}
		else {
			npc = new BossMonster("bossmonster.lua", i);
		}
		lua_State* L = npc->L;
		
		lua_getglobal(L, "set_SpawnXY");
		lua_pushnumber(L, npc->x);
		lua_pushnumber(L, npc->y);
		if (0 != lua_pcall(L, 2, 0, 0))
		{
			cout << lua_tostring(L, -1) << endl;
			lua_pop(L, 1);
		}

		npc->_id = i;
		//npc->maxhp = 20;
		npc->hp = npc->maxhp;
		npc->_state = Character::STATE::ST_INGAME;

		characters[i] = npc;
		CSection[(int)(npc->y / 100)][(int)(npc->x / 100)].push_back(i);
	}
	cout << "NPC Setting is Done" << endl;
}


void InitMAP()
{

	ifstream in;
	int x, y, type;
	in.open("Asset/Map/map1.txt", ios::in);
	if (in.is_open())
	{
		for (int i = 0; ; ++i)
		{
			if (in.eof())
			{
				cout << "Map Loading is Done" << endl;
				break;
			}
			in >> x >> y >> type;
			if (type == 1)
				mMap[x][y] = type;
		}
	}
	else {
		cout << " map doesn't exits!" << endl;
	}
}




void push_queue(int player_id)
{
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	player->vl.lock();
	auto vl = player->viewlist;
	player->vl.unlock();
	auto timing = chrono::system_clock::now() + 1000ms;
	for (auto& other : vl)
	{
		if (characters[other]->is_Npc() == false) continue;
		Npc* npc = reinterpret_cast<Npc*>(characters[other]);
		if (npc->isMoving == true) continue;
		npc->isMoving = true;
		//이 구문은 Peace 몬스터 일때 실행. Npc_Type으로 정해주는것.
		switch (npc->monType)
		{
		case MonsterType::Peace: 
		{
			//랜덤무브는 타겟이없다.
			Timer_Event instq;
			instq.exec_time = chrono::system_clock::now() + 1000ms;
			instq.player_id = -1;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
			instq.npc_id = other;
			if (other == 0)
			{
				cout << "여기오면안된다" << endl;
			}
			timer_queue.push(instq);
			break;
		}
		case MonsterType::Agro:
		{
			AngryMonster* monster = reinterpret_cast<AngryMonster*>(npc);
			if (is_Near_By_Range(monster->_id, player_id, monster->AgroRange))
			{
				monster->target = player_id;
				Timer_Event instq;
				instq.exec_time = chrono::system_clock::now() + 1000ms;
				instq.player_id = player_id;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
				instq.npc_id = other;
				timer_queue.push(instq);
			}
			else {
				Timer_Event instq;
				monster->target = INVALID_TARGET;
				instq.exec_time = chrono::system_clock::now() + 1000ms;
				instq.player_id = INVALID_TARGET;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
				instq.npc_id = other;
				timer_queue.push(instq);
			}
		}
			break;
		}


	}
}