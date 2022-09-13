#include <iostream>
#include <chrono>
#include <thread>
#include <concurrent_priority_queue.h>
#include "TimerThread.h"
#include "../../Network.h"
#include "../../../Object/Character/Npc/ScriptNpc/ScriptNpc.h"

using namespace std;

void TimerThread()
{
	bool triger = false;
	Timer_Event is_already;
	while (true) {
		//�̰� �������� ���ư� �� �־��� .
		while (true) {
			if (triger)
			{
				if (is_already.exec_time <= chrono::system_clock::now())
				{
					if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_NPC_AI)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_NPC_AI);
						memcpy(wsa_ex->getBuf(), &is_already.player_id, sizeof(int));
						PostQueuedCompletionStatus(g_iocp, 1, is_already.npc_id, &wsa_ex->getWsaOver());
						//Ÿ�� ���� �ٸ� Cmd�� �־��ָ� Script���� �ƴ��� ������ �����ϴ�. 
					}
					else if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_NPC_RESPAWN)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_NPC_RESPAWN);
						PostQueuedCompletionStatus(g_iocp, 1, is_already.npc_id, &wsa_ex->getWsaOver());

					}
					else if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_HEAL)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_PLAYER_HEAL);
						PostQueuedCompletionStatus(g_iocp, 1, is_already.player_id, &wsa_ex->getWsaOver());
					}
					else if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_STUNNED)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_PLAYER_STUNNED);
						PostQueuedCompletionStatus(g_iocp, 1, is_already.player_id, &wsa_ex->getWsaOver());
					}
					else if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_BUFF)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_PLAYER_BUFFREMOVE);
						PostQueuedCompletionStatus(g_iocp, 1, is_already.player_id, &wsa_ex->getWsaOver());
					}
					else if (is_already.type == Timer_Event::TIMER_TYPE::TYPE_AUTO_SAVE)
					{
						WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
						wsa_ex->setCmd(CMD_AUTO_SAVE);
						PostQueuedCompletionStatus(g_iocp, 1, is_already.player_id, &wsa_ex->getWsaOver());
					}
					triger = false;

				}
				continue;
			}
			Timer_Event exec_event;
			if (!timer_queue.try_pop(exec_event))
				break;
			if (exec_event.exec_time <= chrono::system_clock::now())
			{
				if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_NPC_AI)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_NPC_AI);
					memcpy(wsa_ex->getBuf(), &exec_event.player_id, sizeof(int));
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.npc_id, &wsa_ex->getWsaOver());
					//Ÿ�� ���� �ٸ� Cmd�� �־��ָ� Script���� �ƴ��� ������ �����ϴ�. 
				}
				else if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_NPC_RESPAWN)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_NPC_RESPAWN);
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.npc_id, &wsa_ex->getWsaOver());

				}
				else if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_HEAL)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_PLAYER_HEAL);
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.player_id, &wsa_ex->getWsaOver());
				}
				else if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_STUNNED)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_PLAYER_STUNNED);
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.player_id, &wsa_ex->getWsaOver());
				}
				else if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_PLAYER_BUFF)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_PLAYER_BUFFREMOVE);
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.player_id, &wsa_ex->getWsaOver());
				}
				else if (exec_event.type == Timer_Event::TIMER_TYPE::TYPE_AUTO_SAVE)
				{
					WSA_OVER_EX* wsa_ex = new WSA_OVER_EX;
					wsa_ex->setCmd(CMD_AUTO_SAVE);
					PostQueuedCompletionStatus(g_iocp, 1, exec_event.player_id, &wsa_ex->getWsaOver());
				}
				
			}
			else {
				if (exec_event.exec_time > chrono::system_clock::now() + 1000ms)
				{// ��ٷ��� �ϴ� �ð��� 1���̻��̶�� �ٸ� �� �߿��Ѱ� �� ���̿� ���� �� �����Ƿ�. 1�������� ��ٷ���. �׸��� �ٽ� ����.
					timer_queue.push(exec_event);
					this_thread::sleep_for(1000ms);
				}
				else {
					is_already = exec_event;
					triger = true;
					break;
				}
			}
		}
		this_thread::sleep_for(chrono::duration_cast<chrono::milliseconds>
			(is_already.exec_time - chrono::system_clock::now()));
	}
}