#include <iostream>
#include <random>
#include "../../Network.h"
#include "../../../../DB/DB.h"
#include "../../../Object/Character/Player/Player.h"
#include "../../../Object/Character/Npc/ScriptNpc/ScriptNpc.h"
#include "../../../Object/Character/Npc/NormalNpc/NormalNpc.h"

using namespace std;

void WorkerThread()
{

	for (;;) {
		DWORD bytes;
		LONG64 iocp_key;
		WSAOVERLAPPED* overlapped;
		BOOL ret = GetQueuedCompletionStatus(g_iocp, &bytes, (PULONG_PTR)&iocp_key, &overlapped, INFINITE);
		int client_id = static_cast<int>(iocp_key);
		WSA_OVER_EX* wsa_ex = reinterpret_cast<WSA_OVER_EX*>(overlapped);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			//cout << "GQCS Error : \n";
			error_display(err_no);
			//cout << endl;
			if (wsa_ex->getCmd() == CMD_RECV)
			{
				//플레이어 정상 접속 종료 요청시에만 save / 서버에서 강제 disconn 할때는 save하지 않음. 
				Player* player = reinterpret_cast<Player*>(characters[client_id]);
				EnterCriticalSection(&db_cs);
				SaveStatus(player->name, player->hp, player->maxhp, player->level, player->exp, player->x, player->y);
				LeaveCriticalSection(&db_cs);
				cout << "GQCS player name :" << player->name << endl;
			}
			Disconnect(client_id);
			if (wsa_ex->getCmd() == CMD_SEND)
				delete wsa_ex;
			continue;
		}

		switch (wsa_ex->getCmd()) {
		case CMD_RECV: {
			if (bytes == 0) {
				Disconnect(client_id);
				break;
			}
			Player* player = reinterpret_cast<Player*>(characters[client_id]);
			int To_Process_Bytes = bytes + player->_prev_size;
			unsigned char* packets = wsa_ex->getBuf();

			while (To_Process_Bytes >= packets[0]) {
				process_packet(client_id, packets);
				To_Process_Bytes -= packets[0];
				packets += packets[0];
				if (To_Process_Bytes <= 0)break;
			}

			//패킷처리 다 한다음 비워주어야함. 남은게 있다면 아래서 채워줄 것.
			ZeroMemory(&player->_prev_size, sizeof(player->_prev_size));
			ZeroMemory(wsa_ex->getBuf(), sizeof(wsa_ex->getBuf()));

			if (To_Process_Bytes > 0) {

				player->_prev_size = To_Process_Bytes;
				memcpy(wsa_ex->getBuf(), packets, player->_prev_size);
			}
			player->recvPacket();
			break;
		}
		case CMD_SEND: {
			//if (num_byte != wsa_ex->_wsa_buf.len) {
			//	Disconnect(client_id);
			//}
			delete wsa_ex;
			break;
		}
		case CMD_ACCEPT: {
			//cout << "Accept.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(wsa_ex->getBuf()));
			int new_id = Generate_Id();
			if (-1 == new_id) {
				cout << "Maxmum user overflow. Accept aborted.\n";

				sc_packet_login_fail packet;
				packet.size = sizeof(packet);
				packet.type = SC_PACKET_LOGIN_FAIL;
				packet.reason = 1;
				WSA_OVER_EX* wsa_ex = new WSA_OVER_EX(CMD_SEND, packet.size, &packet);
				int ret = WSASend(c_socket, &wsa_ex->getWsaBuf(), 1, 0, 0, &wsa_ex->getWsaOver(), NULL);
				if (SOCKET_ERROR == ret) {
					int err = WSAGetLastError();
					if (ERROR_IO_PENDING != err)
						error_display(err);
				}
				closesocket(c_socket);

			}
			else {
				Player* player = reinterpret_cast<Player*>(characters[new_id]);
				player->_id = new_id;
				player->_prev_size = 0;
				player->wsa_ex_recv.getWsaBuf().buf = reinterpret_cast<char*>(player->wsa_ex_recv.getBuf());
				player->wsa_ex_recv.getWsaBuf().len = BUFSIZE;
				player->wsa_ex_recv.setCmd(CMD_RECV);
				ZeroMemory(&player->wsa_ex_recv.getWsaOver(), sizeof(player->wsa_ex_recv.getWsaOver()));
				player->_socket = c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_iocp, new_id, 0);
				player->recvPacket();
			}

			ZeroMemory(&wsa_ex->getWsaOver(), sizeof(wsa_ex->getWsaOver()));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			*(reinterpret_cast<SOCKET*>(wsa_ex->getBuf())) = c_socket;
			AcceptEx(s_socket, c_socket, wsa_ex->getBuf() + 8, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &wsa_ex->getWsaOver());
			break;
		}
		case CMD_NPC_AI: {
			Npc* npc = reinterpret_cast<Npc*>(characters[client_id]);
			//현재 모든 npc들이 다 스크립트 형식에 묶여서 이렇게 해줌. 
			// 그게 아니라면? exp_over에 type을 넣고 스크립트에 묶여야할지 그냥 npc_move인지 골라야겠지.
			npc->lua_lock.lock();

			int player_id = 0;
			memcpy(&player_id, wsa_ex->getBuf(), sizeof(int));
			//cout << "몬스터 타겟 :" << player_id << endl;
			lua_State* L = npc->L;
			lua_getglobal(L, "event_timer_ai");
			lua_pushnumber(L, player_id);
			if (0 != lua_pcall(L, 1, 0, 0))
			{
				cout << lua_tostring(L, -1) << endl;
				lua_pop(L, 1);
			}
			npc->lua_lock.unlock();
			delete wsa_ex;
			//
			break;
		}
		case CMD_NPC_RESPAWN: {
			Npc* npc = reinterpret_cast<Npc*>(characters[client_id]);
			npc->hp = npc->maxhp;
			if (npc->imageType == OBJECT_VERYANGRYMONSTER)
				npc->imageType = OBJECT_ANGRYMONSTER;
			npc->lua_lock.lock();
			lua_State* L = npc->L;
			lua_getglobal(L, "set_is_active");
			lua_pushnumber(L, true);
			if (0 != lua_pcall(L, 1, 0, 0))
			{
				cout << lua_tostring(L, -1) << endl;
				lua_pop(L, 1);
			}

			lua_getglobal(L, "get_SpawnXY");
			if (0 != lua_pcall(L, 0, 2, 0))
			{
				cout << lua_tostring(L, -1) << endl;
				lua_pop(L, 1);
			}
			npc->y = lua_tointeger(L, -1);
			npc->x = lua_tointeger(L, -2);
			lua_pop(L, 2);

			npc->lua_lock.unlock();

			npc->state_lock.lock();
			npc->_state = Character::STATE::ST_INGAME;
			npc->state_lock.unlock();

			unordered_set <int> nearlist;
			for (auto other : characters)
			{
				if (other->is_Npc()) break;
				auto OtherPlayer = reinterpret_cast<Player*>(other);
				OtherPlayer->state_lock.lock();
				if (Character::STATE::ST_INGAME != OtherPlayer->_state) {
					OtherPlayer->state_lock.unlock();
					continue;
				}
				OtherPlayer->state_lock.unlock();

				if (false == is_Near(OtherPlayer->_id, client_id))
					continue;

				nearlist.insert(other->_id);
				OtherPlayer->vl.lock();
				OtherPlayer->viewlist.insert(client_id);
				OtherPlayer->vl.unlock();

				send_put_object(OtherPlayer->_id, client_id, npc->imageType);
			}

			if (nearlist.size() > 0)
			{
				switch (npc->monType)
				{
				case MonsterType::Peace:
				{
					//cout << "부활 후 움직임" << endl;
					//랜덤무브는 타겟이없다.
					npc->target = INVALID_TARGET;
					Timer_Event instq;
					instq.exec_time = chrono::system_clock::now() + 1000ms;
					instq.player_id = INVALID_TARGET;
					instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
					instq.npc_id = client_id;
					timer_queue.push(instq);
				}
				break;
				case MonsterType::Agro:
				{

					npc->target = *nearlist.begin();
					Timer_Event instq;
					instq.exec_time = chrono::system_clock::now() + 1000ms;
					instq.player_id = npc->target;
					instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
					instq.npc_id = client_id;
					timer_queue.push(instq);
				}
				break;
				}
			}
			delete wsa_ex;

			break;
		}
		case CMD_PLAYER_HEAL: {
			Player* player = reinterpret_cast<Player*>(characters[client_id]);
			if (player->_state != Character::STATE::ST_INGAME) break;
			player->hp = min(player->maxhp / 10 + player->hp, player->maxhp);
			send_status_change_packet(client_id);
			if (player->hp < player->maxhp)	//피가 닳아있다면 hp회복에 넣어줌.
			{
				Timer_Event instq;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_HEAL;
				instq.exec_time = chrono::system_clock::now() + 5000ms;
				instq.player_id = player->_id;
				instq.npc_id = INVALID_TARGET;
				timer_queue.push(instq);
			}
			else {
				player->is_Healing = false;
			}
			delete wsa_ex;

			break;
		}
		case CMD_PLAYER_STUNNED: {
			Player* player = reinterpret_cast<Player*>(characters[client_id]);
			if (player->_state != Character::STATE::ST_INGAME) break;
			player->debufftype = 0;

			//플레이어 상태 not stunned인거 보내줘야한다. 
			player->vl.lock();
			unordered_set<int> plist = player->viewlist;
			player->vl.unlock();
			send_buffUpdate_packet(client_id, client_id);
			for (auto& p_id : plist)
			{
				if (characters[p_id]->is_Npc()) continue;
				send_buffUpdate_packet(p_id, client_id);
			}
			delete wsa_ex;
			break;
		}
		case CMD_PLAYER_BUFFREMOVE: {
			Player* player = reinterpret_cast<Player*>(characters[client_id]);
			if (player->_state != Character::STATE::ST_INGAME) break;
			player->bufftype = 0;
			//플레이어 상태 보내줘야한다.

			player->vl.lock();
			unordered_set<int> plist = player->viewlist;
			player->vl.unlock();
			send_buffUpdate_packet(client_id, client_id);
			for (auto& p_id : plist)
			{
				if (characters[p_id]->is_Npc()) continue;
				send_buffUpdate_packet(p_id, client_id);
			}
			delete wsa_ex;

			break;
		}
		case CMD_AUTO_SAVE: {

			Player* player = reinterpret_cast<Player*>(characters[client_id]);
			if (player->_state != Character::STATE::ST_INGAME) break;
			if (player->_type != Character::TYPE::PLAYER)
			{
				cout << "UnExpected Auto Save Player Type!!" << endl;
				delete wsa_ex;
				break;
			}
			SaveStatus(player->name, player->hp, player->maxhp, player->level, player->exp, player->x, player->y);
			random_device rd;
			mt19937 rng(rd());
			uniform_int_distribution<int> timing(300, 600);
			Timer_Event instq;
			instq.player_id = player->_id;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_AUTO_SAVE;
			instq.npc_id = -1;
			instq.exec_time = chrono::system_clock::now() + chrono::seconds(timing(rng));
			delete wsa_ex;
			break;
		}
		}
	}
}
