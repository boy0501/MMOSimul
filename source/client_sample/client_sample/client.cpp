#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <functional>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <fstream>
#include "Button.h"
#include "Dialog.h"
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma warning (disable:4996)
#include "../../TermProj/Source/Game/Network/protocol.h"

sf::TcpSocket socket;

constexpr auto BUF_SIZE = 256;
constexpr auto SCREEN_WIDTH = 17;
constexpr auto SCREEN_HEIGHT = 17;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;
//constexpr auto BUF_SIZE = MAX_BUFFER;

int g_myid;
int g_x_origin;
int g_y_origin;

sf::RenderWindow* g_window;
sf::Font g_font;
sf::Text mLevel;
vector<Button*> Buttons;
vector<Dialog*> Dialogs;

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Sprite m_BuffSprite;
	sf::Sprite m_DebuffSprite;
	sf::Text m_name;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;
public:
	char buff = 0;
	char debuff = 0;
	int m_x, m_y;
	short level;
	short hp, maxhp;
	int exp;

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}
	OBJECT() {
		m_showing = false;
	}
	void setTexture(sf::Texture& t, int x, int y, int x2, int y2)
	{
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}
	void setBuffsTexture(sf::Texture& buff, sf::Texture& debuff)
	{
		m_BuffSprite.setTexture(buff);
		m_BuffSprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
		m_DebuffSprite.setTexture(debuff);
		m_DebuffSprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_x_origin) * 65.0f + 8;
		float ry = (m_y - g_y_origin) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);

		if (buff == 1)
		{
			m_BuffSprite.setPosition(rx, ry);
			g_window->draw(m_BuffSprite);
		}
		if (debuff == 1)
		{
			m_DebuffSprite.setPosition(rx, ry);
			g_window->draw(m_DebuffSprite);
		}

		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx - 10, ry - 20);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx - 10, ry - 20);
			g_window->draw(m_chat);
		}
	}
	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}
	void set_chat(const char str[]) {
		m_chat.setFont(g_font);
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
};

OBJECT avatar;
OBJECT players[MAX_USER + MAX_NPC + MAX_CONVNPC];
OBJECT rock[200000];

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* pieces;
sf::Texture* rocks;
sf::Texture* ui;
sf::Texture* hp;
sf::Texture* exptexture;
sf::Texture* plant;
sf::Texture* pig;
sf::Texture* angrypig;
sf::Texture* veryangryPig;
sf::Texture* Boss1;
sf::Texture* PowerUp;
sf::Texture* Stunned;
sf::Texture* Bimg,*Bdownimg,*Dlgimg;
int objcount = 0;
void map_initialize()
{
	ifstream in;
	int x, y, type;
	in.open("map1.txt", ios::in);
	if (in.is_open())
	{
		for (int i = 0; ; ++i)
		{
			if (in.eof())
			{
				objcount = i;
				cout << "Done" << endl;
				break;
			}
			in >> x >> y >> type;
			if(type == 1)
				rock[i] = OBJECT{ *rocks,0,0,64,64 };
			rock[i].m_x = x;
			rock[i].m_y = y;
			rock[i].set_name("");
			rock[i].show();
		}
	}
	else {
		cout << " map doesn't exits!" << endl;
	}
}
void client_initialize()
{
	rocks = new sf::Texture;
	board = new sf::Texture;
	pieces = new sf::Texture;
	ui = new sf::Texture;
	hp = new sf::Texture;
	exptexture = new sf::Texture;
	plant = new sf::Texture;
	pig = new sf::Texture;
	angrypig = new sf::Texture;
	veryangryPig = new sf::Texture;
	Boss1 = new sf::Texture;
	PowerUp = new sf::Texture;
	Stunned = new sf::Texture;
	Bimg = new sf::Texture;
	Bdownimg = new sf::Texture;
	Dlgimg = new sf::Texture;
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		while (true);
	}
	bool err = 0;
	err = board->loadFromFile("chessmap.bmp");
	err = pieces->loadFromFile("chess2.png");
	err = rocks->loadFromFile("rock.png");
	err = ui->loadFromFile("realui.png");
	err = hp->loadFromFile("Ui_HP.png");
	err = exptexture->loadFromFile("Ui_EXP.png");
	err = plant->loadFromFile("nephendes.png");
	err = pig->loadFromFile("monster1.png");
	err = angrypig->loadFromFile("Angry.png");
	err = veryangryPig->loadFromFile("VeryAngry.png");
	err = Boss1->loadFromFile("Boss1.png");
	err = PowerUp->loadFromFile("PowerUp.png");
	err = Stunned->loadFromFile("Stunned.png");
	err = Bimg->loadFromFile("button.png");
	err = Bdownimg->loadFromFile("buttondown.png");
	err = Dlgimg->loadFromFile("dialog.png");
	cout << err << endl;
	map_initialize();
	{
		sf::Vector2f dlgpos = sf::Vector2f(0, 700);
		Button* b = new Button(Bimg, Bdownimg, "yes", dlgpos + sf::Vector2f(850, 250), g_font, ButtonType::yes);
		Buttons.push_back(b);

		b = new Button(Bimg, Bdownimg, "no", dlgpos + sf::Vector2f(1000, 250), g_font, ButtonType::no);
		Buttons.push_back(b);
		
		b = new Button(Bimg, Bdownimg, "next", dlgpos + sf::Vector2f(1000, 250), g_font, ButtonType::next);
		Buttons.push_back(b);

		Dialog* d = new Dialog(Dlgimg, dlgpos, Buttons[0], Buttons[1], Buttons[2]);
		d->MakeDlg("Curious Monster : \n\rHi Sumin i am Curious Monster\n\r Plz Help me for your owner? maybe if you can i will give you little exp", DialogType::Select, g_font);
		Dialogs.push_back(d);

	}

	mLevel.setFont(g_font);
	mLevel.setColor(sf::Color(232, 188, 7));

	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 128, 0, 64, 64 };
	avatar.setBuffsTexture(*PowerUp, *Stunned);
	for (auto& pl : players) {
		pl = OBJECT{ *pieces, 0, 0, 64, 64 };
		pl.setBuffsTexture(*PowerUp, *Stunned);
	}
}

