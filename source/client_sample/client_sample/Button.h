#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Widget.h"
enum class ButtonType {
    next = 1,
    yes = 2,
    no = 3
};

class Button;
typedef void (*bhandler)(Button*);

class Button :public Widget 
{
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string, sf::Vector2f location,sf::Font& font,ButtonType type, class Dialog* dlg = nullptr);
    ~Button();


    void Destroy();
    virtual void Draw(sf::RenderWindow& window) override;

    void checkClickButtonDown(const sf::Vector2i&);
    void checkClickButtonUp(const sf::Vector2i&);
    void checkHoverOut(const sf::Vector2i&);

    void setState(bool);
    void setText(std::string);
    void setFont(sf::Font font);
    void setSType(StateType state);
    void setParent(class Dialog* dlg) { pDlg = dlg; }

    void ConnectFunc(bhandler func);



    bool getVar();
    sf::Sprite* getSprite();
    sf::Text* getText();
    class Dialog* getParent() { return pDlg; }
private:
    void ButtonPush();
    sf::Sprite normal;
    sf::Sprite clicked;
    sf::Sprite* currentSpr;
    sf::Text String;
    class Dialog* pDlg = nullptr; //parent 원래는 widget의 최 상단을 가리켜야하지만.. 임시로 만든 클라프로그램이므로
    bool current;
    ButtonType mBType;
    bhandler OnPress;
};
