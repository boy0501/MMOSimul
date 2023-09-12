#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

using namespace std;

extern sf::TcpSocket socket;

void send_npc_packet_response(char res);

void send_attack_packet();

void send_npc_packet();

void send_npc_packet_response(char res);

void send_move_packet(char dr);

void send_login_packet(string& name, string& pw);

void send_telportcheat_packet(int x, int y);