#include "../../Game/Network/Network.h"
#include "../../Game/Object/Character/Player/Player.h"
#include "../../Game/Object/Character/Npc/ScriptNpc/ConvNpc/ConvNpc.h"
#include "FindingNpc.h"


/*
�÷��̾� �ֺ� 1ĭ�� npc�� �����ϱ� ���ؼ���
1. ��� ��ƼƼ�� Ȯ���ϸ� �ֺ��� Ȯ�� �ϴ���, 
2. viewlist�� �ִ°͸� Ȯ�� �ϴ���, 
3. ���� �ڷᱸ���� �������
���߿� �ϳ��� �����ϸ� �ȴ�.
1���� �ƿ� ��ܷ� ġ��
2,3���� ������ 2���� ���ߴµ�,

1. npc�� ��ġ�� �����̹Ƿ� const�ϰ� �б⸸ �� �뵵��.
2. npc�� �󸶳� �þ�� �������� ����
3. npc�� ��κ� �� ���� �������� ���� (�ڷᱸ���� ���� ����ٸ� Sectionũ�⿡ �����ٵ� �Ⱦ��� �� ���� �� �̹Ƿ� ��������)
4. viewlist�� �ο��� ������������ ������ npc�� ã�� ���� ������ �� ����, �̹� �����̴� ��Ʈ��ũ ���ϰ� �� Ŭ���̶� ����
5. ���� ���� ����������, �̹� �ִ°� Ȱ���ϱ�

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
