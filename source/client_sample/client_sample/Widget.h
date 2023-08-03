#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
enum class StateType {
    disable = 1,
    enable = 2,
    deleted = 3

};


class Widget
{

protected:

    StateType mSType;
public:

    virtual StateType getSType();
    virtual void Draw(sf::RenderWindow& window) = 0;
};

