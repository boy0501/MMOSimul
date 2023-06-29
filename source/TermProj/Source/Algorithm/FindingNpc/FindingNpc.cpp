#include "../../Game/Network/Network.h"
#include "../../Game/Object/Character/Player/Player.h"
#include "../../Game/Object/Character/Npc/ScriptNpc/ConvNpc/ConvNpc.h"
#include "FindingNpc.h"


/*
플레이어 주변 1칸의 npc를 감지하기 위해서는
1. 모든 엔티티를 확인하며 주변을 확인 하던가, 
2. viewlist에 있는것만 확인 하던가, 
3. 따로 자료구조를 만들던가
셋중에 하나를 선택하면 된다.
1번은 아예 논외로 치고
2,3번중 본인은 2번을 택했는데,

1. npc의 위치는 고정이므로 const하게 읽기만 할 용도임.
2. npc가 얼마나 늘어날지 예측하지 못함
3. npc는 대부분 한 곳에 몰려있을 예정 (자료구조를 새로 만든다면 Section크기에 맞출텐데 안쓰는 게 많을 것 이므로 부적합함)
4. viewlist에 인원이 많아졌을때는 어차피 npc를 찾기 위해 돌리는 렉 보다, 이미 움직이는 네트워크 부하가 더 클것이라 예상
5. 굳이 새로 만들지말고, 이미 있는걸 활용하기

*/

int FindingNearNpc(int player_id)
{
	Player* player = dynamic_cast<Player*>(characters[player_id]);
	if (nullptr == player) return -1;

	int sx = player->x / 100;	//sectionX
	int sy = player->y / 100;	//sectionY

	player->vl.lock();	
	std::unordered_set<int> pvlist{ player->viewlist };
	player->vl.unlock();
	
	for (auto& npc_id : pvlist)
	{
		ConvNpc* npc = dynamic_cast<ConvNpc*>(characters[npc_id]);
		if (nullptr == npc) continue;

		if (is_Near_By_Range(player_id, npc_id, 1))
		{
			return npc_id;
		}
	}
	
	return -1;
}
