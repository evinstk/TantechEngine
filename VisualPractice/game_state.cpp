#include "game_state.h"
#include <algorithm>

namespace te
{
    GameState::GameState()
        : mpStack(nullptr)
        , mPendingChanges()
    {}
    GameState::~GameState() {}

    void GameState::queuePop()
    {
        mPendingChanges.push_back({
            StackOp::POP,
            nullptr
        });
    }

    void GameState::queuePush(std::shared_ptr<GameState> newState)
    {
        mPendingChanges.push_back({
            StackOp::PUSH,
            newState
        });
    }

    void GameState::queueClear()
    {
        mPendingChanges.push_back({
            StackOp::CLEAR,
            nullptr
        });
    }

    void GameState::applyPendingChanges()
    {
        if (mpStack) {
            StateStack& stack = *mpStack;
            std::for_each(std::begin(mPendingChanges), std::end(mPendingChanges), [&stack, this](Change& change)
            {
                switch (change.op) {
                case StackOp::PUSH:
                    stack.push(change.state);
                    break;
                case StackOp::POP:
                    stack.popAt(this);
                    break;
                case StackOp::CLEAR:
                    stack.clear();
                    break;
                default:
                    throw std::runtime_error("Unsupported stack operation.");
                }
            });
            mPendingChanges.clear();
        }
        else {
            throw std::runtime_error("State not associated with stack.");
        }
    }

    StateStack::StateStack(std::shared_ptr<GameState> pInitialState)
        : mStack()
    {
        push(pInitialState);
    }

    void StateStack::push(std::shared_ptr<GameState> pState)
    {
        if (pState) {
            if (!pState->mpStack) {
                pState->mpStack = this;
                mStack.push_back(pState);
            }
            else {
                throw std::runtime_error("State already in a stack.");
            }
        }
        else {
            throw std::runtime_error("Must supply state to stack.");
        }
    }

    void StateStack::popAt(GameState* pState)
    {
        auto last = mStack.begin();
        for (auto it = mStack.begin(); it != mStack.end(); ++it) {
            if (it->get() == pState) {
                last = it + 1;
                continue;
            }
        }
        mStack.erase(mStack.begin(), last);
    }

    void StateStack::clear()
    {
        mStack.clear();
    }

    void StateStack::update(float dt) const
    {
        for (auto it = mStack.begin(); it != mStack.end(); ++it) {
            if (!it->get()->update(dt)) {
                continue;
            }
        }
    }

    void StateStack::draw() const
    {
        for (auto it = mStack.begin(); it != mStack.end(); ++it) {
            if (!it->get()->draw()) {
                continue;
            }
        }
    }

    void executeStack(const StateStack& stack, float dt)
    {
        stack.update(dt);
        stack.draw();
    }
}
