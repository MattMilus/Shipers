#include "TextField.h"

#include <utility>

namespace {
sf::Font textFieldFont("assets/fonts/Arial.ttf");
}

TextField::TextField()
    : label(textFieldFont),
      valueText(textFieldFont),
      placeholderText(textFieldFont),
      maxLength(31),
      focused(false) {
    background.setFillColor(sf::Color(245, 248, 252, 245));
    background.setOutlineThickness(2.0f);
    background.setSize({240.0f, 46.0f});

    label.setCharacterSize(16);
    label.setFillColor(sf::Color(220, 228, 239));

    valueText.setCharacterSize(20);
    valueText.setFillColor(sf::Color(20, 35, 55));

    placeholderText.setCharacterSize(20);
    placeholderText.setFillColor(sf::Color(110, 125, 145));

    updateVisualState();
    updateLayout();
}

void TextField::setPosition(const sf::Vector2f& position) {
    background.setPosition(position);
    updateLayout();
}

void TextField::setSize(const sf::Vector2f& size) {
    background.setSize(size);
    updateLayout();
}

void TextField::setLabel(const std::string& text) {
    label.setString(text);
    updateLayout();
}

void TextField::setPlaceholder(const std::string& text) {
    placeholderText.setString(text);
    updateLayout();
}

void TextField::setValue(std::string text) {
    if (text.size() > maxLength) {
        text.resize(maxLength);
    }

    value = std::move(text);
    valueText.setString(value);
    updateLayout();
}

void TextField::setMaxLength(const std::size_t length) {
    maxLength = length;
    if (value.size() > maxLength) {
        value.resize(maxLength);
        valueText.setString(value);
        updateLayout();
    }
}

const std::string& TextField::getValue() const {
    return value;
}

void TextField::setFocused(const bool shouldFocus) {
    focused = shouldFocus;
    updateVisualState();
}

bool TextField::isFocused() const {
    return focused;
}

bool TextField::contains(const sf::Vector2f& point) const {
    return background.getGlobalBounds().contains(point);
}

void TextField::handleTextEntered(const char32_t unicode) {
    if (unicode == U'\b') {
        if (!value.empty()) {
            value.pop_back();
            valueText.setString(value);
            updateLayout();
        }
        return;
    }

    if (unicode < 32 || unicode > 126 || value.size() >= maxLength) {
        return;
    }

    value.push_back(static_cast<char>(unicode));
    valueText.setString(value);
    updateLayout();
}

void TextField::draw(sf::RenderWindow& window) const {
    window.draw(label);
    window.draw(background);

    if (value.empty()) {
        window.draw(placeholderText);
    } else {
        window.draw(valueText);
    }
}

void TextField::updateLayout() {
    const sf::Vector2f position = background.getPosition();

    label.setPosition({position.x, position.y - 28.0f});
    valueText.setPosition({position.x + 14.0f, position.y + 8.0f});
    placeholderText.setPosition({position.x + 14.0f, position.y + 8.0f});
}

void TextField::updateVisualState() {
    background.setOutlineColor(focused ? sf::Color(47, 109, 214) : sf::Color(170, 188, 210));
}
