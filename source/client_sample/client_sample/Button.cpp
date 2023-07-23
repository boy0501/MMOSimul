#include "Button.h"
#include "Dialog.h"
#include <iostream>
Button::Button(sf::Texture* normal, sf::Texture* clicked, std::string words, sf::Vector2f location, sf::Font& font,ButtonType type,Dialog* dlg): 
    mBType(type),
    mSType(StateType::disable),
    pDlg(dlg)
{
    this->normal.setTexture(*normal);
    this->normal.setScale(0.2, 0.1);
    this->normal.setPosition(location);
    this->clicked.setTexture(*clicked);
    this->clicked.setScale(0.2,0.1);
    this->clicked.setPosition(location);
    this->currentSpr = &this->normal;
    current = false;
    String.setFont(font);
    String.setString(words);
    String.setPosition(location.x + 20, location.y + 3);
    String.setFillColor(sf::Color(0, 0, 0));
    String.setCharacterSize(25);
}

Button::~Button()
{
}

void Button::Destroy()
{
    mSType = StateType::deleted;
}
void Button::Draw(sf::RenderWindow& window)
{
    if (mSType != StateType::enable) return;
    window.draw(*currentSpr);
    window.draw(String);
}

void Button::checkClickButtonDown(const sf::Vector2i& mousePos) {
    if (mSType != StateType::enable) return;
    if (mousePos.x > currentSpr->getPosition().x && mousePos.x < (currentSpr->getPosition().x + currentSpr->getTextureRect().width * currentSpr->getScale().x)) {
        if (mousePos.y > currentSpr->getPosition().y && mousePos.y < (currentSpr->getPosition().y + currentSpr->getTextureRect().height * currentSpr->getScale().y)) {
            setState(true);
        }
    }
}
void Button::checkClickButtonUp(const sf::Vector2i& mousePos)
{
    if (mSType != StateType::enable) return;
    if (mousePos.x > currentSpr->getPosition().x && mousePos.x < (currentSpr->getPosition().x + currentSpr->getTextureRect().width)) {
        if (mousePos.y > currentSpr->getPosition().y && mousePos.y < (currentSpr->getPosition().y + currentSpr->getTextureRect().height)) {
            setState(false);
            ButtonPush();
        }
    }
}
void Button::checkHoverOut(const sf::Vector2i& mousePos)
{
    if (!current) return;
    if (mousePos.x > currentSpr->getPosition().x && mousePos.x < (currentSpr->getPosition().x + currentSpr->getTextureRect().width)) {
        if (mousePos.y > currentSpr->getPosition().y && mousePos.y < (currentSpr->getPosition().y + currentSpr->getTextureRect().height)) {
            return;
        }
    }

    setState(false);
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
void Button::setFont(sf::Font font)
{
    String.setFont(font);
}
void Button::setSType(StateType state)
{
    mSType = state;
}
bool Button::getVar() {
    return current;
}
StateType Button::getSType()
{
    return mSType;
}
sf::Sprite* Button::getSprite() {
    return currentSpr;
}

sf::Text* Button::getText() {
    return &String;
}

void Button::ButtonPush()
{
    switch (mBType)
    {
    case ButtonType::next:
    {
        break;
    }
    case ButtonType::yes:
    {
        break;
    }
    case ButtonType::no:
    {
        break;
    }
    }

    if (pDlg != nullptr)
        pDlg->NextDlg();

}
