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
				
				//어쨋든 얘도 공격을 시도하는것이므로 어택쿨타임을 돌려야함.
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
		//찾는걸 포기. 피스풀은 평화를 좋아하기에,,
		//어쨋든 ai를 다시 실행시킨다음 타겟이 -1이기때문에 
		//어차피 이제 체이스 안하고,  랜덤무브할거임. 
		//랜덤무브 하기위해 isMoving = false로 해줌.
		Timer_Event instq;
		instq.exec_time = chrono::system_clock::now() + 1000ms;
		instq.player_id = INVALID_TARGET;
		instq.type = Timer_Event::TIMER_TYPE::TYPE_NPC_AI;
		instq.npc_id = _id;
		timer_queue.push(instq);
	}
	else {
		//cout << "타이머 넣어주는곳 1" << endl;
		//때리지는 못했지만 때리는 시도를 하려했으므로, 쿨타임을 넣어준다.
		ReloadAttack();
	}
}
