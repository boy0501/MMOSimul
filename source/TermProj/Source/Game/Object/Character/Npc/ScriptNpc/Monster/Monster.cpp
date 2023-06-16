#include "Monster.h"
#include <chrono>

using namespace std;

Monster::Monster(const char* scriptname, int n_id, MonsterType montype, MonsterMoveType monmovetype)
	: ScriptNpc(scriptname, n_id)
	, FailToChaseTarget(0)
	, Peace_Notice_Range(0)
	, monType(montype)
	, monMoveType(monmovetype)
	, attack_cooltime(std::chrono::system_clock::now())
	, target(INVALID_TARGET)
	, attackRange(0)
{
}

Monster::~Monster()
{
}


bool Monster::is_Near_By_Range_Target_Peace()
{
	if (Peace_Notice_Range < abs(characters[target]->x - x)) return false;
	if (Peace_Notice_Range < abs(characters[target]->y - y)) return false;
	return true;
}

void Monster::DeleteTarget()
{
	if (monMoveType == MonsterMoveType::MT_Siege)
	{
		if (target != INVALID_TARGET)
		{
			if (is_Near_By_Range_Target_Peace())
			{
				
				//��¶�� �굵 ������ �õ��ϴ°��̹Ƿ� ������Ÿ���� ��������.
				attack_cooltime = chrono::system_clock::now() + 1000ms;
				Timer_Event instq;
				auto timing = chrono::system_clock::now() + 1000ms;
				instq.player_id = target;
				instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
				instq.npc_id = _id;
				instq.exec_time = timing;
				timer_queue.push(instq);
			}
			else {
				target = INVALID_TARGET;
			}
		}
	}
}

void Monster::ReloadAttack()
{
	attack_cooltime = chrono::system_clock::now() + 1000ms;
	Timer_Event instq;
	auto timing = chrono::system_clock::now() + 1000ms;
	instq.player_id = target;
	instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
	instq.npc_id = _id;
	instq.exec_time = timing;
	timer_queue.push(instq);
}

int Monster::RewardEXP()
{
	int EXP = level* level * 2;
	if (monType == MonsterType::Agro)
		EXP *= 2;
	if (monMoveType == MonsterMoveType::MT_Move)
		EXP *= 2;

	return EXP;
}

void Monster::CheckMonsterChasing(int flag)
{
	FailToChaseTarget += flag;
	if (FailToChaseTarget > 1)
	{
		target = INVALID_TARGET;
		isMoving = false;
		FailToChaseTarget = 0;
		//ã�°� ����. �ǽ�Ǯ�� ��ȭ�� �����ϱ⿡,,
		//��¶�� ai�� �ٽ� �����Ų���� Ÿ���� -1�̱⶧���� 
		//������ ���� ü�̽� ���ϰ�,  ���������Ұ���. 
		//�������� �ϱ����� isMoving = false�� ����.
		Timer_Event instq;
		instq.exec_time = chrono::system_clock::now() + 1000ms;
		instq.player_id = INVALID_TARGET;
		instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
		instq.npc_id = _id;
		timer_queue.push(instq);
	}
	else {
		//cout << "Ÿ�̸� �־��ִ°� 1" << endl;
		//�������� �������� ������ �õ��� �Ϸ������Ƿ�, ��Ÿ���� �־��ش�.
		ReloadAttack();
	}
}
