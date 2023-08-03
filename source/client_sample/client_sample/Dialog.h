#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <unordered_map>
#include "Widget.h"

extern std::unordered_map<int,class Dialog*> m_npcdlg;

enum class DialogType{
	Normal = 1,
	Select = 2
};

class Dialog : public Widget
{
public:
	Dialog(sf::Texture* Board, sf::Vector2f location, int npc_id, class Button* YesB, class Button* NoB, class Button* NextB);
	~Dialog();

	virtual void Draw(sf::RenderWindow& window) override;

	void MakeDlg(std::string str, DialogType type, sf::Font& font);
	void AddDlg(std::string str, DialogType type, sf::Font& font);
	void NextDlg();
	void DeleteDlg();
	void SetString(std::string& str);

	static void PushYes(class Button* button);
	static void PushNo(class Button* button);
	static void PushNext(class Button* button);
private:

	//DialogType mType; // 1 - Normal Dialog, 2 - select Dialog
	sf::Sprite mBoard;
	class Button* YesButton;
	class Button* NoButton;
	class Button* NextButton;
	std::vector<std::pair<sf::Text*, DialogType>> mDlg;
	int mCurrDlgNum;
	int npc_id;	//어떤npc에 종속되어있는지.

};

