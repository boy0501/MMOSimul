#include <iostream>
#include <thread>
#include "Source/Game/Network/Thread/TimerThread/TimerThread.h"
#include "Source/Game/Network/Thread/WorkerThread/WorkerThread.h"
#include "Source/Game/Object/Character/Character.h"
#include "Source/Game/Object/Character/Player/Player.h"
#include "Source/Game/Network/Network.h"
#include "Source/DB/DB.h"
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")




using namespace std;


int main()
{
	setlocale(LC_ALL, "korean");
	wcout.imbue(locale("korean"));
	InitializeCriticalSection(&db_cs);
	for(int i = 0; i < 6 ;++i)
		InitializeDB(henv[i],hdbc[i],hstmt[i]);
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	listen(s_socket, SOMAXCONN);
	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(s_socket), g_iocp, 0, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char	accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	WSA_OVER_EX	accept_ex;
	*(reinterpret_cast<SOCKET*>(accept_ex.getBuf())) = c_socket;
	ZeroMemory(&accept_ex.getWsaOver(), sizeof(accept_ex.getWsaOver()));
	accept_ex.setCmd(CMD_ACCEPT);

	AcceptEx(s_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex.getWsaOver());
	cout << "Accept Called\n";

	for (int i = 0; i < MAX_USER; ++i)
	{
		characters[i] = new Player();
	}
	cout << "Creating Worker Threads\n";
	InitMAP();
	InitNPC();
	
	vector <thread> worker_threads;
	thread timer_thread{ TimerThread };
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(WorkerThread,i);
	for (auto& th : worker_threads)
		th.join();

	timer_thread.join();
	for (auto character : characters) {
		if (Character::STATE::ST_INGAME == character->_state)
			Disconnect(character->_id);
	}


	for (int i = 0; i < 6; ++i)
		ReleaseDB(henv[i], hdbc[i], hstmt[i]);
	DeleteCriticalSection(&db_cs);
	closesocket(s_socket);
	WSACleanup();

}