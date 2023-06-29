#include <iostream>
#include <chrono>
#include "cplustolua.h"
#include "../Source/Algorithm/AStar.h"
#include "../Source/DB/DB.h"
#include "../Source/Game/Network/Network.h"
#include "../Source/Game/Object/Character/Character.h"
#include "../Source/Game/Object/Character/Player/Player.h"
#include "../Source/Game/Object/Character/Npc/ScriptNpc/ScriptNpc.h"
#include "../Source/Game/Object/Character/Npc/ScriptNpc/Monster/AngryMonster/AngryMonster.h"
using namespace std;

void do_npc_move(int npc_id,int spawnX,int spawnY,int movelimit);



int CPP_AutoMoveNPC(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -4);
	int spawnX = (int)lua_tointeger(L, -3);
	int spawnY = (int)lua_tointeger(L, -2);
	int movelimit = (int)lua_tointeger(L, -1);
	lua_pop(L, 5);
	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);

	//Chase 할 대상이 없다면 Random Move한다.
	// 	   근데 Chase할 대상이 있을수도있음 (autoMove하려는데 target이 생겨버린경우)
	// 	   그러면 Chase를 돌리고 여긴 돌아가서는 안됨
	// 	   Chase는 때렸을때 그때 돌아가도록 해줌. Attack 추가하고,,
	//     그렇다는 말은. 때릴때 peace뿐만이 아니라 어그로도 때릴때 들어가도록함. 
	//
	if (npc->GetTarget() == INVALID_TARGET)
		do_npc_move(npc_id, spawnX, spawnY, movelimit);
	
	return 0;
}

int CPP_SendMessage(lua_State* L)
{
	//대상에게 메시지 보냄.
	int npc_id = (int)lua_tointeger(L, -3);
	int player_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);
	send_chat_packet(player_id, npc_id, mess);
	return 0;
}

int CPP_SendMessageMySight(lua_State* L)
{
	//Sight에 보이는 대상에게 메시지 보냄. update가 필요하다면 update도 해줌.
	int npc_id = (int)lua_tointeger(L, -3);
	char* mess = (char*)lua_tostring(L, -2);
	bool Needupdate= (bool)lua_toboolean(L, -1);
	lua_pop(L, 4);
	
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

		if (false == is_Near(OtherPlayer->_id, npc_id))
			continue;

		send_chat_packet(OtherPlayer->_id, npc_id, mess);
		if (Needupdate)
		{
			if (characters[npc_id]->imageType == OBJECT_ANGRYMONSTER)
				characters[npc_id]->imageType = OBJECT_VERYANGRYMONSTER;

			send_imgupdate_packet(OtherPlayer->_id, npc_id);
		}
	}
	return 0;
}

int CPP_get_x(lua_State* L)
{
	int player_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = characters[player_id]->x;
	lua_pushnumber(L, x);
	return 1;
}

int CPP_get_y(lua_State* L)
{
	int player_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = characters[player_id]->y;
	lua_pushnumber(L, y);
	return 1;
}

int CPP_get_hp(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int hp = characters[npc_id]->hp;
	lua_pushnumber(L, hp);
	return 1;
}

int CPP_get_MaxHp(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int MaxHp = characters[npc_id]->maxhp;
	lua_pushnumber(L, MaxHp);
	return 1;
}

