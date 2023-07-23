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

class Button {
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string, sf::Vector2f location,sf::Font& font,ButtonType type,class Dialog* dlg = nullptr);
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
    class Dialog* pDlg = nullptr; //parent
    bool current;
    ButtonType mBType;
    StateType mSType;
};
