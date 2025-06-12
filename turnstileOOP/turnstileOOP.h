#pragma once

#include <fmt/base.h>
#include <memory>
#include <stdexcept>

enum class Event { Coin, Push };

struct IState {
    IState(struct StateMachine& fsm) : fsm_{fsm} {}
    virtual ~IState() = default;
    virtual std::unique_ptr<IState> transition(Event) = 0;

  protected:
    StateMachine& fsm_;
};

struct Locked : IState {
    using IState::IState;
    std::unique_ptr<IState> transition(Event evt) override;
    static std::unique_ptr<IState> create(StateMachine& fsm);
};

struct Unlocked : IState {
    using IState::IState;
    std::unique_ptr<IState> transition(Event evt) override;
    static std::unique_ptr<IState> create(StateMachine& fsm);
};

struct StateMachine {
  private:
    std::unique_ptr<IState> state_;

  public:
    StateMachine() : state_{Locked::create(*this)} {};
    void next(Event evt);
};