int CPP_MonsterDie(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -2);
	int player_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 3);
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);
	cout << "[log] monster[" << npc_id << "] die!" << endl;

	int playerMaxExp = pow(2, player->level) * 100;
	int exp = npc->RewardEXP();
	player->exp += exp;
	char buf[MAX_CHAT_SIZE];
	sprintf_s(buf, "몬스터 %s를 무찔러서 %d의 경험치 획득!", npc->name, exp);

	send_log_packet(player_id, buf);
	if (player->exp >= playerMaxExp)
	{
		player->level += 1;
		player->maxhp = player->maxhp * 1.3 + 10;
		player->hp = player->maxhp;
		player->exp = 0;
		char logMsg[MAX_CHAT_SIZE];
		sprintf_s(logMsg, "Level Up!");
		send_log_packet(player_id, logMsg);
		//levelup!
	}
	send_status_change_packet(player_id);
	unordered_set <int> nearlist;
	for (auto c : characters)
	{
		if (c->is_Npc()) break;
		if (Character::STATE::ST_INGAME != c->_state)
			continue;
		if (is_Near(c->_id, npc_id))
			nearlist.insert(c->_id);		
	}
	for (auto& other : nearlist) {
		Player* target = reinterpret_cast<Player*>(characters[other]);
		if (Character::STATE::ST_INGAME != target->_state)
			continue;
		target->vl.lock();
		if (0 != target->viewlist.count(npc_id)) {
			target->viewlist.erase(npc_id);
			target->vl.unlock();
			send_remove_object(other, npc_id);
		}
		else target->vl.unlock();
	}

	//몬스터가 죽으면 active 꺼줌.
	npc->state_lock.lock();
	npc->_state = Character::STATE::ST_FREE;
	npc->state_lock.unlock();
	lua_getglobal(L, "set_is_active");
	lua_pushboolean(L, false);
	if (0 != lua_pcall(L, 1, 0, 0))
	{
		cout << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}

	//30초 뒤 부활.
	auto timing = chrono::system_clock::now() + 30000ms;
	Timer_Event instq;
	instq.player_id = INVALID_TARGET;
	instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_RESPAWN;
	instq.npc_id = npc_id;
	instq.exec_time = timing;
	timer_queue.push(instq);

	return 0;
}


