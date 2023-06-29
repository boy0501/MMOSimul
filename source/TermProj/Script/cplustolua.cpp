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

	//Chase �� ����� ���ٸ� Random Move�Ѵ�.
	// 	   �ٵ� Chase�� ����� ������������ (autoMove�Ϸ��µ� target�� ���ܹ������)
	// 	   �׷��� Chase�� ������ ���� ���ư����� �ȵ�
	// 	   Chase�� �������� �׶� ���ư����� ����. Attack �߰��ϰ�,,
	//     �׷��ٴ� ����. ������ peace�Ӹ��� �ƴ϶� ��׷ε� ������ ��������. 
	//
	if (npc->GetTarget() == INVALID_TARGET)
		do_npc_move(npc_id, spawnX, spawnY, movelimit);
	
	return 0;
}

int CPP_SendMessage(lua_State* L)
{
	//��󿡰� �޽��� ����.
	int npc_id = (int)lua_tointeger(L, -3);
	int player_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);
	send_chat_packet(player_id, npc_id, mess);
	return 0;
}

int CPP_SendMessageMySight(lua_State* L)
{
	//Sight�� ���̴� ��󿡰� �޽��� ����. update�� �ʿ��ϴٸ� update�� ����.
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
	sprintf_s(buf, "���� %s�� ���񷯼� %d�� ����ġ ȹ��!", npc->name, exp);

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

	//���Ͱ� ������ active ����.
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

	//30�� �� ��Ȱ.
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
			sprintf_s(logbuf, MAX_CHAT_SIZE, "�÷��̾� ���");
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



			//������ �ִ� �÷��̾�鿡�� �����ٰ� �˷��ش�.
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
						npc->SetTarget(INVALID_TARGET);	//NPC Ÿ���ô���̾��ٸ� Ǯ���ش�.
					}
				}
				else {
					send_remove_object(p_id, player_id);//�÷��̾��� remove���ش�.
				}
			}


			// ���� ������ �÷��̾��� ������ ���� �÷��̾�� ������
			for (auto other : characters) {
				if (other->is_Npc()) break;
				if (other->_id == player_id) continue;
				//if npc -> break  So, under lines other is player
				// npc�� break������ �� �Ʒ����� other�� ��� player�̴�.
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

			// ���� ������ �÷��̾�� ���� ��ü ������ ������
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
			//������ ���� vl������, vl�� npc�� �ִٸ� queue�� �־��ش�.
			//�̰� �÷��̾ ��Ƴ��� �� �ֺ����� ��Ȱ�����ִ� �� ��.
			push_queue(player_id);

			//�÷��̾� �׾����� �ٽ� ���ʹ� ���������Ұ���.
			Timer_Event instq;
			instq.exec_time = chrono::system_clock::now() + 1000ms;
			instq.player_id = INVALID_TARGET;
			instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
			instq.npc_id = npc_id;
			timer_queue.push(instq);
		}
		else {
			char logbuf[MAX_CHAT_SIZE];
			sprintf_s(logbuf, MAX_CHAT_SIZE, "����[%s]��/�� �÷��̾�� %d��ŭ�� ���ظ� ����", npc->name, damage);
			send_log_packet(player_id, logbuf);
			//�÷��̾ ���׾����� ��� ����
			//�������̵�, �����̴����� �ѹ� �������� �ٽ� ��������.
			//cout << "Ÿ�̸ӳ־��ִ°� 3" << endl;
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
		//cout << "�÷��̾� hp: " << characters[player_id]->hp << endl;
		send_status_change_packet(player_id);
	}
	else {
	//���� ���� ����! �̵����� �� �پ�� �����ؼ� ������ ������
	//������� �ָ��־ �����ϴ� ��쵵 ����. 
	// Ÿ�� ��ü�� ������� ��쵵 ���� (�÷��̾ �׾) �׷� ���� �ʿ� ����.
	//������ ��� ���� �ȿ� �ִٸ�~ Ai�� ��� ���ư�. 
	//��۹����ȿ��� ���ٸ� ? ��¥ ���� ����� ���°�. Ÿ�� ������.
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
	//cout << "ü�̽��ϳ�" << endl;
	int npc_id = (int)lua_tointeger(L, -5);
	int player_id = (int)lua_tointeger(L, -4);
	int spawnX = (int)lua_tointeger(L, -3);
	int spawnY = (int)lua_tointeger(L, -2);
	int movelimit = (int)lua_tointeger(L, -1);
	lua_pop(L, 6);

	Monster* npc = reinterpret_cast<Monster*>(characters[npc_id]);
	Player* player = reinterpret_cast<Player*>(characters[player_id]);
	// A*������ �� 
	int normdx = 0;
	int normdy = 0;
	AStarAlgo(npc->x, npc->y, player->x, player->y, normdx, normdy);
	int dx = player->x - npc->x ;
	int dy = player->y - npc->y ;

	//�÷��̾�� ���ͽ��������� ���� ������ ����
	//���Ͱ� �ʹ� �÷��̾�� �ָ� flag = 1 �� �ɰ��̴�.
	int moverangex = abs(spawnX - (npc->x + dx));
	int moverangey = abs(spawnY - (npc->y + dy));
	int flag = 0;	//�������̶� �����°��� flag = 1

	
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
	//�̵� ����
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
	//�̵� ��
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			new_viewlist.insert(obj->_id);
	}

	// ���� �þ߿� ���� �÷��̾�
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
	// �þ߿��� ������� ���
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			reinterpret_cast<Player*>(characters[pl])->vl.lock();
			reinterpret_cast<Player*>(characters[pl])->viewlist.erase(npc_id);
			reinterpret_cast<Player*>(characters[pl])->vl.unlock();
			send_remove_object(pl, npc_id);
		}
	}
	//----------------------------

	//Ÿ���� ��Ǯ���� ü�̽��� ��ӿ��ϱ� �Ǿ�Ǯ�̳� ��׷γ� �� 1�������� �õ� ����.
	//������ �̵� ��Ÿ� ������ ������ ��ٸ��°�, 
	//��׷ΰ� Ǯ������ ��׷� ���� ������ ������ ��׷ΰ� ���̻� �������ʴ´�.
	npc->CheckMonsterChasing(flag);

	return 0;
}



