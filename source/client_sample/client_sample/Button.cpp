#include "Button.h"
#include <iostream>
Button::Button(sf::Texture* normal, sf::Texture* clicked, std::string words, sf::Vector2f location) {
    this->normal.setTexture(*normal);
    this->clicked.setTexture(*clicked);
    this->currentSpr = &this->normal;
    current = false;
    this->normal.setPosition(location);
    this->clicked.setPosition(location);
    String.setString(words);
    String.setPosition(location.x + 3, location.y + 3);
    String.setCharacterSize(14);
}
void Button::checkClick(sf::Vector2i mousePos) {
    if (mousePos.x > currentSpr->getPosition().x && mousePos.x < (currentSpr->getPosition().x + currentSpr->getTextureRect().width)) {
        if (mousePos.y > currentSpr->getPosition().y && mousePos.y < (currentSpr->getPosition().y + currentSpr->getTextureRect().height)) {
            setState(!current);
            std::cout << " clicked \n";
        }
    }
}
void Button::setState(bool which) {
    current = which;
    if (current) {
        currentSpr = &clicked;
        return;
    }
    currentSpr = &normal;
}
void Button::setText(std::string words) {
    String.setString(words);
}
bool Button::getVar() {
    return current;
}
sf::Sprite* Button::getSprite() {
    return currentSpr;
}

sf::Text* Button::getText() {
    return &String;
}