int CPP_MonsterAttack(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -4);
	int player_id = (int)lua_tointeger(L, -3);
	int damage = (int)lua_tointeger(L, -2);
	int npc_Range = (int)lua_tointeger(L, -1);
	lua_pop(L, 5);

	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);


	if (is_Near_By_Range(npc_id, player_id, npc_Range))
	{
		characters[player_id]->hp = max(characters[player_id]->hp - damage, 0);
		if (characters[player_id]->hp <= 0)
		{
			npc->SetTarget(INVALID_TARGET);
			npc->isMoving = false;
			char logbuf[MAX_CHAT_SIZE];
			sprintf_s(logbuf, MAX_CHAT_SIZE, "플레이어 사망");
			send_log_packet(player_id, logbuf);
			auto player = reinterpret_cast<Player*>(characters[player_id]);


			//------------------- add
			int oldsx = player->x / 100;
			int oldsy = player->y / 100;
			player->x = 10;
			player->y = 10;

			int sx = player->x / 100;	//sectionX
			int sy = player->y / 100;	//sectionY

			if ((oldsy != sy) || (oldsx != sx))
			{
				section_lock[oldsy][oldsx].lock();
				CSection[oldsy][oldsx].erase(remove(CSection[oldsy][oldsx].begin(), CSection[oldsy][oldsx].end(), npc_id), CSection[oldsy][oldsx].end());
				section_lock[oldsy][oldsx].unlock();

				section_lock[sy][sx].lock();
				CSection[sy][sx].push_back(player->_id);
				section_lock[sy][sx].unlock();
			}
			//------------------------




			player->hp = player->maxhp;
			player->exp /= 2;
			//
			send_move_packet(player_id, player_id);



			//기존에 있던 플레이어들에게 나간다고 알려준다.
			player->vl.lock();
			unordered_set<int> LeaveList = player->viewlist;
			player->viewlist.clear();
			player->vl.unlock();

			for (auto p_id : LeaveList)
			{
				if (characters[p_id]->is_Npc())
				{
					Monster* npc = reinterpret_cast<Monster*>(characters[p_id]);
					if (npc->GetTarget() == player_id)
					{
						npc->SetTarget(INVALID_TARGET);	//NPC 타게팅대상이었다면 풀어준다.
					}
				}
				else {
					send_remove_object(p_id, player_id);//플레이어라면 remove해준다.
				}
			}


			// 새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다
			for (auto other : characters) {
				if (other->is_Npc()) break;
				if (other->_id == player_id) continue;
				//if npc -> break  So, under lines other is player
				// npc면 break했으니 이 아래부터 other는 모두 player이다.
				auto OtherPlayer = reinterpret_cast<Player*>(other);
				OtherPlayer->state_lock.lock();
				if (Character::STATE::ST_INGAME != OtherPlayer->_state) {
					OtherPlayer->state_lock.unlock();
					continue;
				}
				OtherPlayer->state_lock.unlock();

				if (false == is_Near(OtherPlayer->_id, player_id))
					continue;

				OtherPlayer->vl.lock();
				OtherPlayer->viewlist.insert(player_id);
				OtherPlayer->vl.unlock();


				send_put_object(OtherPlayer->_id, player_id, characters[player_id]->imageType);
			}

			// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
			for (auto other : characters) {
				if (other->_id == player_id) continue;
				other->state_lock.lock();
				if (Character::STATE::ST_INGAME != other->_state) {
					other->state_lock.unlock();
					continue;
				}
				other->state_lock.unlock();

				if (false == is_Near(other->_id, player_id))
					continue;

				player->vl.lock();
				player->viewlist.insert(other->_id);
				player->vl.unlock();


				send_put_object(player_id, other->_id, characters[other->_id]->imageType);
			}
			//위에서 만든 vl정보로, vl에 npc가 있다면 queue에 넣어준다.
			//이건 플레이어가 살아나면 그 주변에껄 부활시켜주는 것 임.
			push_queue(player_id);

			//플레이어 죽었으니 다시 몬스터는 랜덤무브할거임.
			Timer_Event instq;
			instq.exec_time = chrono::system_clock::now() + 1000ms;
			instq.player_id = INVALID_TARGET;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
			instq.npc_id = npc_id;
			timer_queue.push(instq);
		}
		else {
			char logbuf[MAX_CHAT_SIZE];
			sprintf_s(logbuf, MAX_CHAT_SIZE, "몬스터[%s]이/가 플레이어에게 %d만큼의 피해를 입힘", npc->name, damage);
			send_log_packet(player_id, logbuf);
			//플레이어가 안죽었으면 계속 공격
			//고정형이든, 움직이던간에 한번 때렸으면 다시 때려야함.
			//cout << "타이머넣어주는곳 3" << endl;
			Monster* Mnpc = reinterpret_cast<Monster*>(npc);
			Mnpc->ReloadAttack();

			auto player = reinterpret_cast<Player*>(characters[player_id]);
			if (player->is_Healing == false)
			{
				Timer_Event instq;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_HEAL;
				instq.exec_time = chrono::system_clock::now() + 5000ms;
				instq.player_id = player->_id;
				instq.npc_id = INVALID_TARGET;
				player->is_Healing = true;
				timer_queue.push(instq);
			}
		}
		//cout << "플레이어 hp: " << characters[player_id]->hp << endl;
		send_status_change_packet(player_id);
	}
	else {
	//몬스터 공격 실패! 이동형은 딱 붙어야 공격해서 문제가 없지만
	//시즈모드는 멀리있어서 실패하는 경우도 생김. 
	// 타겟 자체가 사라지는 경우도 있음 (플레이어가 죽어서) 그럼 넣을 필요 없음.
	//하지만 어글 범위 안에 있다면~ Ai는 계속 돌아감. 
	//어글범위안에도 없다면 ? 진짜 때릴 사람이 없는것. 타겟 없애줌.
	Monster* Mnpc = reinterpret_cast<Monster*>(npc);
	if (Mnpc->GetMonMoveType() == MonsterMoveType::MT_Siege)
	{
		Mnpc->DeleteTarget();
	}
	}


	return 0;
}