//npc�� �÷��̾�� �־����� �þ߰� �Ⱥ��̸� ���̻� npc_ai�ȵ���������.
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
	
	//-0--------------- npc�� �÷��̾ �Ǵ��ϸ� �Ǵϱ� �׷��� �����ϴ� �� �õ� ? 
	for (auto obj : characters) {
		if (obj->_state != Character::STATE::ST_INGAME)
			continue;
		if (false == obj->is_player())
			break;
		if (true == is_Near(npc_id, obj->_id))
			new_viewlist.insert(obj->_id);
	}
	//------------------



	// ���� �þ߿� ���� �÷��̾�
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
	// �þ߿��� ������� ���
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
	//�þ߿����Դ��� Ȯ���ϴ� viewlist�� ������� 
	//���� �ݰ泻�� ���Դ��� �ѹ��� �Ǻ�
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

	//npc�þ߿� �ƹ��������� ���̻� timer �ٻڰ� ����.
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
			//��������� Ÿ���̾���.
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
				//���������ϴٰ� ��׷� �ݰ泻�� ���Դٸ� �丮��Ʈ�� �� �տ��ִ¾긦 ����.
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

	//npc �����̵�.
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
	// �þ߿��� ������� ���
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
		//����
		player->debufftype = 1;
		Timer_Event instq;
		instq.exec_time = chrono::system_clock::now() + chrono::seconds(debuffTime);
		instq.player_id = player_id;
		instq.type = Timer_Event::TIMER_TYPE::TYPE_PLAYER_STUNNED;
		timer_queue.push(instq);

		player->vl.lock();
		unordered_set<int> viewlist = player->viewlist;
		player->vl.unlock();
		//�� �ڽſ��� ������Ʈ
		send_buffUpdate_packet(player_id, player_id);
		//���� ���̴� ����鿡�� ������Ʈ
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
	case 1: {	//���ݷ� ���� ����.
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
			//�� �ڽſ��� ������Ʈ
			send_buffUpdate_packet(p_id, p_id);
			//���� ���̴� ����鿡�� ������Ʈ
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
