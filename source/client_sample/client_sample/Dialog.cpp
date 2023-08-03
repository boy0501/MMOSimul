#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include "Dialog.h"
#include "Button.h"
#include "Network.h"

std::unordered_map<int, Dialog*> m_npcdlg;

Dialog::Dialog(sf::Texture* Board, sf::Vector2f location,int npc_id, Button* YesB, Button* NoB, Button* NextB) :
	YesButton(YesB),
	NoButton(NoB),
	NextButton(NextB),
	mCurrDlgNum(0),
	npc_id(npc_id)
{
	mSType = StateType::disable;
	mBoard.setTexture(*Board);
	mBoard.setPosition(location);
}

Dialog::~Dialog()
{
}

void Dialog::MakeDlg(std::string str, DialogType type, sf::Font& font)
{
	YesButton->ConnectFunc(&Dialog::PushYes);
	YesButton->setParent(this);
	NoButton->ConnectFunc(&Dialog::PushNo);
	NoButton->setParent(this);
	NextButton->ConnectFunc(&Dialog::PushNext);
	NextButton->setParent(this);

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

void Dialog::AddDlg(std::string str, DialogType type, sf::Font& font)
{
	std::pair<sf::Text*, DialogType> dlg;
	sf::Text* dt = dlg.first = new sf::Text();
	dlg.second = type;
	dt->setFont(font);
	dt->setString(str);
	dt->setPosition(mBoard.getPosition());
	dt->setFillColor(sf::Color(0, 0, 0));

	mDlg.push_back(dlg);
}

void Dialog::NextDlg()
{
	if (mCurrDlgNum != mDlg.size() - 1)
	{
		mCurrDlgNum++;
		auto dlg = mDlg[mCurrDlgNum];
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
	}
	else
	{
		DeleteDlg();
	}
}

void Dialog::DeleteDlg()
{
	mSType = StateType::deleted;
	if (YesButton)
		YesButton->Destroy();
	if (NoButton)
		NoButton->Destroy();
	if (NextButton)
		NextButton->Destroy();
	m_npcdlg[npc_id] = nullptr;
}

void Dialog::Draw(sf::RenderWindow& window)
{
	auto dlg = mDlg[mCurrDlgNum];
	auto text = dlg.first;
	text->setPosition(mBoard.getPosition() + sf::Vector2f(30, 20));
	window.draw(mBoard);
	window.draw(*text);

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
	auto dlg = button->getParent();
	//서버로 보내기

	send_npc_packet_response(1);
	if (dlg != nullptr)
		dlg->NextDlg();
}

void Dialog::PushNo(Button* button)
{
	auto dlg = button->getParent();
	//서버로 보내기

	send_npc_packet_response(0);
	if (dlg != nullptr)
		dlg->NextDlg();
}

void Dialog::PushNext(Button* button)
{
	auto dlg = button->getParent();
	//서버로 보내기

	if (dlg != nullptr)
		dlg->NextDlg();
}