void client_finish()
{
	delete board;
	delete pieces;
	delete rocks;
	delete ui;
	delete hp;
	delete exptexture;
	delete plant;
	delete pig;
	delete angrypig;
	delete Bimg;
	delete Bdownimg;
	for (auto b : Buttons)
		delete b;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		avatar.exp = packet->exp;
		avatar.hp = packet->hp;
		avatar.maxhp = packet->hp;
		avatar.level = packet->level;
		g_x_origin = packet->x - SCREEN_WIDTH / 2;
		g_y_origin = packet->y - SCREEN_WIDTH / 2;
		avatar.move(packet->x, packet->y);
		avatar.show();
	}
	break;
	case SC_PACKET_PUT_OBJECT:
	{
		sc_packet_put_object* my_packet = reinterpret_cast<sc_packet_put_object*>(ptr);
		int id = my_packet->id;

		if (id < MAX_USER) { // PLAYER
			players[id].set_name(my_packet->name);
			players[id].move(my_packet->x, my_packet->y);
			players[id].buff = my_packet->bufftype;
			players[id].debuff = my_packet->debufftype;
			players[id].show();
		}
		else {  // NPC
			players[id].set_name(my_packet->name);
			switch (my_packet->object_type)
			{
			case OBJECT_PLANT:
				players[id].setTexture(*plant,0,0,37,64);
				break;
			case OBJECT_NORMALMONSTER:
				players[id].setTexture(*pig,0,0,64,52);
				break;
			case OBJECT_ANGRYMONSTER:
				players[id].setTexture(*angrypig,0,0,64,52);
				break;
			case OBJECT_VERYANGRYMONSTER:
				players[id].setTexture(*veryangryPig,0,0,64,52);
				break;
			case OBJECT_BOSS:
				players[id].setTexture(*Boss1, 0, 0, 58, 64);
				break;
			default:
				players[id].setTexture(*pieces,0,0,64,64);
			}
			players[id].move(my_packet->x, my_packet->y);
			players[id].show();
		}
		break;
	}
	case SC_PACKET_MOVE:
	{
		sc_packet_move* my_packet = reinterpret_cast<sc_packet_move*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_x_origin = my_packet->x - SCREEN_WIDTH / 2;
			g_y_origin = my_packet->y - SCREEN_WIDTH / 2;
		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_PACKET_REMOVE_OBJECT:
	{
		sc_packet_remove_object* my_packet = reinterpret_cast<sc_packet_remove_object*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else if (other_id < MAX_USER) {
			players[other_id].hide();
		}
		else {
			players[other_id].hide();
		}
		break;
	}
	case SC_PACKET_STATUS_CHANGE:
	{
		sc_packet_status_change* my_packet = reinterpret_cast<sc_packet_status_change*>(ptr);
		avatar.exp = my_packet->exp;
		avatar.hp = my_packet->hp;
		avatar.maxhp = my_packet->maxhp;
		avatar.level = my_packet->level;
		//cout << my_packet->maxhp << "," << my_packet->hp << "," << my_packet->exp << endl;
		break;
	}
	case SC_PACKET_CHAT:
	{
		sc_packet_chat* my_packet = reinterpret_cast<sc_packet_chat*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.set_chat(my_packet->message);
		}
		else if (other_id < MAX_USER) {
			players[other_id].set_chat(my_packet->message);
		}
		else {
			players[other_id].set_chat(my_packet->message);
		}
		break;
	}
	case SC_PACKET_LOG: {
		sc_packet_log* my_packet = reinterpret_cast<sc_packet_log*>(ptr);
		char logMsg[MAX_CHAT_SIZE];
		strcpy_s(logMsg, MAX_CHAT_SIZE, my_packet->message);
		printf("[log] %s\n", logMsg);
		break;
	}
	case SC_PACKET_IMGUPDATE: {
		sc_packet_imgupdate* my_packet = reinterpret_cast<sc_packet_imgupdate*>(ptr);
		if (my_packet->imgtype == OBJECT_VERYANGRYMONSTER)
			players[my_packet->id].setTexture(*veryangryPig,0,0,64,52);
		break;
	}
	case SC_PACKET_LOGIN_FAIL: {
		sc_packet_login_fail* my_packet = reinterpret_cast<sc_packet_login_fail*>(ptr);
		if (my_packet->reason == 0)
		{
			cout << "이미 접속하고 있는 id입니다." << endl;
			exit(-1);
		}
		else {
			cout << "어떠한 이유로 종료되었습니다." << endl;
			exit(-1);
		}
	}
	case SC_PACKET_BUFFUPDATE: {
		sc_packet_buffUpdate* my_packet = reinterpret_cast<sc_packet_buffUpdate*>(ptr);
		int other_id = my_packet->id;
		if (g_myid == other_id)
		{
			avatar.buff = my_packet->buff;
			avatar.debuff = my_packet->debuff;
		}
		else {
			cout << other_id << "변경하려나? : " << (int)my_packet->debuff << endl;
			players[other_id].buff = my_packet->buff;
			players[other_id].debuff = my_packet->debuff;
		}

		break;
	}
	case SC_PACKET_NPC: {
		sc_packet_npc* my_packet = reinterpret_cast<sc_packet_npc*>(ptr);

		cout << "npc code: " << my_packet->id << endl;
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void deleteObject()
{
	Buttons.erase(remove_if(Buttons.begin(), Buttons.end(), [](Button* t) {
		StateType isdelete = t->getSType();
		if (isdelete == StateType::deleted)
		{
			delete t;
			return true;
		}
		return false;
	}),Buttons.end());
}

bool client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result == sf::Socket::Disconnected)
	{
		wcout << L"서버 접속 종료.\n";
		return false;
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_x_origin;
			int tile_y = j + g_y_origin;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((((tile_x / 3) + (tile_y / 3)) % 2) == 1) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}
	for (auto& r : rock) r.draw();
	avatar.draw();
	for (auto& pl : players) pl.draw();
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);
	g_window->draw(text);



	sf::Sprite hpui;
	hpui.setTexture(*ui);
	hpui.setTextureRect(sf::IntRect(0, 0, 600, 193));
	hpui.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2), WINDOW_HEIGHT - hpui.getTextureRect().height);
	g_window->draw(hpui);

	int hppercent = avatar.hp * 518 / avatar.maxhp;
	int exppercent = avatar.exp * 518 / (pow(2, avatar.level) * 100);


	sf::Sprite realHp;
	realHp.setTexture(*hp);
	realHp.setTextureRect(sf::IntRect(0, 0, hppercent, 39));
	realHp.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2) + 70, WINDOW_HEIGHT - hpui.getTextureRect().height + 82);
	g_window->draw(realHp);
	sf::Text hpNum;
	hpNum.setFont(g_font);
	sprintf_s(buf, "%d / %d", avatar.hp, avatar.maxhp);
	hpNum.setString(buf);
	hpNum.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2) + 250, WINDOW_HEIGHT - hpui.getTextureRect().height + 82);
	g_window->draw(hpNum);

	sf::Sprite realExp;
	realExp.setTexture(*exptexture);
	realExp.setTextureRect(sf::IntRect(0, 0, exppercent, 39));
	realExp.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2) + 70, WINDOW_HEIGHT - hpui.getTextureRect().height + 130);
	g_window->draw(realExp);

	sf::Text expNum;
	expNum.setFont(g_font);
	int maxexp = pow(2, avatar.level) * 100;
	sprintf_s(buf, "%d / %d", avatar.exp, maxexp);
	expNum.setString(buf);
	expNum.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2) + 250, WINDOW_HEIGHT - hpui.getTextureRect().height + 130);
	g_window->draw(expNum);

	mLevel.setPosition(WINDOW_WIDTH / 2 - (hpui.getTextureRect().width / 2 - 316), WINDOW_HEIGHT - hpui.getTextureRect().height + 15);
	sprintf_s(buf, "%d", avatar.level);
	mLevel.setString(buf);
	g_window->draw(mLevel);

	for (auto& button : Buttons)
	{
		//button->Draw(*g_window);
	}

	for (auto& dlg : Dialogs)
	{
		dlg->Draw(*g_window);
	}

	deleteObject();


	return true;
}

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

