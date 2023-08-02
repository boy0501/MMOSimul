#include "Dialog.h"
#include "Button.h"
#include <iostream>
#include <algorithm>
#include <vector>

Dialog::Dialog(sf::Texture* Board, sf::Vector2f location, Button* YesB, Button* NoB, Button* NextB) :
	YesButton(YesB),
	NoButton(NoB),
	NextButton(NextB),
	mCurrDlgNum(0)
{
	mBoard.setTexture(*Board);
	mBoard.setPosition(location);
}

Dialog::~Dialog()
{
}

void Dialog::MakeDlg(std::string str, DialogType type, sf::Font& font)
{
	YesButton->ConnectFunc(&Dialog::PushYes);
	NoButton->ConnectFunc(&Dialog::PushNo);
	NextButton->ConnectFunc(&Dialog::PushNext);

	std::pair<sf::Text*, DialogType> dlg;
	sf::Text* dt = dlg.first = new sf::Text();
	dlg.second = type;
	switch (dlg.second)
	{
	case DialogType::Normal:
		NoButton->setSType(StateType::disable);
		YesButton->setSType(StateType::disable);
		NextButton->setSType(StateType::enable);
		break;
	case DialogType::Select:
		NoButton->setSType(StateType::enable);
		YesButton->setSType(StateType::enable);
		NextButton->setSType(StateType::disable);
		break;
	}
	dt->setFont(font);
	dt->setString(str);
	dt->setPosition(mBoard.getPosition());
	dt->setFillColor(sf::Color(0, 0, 0));

	mDlg.push_back(dlg);

}

void Dialog::NextDlg()
{
	if (mCurrDlgNum != mDlg.size())
	{
		mCurrDlgNum++;
	}
	else
	{
		BeDelete = true;
	}
}

void Dialog::Draw(sf::RenderWindow& window)
{
	auto dlg = mDlg[mCurrDlgNum];
	window.draw(mBoard);
	window.draw(*dlg.first);

	switch (dlg.second)
	{
	case DialogType::Normal:
	{
		if(NextButton)
			NextButton->Draw(window);
		break;
	}
	case DialogType::Select:
	{
		if(YesButton)
			YesButton->Draw(window);
		if(NoButton)
			NoButton->Draw(window);
		break;
	}
	}

}

void Dialog::SetString(std::string& str)
{
}

void Dialog::PushYes(Button* button)
{
	
}

void Dialog::PushNo(Button* button)
{
}

void Dialog::PushNext(Button* button)
{
}

