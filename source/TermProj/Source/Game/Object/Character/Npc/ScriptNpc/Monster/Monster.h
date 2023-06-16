#pragma once
#include "../ScriptNpc.h"

#define RANDOMMOVE 5
#define INVALID_TARGET -1

enum MonsterType { Peace, Agro };
enum MonsterMoveType { MT_Siege, MT_Move };

class Monster :   public ScriptNpc
{

public:
	Monster(const char* scriptname, int n_id,
		MonsterType montype = MonsterType::Peace,
		MonsterMoveType monmovetype = MonsterMoveType::MT_Siege);
	virtual ~Monster();

	void SetTarget(int target_id) { target = target_id; }
	void SetMonType(const MonsterType& mType) { monType = mType; }
	void SetAttackCoolTime(const std::chrono::system_clock::time_point& time) { attack_cooltime = time; }


	int GetTarget() const { return target; }
	MonsterType GetMonType() const { return monType; }
	MonsterMoveType GetMonMoveType() const { return monMoveType; }
	std::chrono::system_clock::time_point GetAttackCoolTime() const {return attack_cooltime;}

	bool is_Near_By_Range_Target_Peace();
	void DeleteTarget();
	void ReloadAttack();
	int RewardEXP();
	void CheckMonsterChasing(int flag);
protected:
	int FailToChaseTarget;
	int Peace_Notice_Range;
	int attackRange;
	int target;	//몬스터 어글자 대상
	MonsterType monType;
	MonsterMoveType monMoveType;
	std::chrono::system_clock::time_point attack_cooltime;

};

