#pragma once
#include "../Object.h"
#include "../../Network/protocol.h"
#include <WS2tcpip.h>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include "../../Network/Network.h"
class Character : public Object
{
public:
	enum class STATE { ST_FREE, ST_ACCEPT, ST_INGAME };
	enum class TYPE { NONE, PLAYER, SCRIPTNPC, NOSCRIPTNPC };
	Character(TYPE type = TYPE::NONE, STATE state = STATE::ST_FREE);
	virtual ~Character();
	char name[MAX_NAME_SIZE];
	int	   _id;
	std::atomic_short hp;
	short maxhp;
	short level;
	char imageType;
	TYPE	_type;	
	STATE _state;
	std::mutex state_lock;
	std::atomic_bool	_is_active;
	bool is_Npc();
	bool is_player();
};

