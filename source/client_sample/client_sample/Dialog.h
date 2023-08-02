#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

enum class DialogType{
	Normal = 1,
	Select = 2
};

class Dialog 
{
public:
	Dialog(sf::Texture* Board, sf::Vector2f location,class Button* YesB, class Button* NoB, class Button* NextB);
	~Dialog();

	void MakeDlg(std::string str, DialogType type, sf::Font& font);
	void NextDlg();
	void Draw(sf::RenderWindow& window);
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
	bool BeDelete = false;

};

