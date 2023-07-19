#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class Button {
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string, sf::Vector2f location);
    void checkClick(sf::Vector2i);
    void setState(bool);
    void setText(std::string);
    bool getVar();
    sf::Sprite* getSprite();
    sf::Text* getText();
private:
    sf::Sprite normal;
    sf::Sprite clicked;
    sf::Sprite* currentSpr;
    sf::Text String;
    bool current;
};
