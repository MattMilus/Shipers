#include "Panel.h"
#include <cstdarg>


sf::Font arial("assets/fonts/Arial.ttf");


Panel::Panel() : text(arial), clickedText(arial) {
}

void Panel::setFontSize(unsigned int size) {
    text.setCharacterSize(size);
}

void Panel::setText(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    text.setString(buffer);
}

void Panel::setHeldText(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    clickedText.setString(buffer);
}

void Panel::draw(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(text);
}