int main()
{
	wcout.imbue(locale("korean"));
	string ip{ "127.0.0.1" };
	//cout << "ip 입력 :";
	//cin >> ip;
	string name{ "PL" };
	cout << "id 입력 :";
	cin >> name;
	cout << "pw 입력 :";
	string pw{ "" };
	cin >> pw;

	sf::Socket::Status status = socket.connect(ip.c_str(), SERVER_PORT);


	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();
	//auto tt = chrono::duration_cast<chrono::milliseconds>
	//	(chrono::system_clock::now().
	//		time_since_epoch()).count();
	//name += to_string(tt % 1000);

	send_login_packet(name,pw);
	avatar.set_name(name.c_str());
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			//---mouse
			switch (event.type)
			{
			case sf::Event::MouseButtonPressed: {
				for (auto& b : Buttons)
					b->checkClickButtonDown(sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window))));
				break;
			}
			case sf::Event::MouseButtonReleased: {
				for (auto& b : Buttons)
					b->checkClickButtonUp(sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window))));
				break;
			}
			case sf::Event::MouseMoved:{
				for (auto& b : Buttons)
					b->checkHoverOut(sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window))));
				break;
			}
			}


			//--keyboard
			if (event.type == sf::Event::KeyPressed) {
				int direction = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					direction = 2;
					break;
				case sf::Keyboard::Right:
					direction = 3;
					break;
				case sf::Keyboard::Up:
					direction = 0;
					break;
				case sf::Keyboard::Down:
					direction = 1;
					break;
				case sf::Keyboard::Num5:
					Buttons[0]->Destroy();
					break;
				case sf::Keyboard::A:
					send_attack_packet();
					break;
				case sf::Keyboard::Num8:
					send_npc_packet();
					break;
				case sf::Keyboard::Num9:
					send_npc_packet_response(1);
					break;
				case sf::Keyboard::Num0:
					send_npc_packet_response(0);
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
				if (-1 != direction) send_move_packet(direction);
			}
		}

		window.clear();
		if (false == client_main())
			window.close();
		window.display();
	}
	client_finish();

	return 0;
}