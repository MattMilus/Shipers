#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <string>

class TextField {
private:
    sf::RectangleShape background;
    sf::Text label;
    sf::Text valueText;
    sf::Text placeholderText;
    std::string value;
    std::size_t maxLength;
    bool focused;

    void updateLayout();
    void updateVisualState();

public:
    TextField();

    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    void setLabel(const std::string& text);
    void setPlaceholder(const std::string& text);
    void setValue(std::string text);
    void setMaxLength(std::size_t length);
    const std::string& getValue() const;
    void setFocused(bool shouldFocus);
    bool isFocused() const;
    bool contains(const sf::Vector2f& point) const;
    void handleTextEntered(char32_t unicode);
    void draw(sf::RenderWindow& window) const;
};

#endif // TEXTFIELD_H
