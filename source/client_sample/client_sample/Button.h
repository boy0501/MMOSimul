#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
enum class ButtonType {
    next = 1,
    yes = 2,
    no = 3
};

enum class StateType {
    disable = 1,
    enable = 2,
    deleted = 3
    
};

class Button;
typedef void (*bhandler)(Button*);

class Button {
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string, sf::Vector2f location,sf::Font& font,ButtonType type, class Dialog* dlg = nullptr);
    ~Button();


    void Destroy();
    void Draw(sf::RenderWindow& window);

    void checkClickButtonDown(const sf::Vector2i&);
    void checkClickButtonUp(const sf::Vector2i&);
    void checkHoverOut(const sf::Vector2i&);

    void setState(bool);
    void setText(std::string);
    void setFont(sf::Font font);
    void setSType(StateType state);

    void ConnectFunc(bhandler func);



    bool getVar();
    StateType getSType();
    sf::Sprite* getSprite();
    sf::Text* getText();

private:
    void ButtonPush();
    sf::Sprite normal;
    sf::Sprite clicked;
    sf::Sprite* currentSpr;
    sf::Text String;
    class Dialog* pDlg = nullptr; //parent 원래는 widget의 최 상단을 가리켜야하지만.. 임시로 만든 클라프로그램이므로
    bool current;
    ButtonType mBType;
    StateType mSType;
    bhandler OnPress;
};
