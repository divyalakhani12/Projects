#ifndef SCREENS_HPP
#define SCREENS_HPP

#include <SFML/Graphics.hpp>

bool runWelcomeScreen(sf::RenderWindow &window);
std::string runLoginScreen(sf::RenderWindow &window);
void runTransportScreen(sf::RenderWindow &window);
void runAdminScreen(sf::RenderWindow &window);

#endif