int CPP_ChaseTarget(lua_State* L)
{
	//cout << "체이스하나" << endl;
	int npc_id = (int)lua_tointeger(L, -5);
	int player_id = (int)lua_tointeger(L, -4);
	int spawnX = (int)lua_tointeger(L, -3);
	int spawnY = (int)lua_tointeger(L, -2);
	int movelimit = (int)lua_tointeger(L, -1);
	lua_pop(L, 6);

	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	// A*들어가야할 곳 
	int normdx = 0;
	int normdy = 0;
	AStarAlgo(npc->x, npc->y, player->x, player->y, normdx, normdy);
	int dx = player->x - npc->x ;
	int dy = player->y - npc->y ;

	//플레이어와 몬스터스폰중점과 몬스터 사이의 관계
	//몬스터가 너무 플레이어와 멀면 flag = 1 이 될것이다.
	int moverangex = abs(spawnX - (npc->x + dx));
	int moverangey = abs(spawnY - (npc->y + dy));
	int flag = 0;	//범위밖이라 못가는경우면 flag = 1

	
	//-------------------
	unordered_set <int> old_viewlist;
	unordered_set <int> new_viewlist;
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			old_viewlist.insert(obj->_id);
	}
	//이동 시작
	if (moverangex <= movelimit)
	{
		if(npc->x + normdx > 0 || npc->x + normdx < WORLD_WIDTH)
			if (mMap[npc->x + normdx][npc->y] == 0)
				npc->x += normdx;
	}
	else if (moverangex > movelimit)
		flag = 1;

	if (moverangey <= movelimit)
	{
		if(npc->y + normdy > 0 || npc->y + normdy < WORLD_HEIGHT)
			if (mMap[npc->x][npc->y + normdy] == 0)
				npc->y += normdy;
	}
	else if (moverangey > movelimit)
		flag = 1;
	//이동 끝
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			new_viewlist.insert(obj->_id);
	}

	// 새로 시야에 들어온 플레이어
	for (auto pl : new_viewlist) {
		if (0 == old_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.insert(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();

			send_put_object(pl, npc_id, characters[npc_id]->imageType);
		}
		else {
			send_move_packet(pl, npc_id);
		}
	}
	// 시야에서 사라지는 경우
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.erase(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();
			send_remove_object(pl, npc_id);
		}
	}
	//----------------------------

	//타게팅 안풀리면 체이스로 계속오니까 피쓰풀이나 어그로나 다 1번정도만 시도 해줌.
	//몬스터의 이동 사거리 밖으로 나가서 기다리는것, 
	//어그로가 풀리고나서 어그로 범위 밖으로 나가면 어그로가 더이상 끌리지않는다.
	npc->CheckMonsterChasing(flag);

	return 0;
}



