#pragma once
#include <Arduino.h>
/// @brief Небольшая библиотека для удобного управления модулем переключателя (реле)
class Relay
{
private:
    byte _pin;
    bool _truth, _state;
    void _setState(bool state);

public:
    Relay(byte pin, bool truth = 0);
    ~Relay();
    bool set(bool state);
    bool getState();
    void change();
};

/// @brief Конструктор класса реле
/// @param pin номер пина управления реле
/// @param truth Какой сигнал является сигналом включения 0 или 1 default = 0
Relay::Relay(byte pin, bool truth)
{
    pinMode(pin, OUTPUT);
    _pin = pin;
    _truth = truth;
    digitalWrite(_pin, !_truth);
    _state = !_truth;
}

Relay::~Relay()
{
}

/// @brief Управление реле
/// @param state true - включить. false - выключить
/// @return Возвращает true если произошло изменение состояния
bool Relay::set(bool state)
{
    if (getState() != state)
    {
        _setState(state);
        digitalWrite(_pin, _state);
        return true;
    }
    return false;
}
/// @brief Узнать состояние реле
/// @return true - включено, false - выключено
bool Relay::getState()
{
    return _state ^ !_truth;
}

/// @brief Переключить реле
void Relay::change()
{
    _setState(!getState());
    digitalWrite(_pin, _state);
}

void Relay::_setState(bool state)
{
    _state = state ^ !_truth;
}
