#include "Network.h"

#include "../../TermProj/Source/Game/Network/protocol.h"

sf::TcpSocket socket;


void send_attack_packet()
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}


void send_npc_packet()
{
	cs_packet_npc_interact packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_NPC_INTERACT;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_npc_packet_response(char res)
{
	cs_packet_npc_response packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_NPC_RESPONSE;
	packet.response = res;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_move_packet(char dr)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	packet.direction = dr;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_login_packet(string& name, string& pw)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, name.c_str());
	strcpy_s(packet.pw, pw.c_str());
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_telportcheat_packet(int x, int y)
{
	cs_packet_teleportCheat packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_TELEPORTCHEAT;
	packet.x = x;
	packet.y = y;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}