//npc가 플레이어와 멀어져서 시야가 안보이면 더이상 npc_ai안돌리게해줌.
void do_npc_move(int npc_id, int spawnX, int spawnY, int movelimit)
{
	unordered_set <int> old_viewlist;
	unordered_set <int> new_viewlist;
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			old_viewlist.insert(obj->_id);
	}
	auto& x = characters[npc_id]->x;
	auto& y = characters[npc_id]->y;

	int oldsx = x / 100;
	int oldsy = y / 100;
	switch (rand() % 4) {
	case 0:
		if (y > 0)
		{
			int moverangey = abs(spawnY - (y - 1));
			if(moverangey <= movelimit)
				if (mMap[x][y - 1] == 0)
					y--;
		}
		break;
	case 1:
		if (y < (WORLD_HEIGHT - 1))
		{
			int moverangey = abs(spawnY - (y + 1));
			if (moverangey <= movelimit)
				if (mMap[x][y + 1] == 0)
					y++;
		}
		break;
	case 2:
		if (x > 0)
		{
			int moverangex = abs(spawnX - (x - 1));
			if (moverangex <= movelimit)
				if (mMap[x - 1][y] == 0)
					x--;
		}
		break;
	case 3:
		if (x < (WORLD_WIDTH - 1))
		{
			int moverangex = abs(spawnX - (x + 1));
			if (moverangex <= movelimit)
				if (mMap[x + 1][y] == 0)
					x++;
		}
		break;
	}

	int sx = x / 100;	//sectionX
	int sy = y / 100;	//sectionY

	if ((oldsy != sy) || (oldsx != sx))
	{
		section_lock[oldsy][oldsx].lock();
		CSection[oldsy][oldsx].erase(remove(CSection[oldsy][oldsx].begin(), CSection[oldsy][oldsx].end(), npc_id), CSection[oldsy][oldsx].end());
		section_lock[oldsy][oldsx].unlock();

		section_lock[sy][sx].lock();
		CSection[sy][sx].push_back(npc_id);
		section_lock[sy][sx].unlock();
	}
	//unordered_set <int> nearlist;
	//for (int i = min(0, sx - 1); i < max(20, sx + 1); ++i)
	//{
	//	for (int j = min(0, sy - 1); j < max(20, sy + 1); ++j)
	//	{
	//		section_lock[j][i].lock();
	//		vector<int> Sector{ CSection[j][i] };
	//		section_lock[j][i].unlock();

	//		for (auto other : Sector)
	//		{
	//			if (characters[other]->_state != Character::STATE::ST_INGAME)
	//				continue;
	//			//if (false == characters[other]->is_player())
	//			//	break;
	//			if (true == is_Near(npc_id, other))
	//				new_viewlist.insert(other);
	//		}
	//	}
	//}
	// 
	
	//-0--------------- npc는 플레이어만 판단하면 되니깐 그렇게 과부하는 안 올듯 ? 
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			new_viewlist.insert(obj->_id);
	}
	//------------------



	// 새로 시야에 들어온 플레이어
	int flag = 0;
	for (auto pl : new_viewlist) {
		if (0 == old_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.insert(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();

			send_put_object(pl, npc_id, characters[npc_id]->imageType);
		}
		else {
			send_move_packet(pl, npc_id);
		}
	}
	// 시야에서 사라지는 경우
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.erase(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();
			send_remove_object(pl, npc_id);
		}
	}

	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);
	npc->isMoving = false;
	//시야에들어왔는지 확인하는 viewlist를 기반으로 
	//공격 반경내에 들어왔는지 한번더 판별
	unordered_set<int> AttackRangeList;
	bool AttackFlag = false;
	if (npc->GetMonType() == MonsterType::Agro)
	{
		AngryMonster* monster = reinterpret_cast<AngryMonster*>(npc);
		for (auto player_id : new_viewlist)
		{
			if (is_Near_By_Range(player_id, npc_id, monster->AgroRange))
			{
				AttackRangeList.insert(player_id);
				AttackFlag = true;
			}
		}
	}

	//npc시야에 아무도없으면 더이상 timer 바쁘게 안함.
	if (new_viewlist.size() > 0)
		flag = 1;
	if (flag == 1)
	{
		if (npc->isMoving == true)
			return;
		npc->isMoving = true;

		switch (npc->GetMonType())
		{
		case MonsterType::Peace:
		{
			//랜덤무브는 타겟이없다.
			npc->SetTarget(INVALID_TARGET);
			Timer_Event instq;
			instq.exec_time = chrono::system_clock::now() + 1000ms;
			instq.player_id = INVALID_TARGET;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
			instq.npc_id = npc_id;
			timer_queue.push(instq);
		}
		break;
		case MonsterType::Agro:
		{
			if (AttackFlag)
			{
				//랜덤무브하다가 어그로 반경내에 들어왔다면 뷰리스트중 젤 앞에있는얘를 따라감.
				npc->SetTarget(*new_viewlist.begin());
				Timer_Event instq;
				instq.exec_time = chrono::system_clock::now() + 1000ms;
				instq.player_id = npc->GetTarget();
				instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
				instq.npc_id = npc_id;
				timer_queue.push(instq);
			}
			else {
				npc->SetTarget(INVALID_TARGET);
				Timer_Event instq;
				instq.exec_time = chrono::system_clock::now() + 1000ms;
				instq.player_id = INVALID_TARGET;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
				instq.npc_id = npc_id;
				timer_queue.push(instq);
			}
		}
		break;
		}

	}

}


