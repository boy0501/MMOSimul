#include "Character.h"


Character::Character(TYPE type,STATE state)
	:_state(state)
	,_type(type)
	,_id(-1)
	,_is_active(true)
	,imageType(OBJECT_PLAYER)
{
	x = 0;
	y = 0;
}

Character::~Character()
{

}

bool Character::is_Npc()
{
	return (_id >= NPC_ID_START) && (_id <= CONVNPC_ID_END);
}

bool Character::is_player()
{
	return (_id >= 0) && (_id < MAX_USER);
}