int CPP_TelePortTarget(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -2);
	int player_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 3);

	Npc* npc = reinterpret_cast<Npc*>(characters[npc_id]);
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	
	unordered_set <int> old_viewlist;
	unordered_set <int> new_viewlist;
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			old_viewlist.insert(obj->_id);
	}

	//npc 순간이동.
	npc->x = player->x;
	npc->y = player->y;

	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			new_viewlist.insert(obj->_id);
	}


	for (auto pl : new_viewlist) {
		if (0 == old_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.insert(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();

			send_put_object(pl, npc_id, characters[npc_id]->imageType);
		}
		else {
			send_move_packet(pl, npc_id);
		}
	}
	// 시야에서 사라지는 경우
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.erase(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();
			send_remove_object(pl, npc_id);
		}
	}







	return 0;
}

int CPP_BossDeBuff(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -4);
	int player_id = (int)lua_tointeger(L, -3);
	int debuffType = (int)lua_tointeger(L, -2);
	int debuffTime = (int)lua_tointeger(L, -1);
	lua_pop(L, 5);
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);

	switch (debuffType)
	{
	case 1: {
		//기절
		player->debufftype = 1;
		Timer_Event instq;
		instq.exec_time = chrono::system_clock::now() + chrono::seconds(debuffTime);
		instq.player_id = player_id;
		instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_STUNNED;
		timer_queue.push(instq);

		player->vl.lock();
		unordered_set<int> viewlist = player->viewlist;
		player->vl.unlock();
		//나 자신에게 업데이트
		send_buffUpdate_packet(player_id, player_id);
		//내가 보이는 사람들에게 업데이트
		for (auto& p : viewlist)
		{	
			if (characters[p]->is_Npc()) continue;
			send_buffUpdate_packet(p, player_id);
		}

		break;
	}
	}

	npc->ReloadAttack();

	return 0;
}

int CPP_BossBuffMySight(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -3);
	int BuffType = (int)lua_tointeger(L, -2);
	int BuffTime = (int)lua_tointeger(L, -1);
	lua_pop(L, 4);

	Npc* npc = reinterpret_cast<Npc*>(characters[npc_id]);

	unordered_set <int> viewlist;
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			viewlist.insert(obj->_id);
	}

	
	switch (BuffType)
	{
	case 0:
		break;
	case 1: {	//공격력 증가 버프.
		for (auto& p_id: viewlist)
		{
			Player* player = reinterpret_cast<Player*>(characters[p_id]);
			player->bufftype = 1;
			Timer_Event instq;
			instq.exec_time = chrono::system_clock::now() + chrono::seconds(BuffTime);
			instq.player_id = p_id;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_BUFF;
			timer_queue.push(instq);

			player->vl.lock();
			unordered_set<int> viewlist = player->viewlist;
			player->vl.unlock();
			//나 자신에게 업데이트
			send_buffUpdate_packet(p_id, p_id);
			//내가 보이는 사람들에게 업데이트
			for (auto& p : viewlist)
			{
				if (characters[p]->is_Npc()) continue;
				send_buffUpdate_packet(p, p_id);
			}
		}
		break;
	}
	}


	return 0;
}

int CPP_NoticeWindow(lua_State* L)
{

	int player_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);
	lua_pop(L, 3);

	return 0;
}

int CPP_NoticeWindowOK(lua_State* L)
{

	int player_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 3);

	send_npc_packet(player_id);

	//int hp = characters[npc_id]->hp;
	//lua_pushnumber(L, hp);

	return 0;
}
