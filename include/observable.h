#pragma once

//inspired from http://www.codereversing.com/blog/?p=137

#include "os.log.h"

#include <functional>
#include <map>
#include <vector>
#include <utility>

namespace imajuscule
{
    template <typename Event>
    struct FunctionInfo
    {
        Event m_event;
        unsigned int m_vectorIndex;
    };

    template <typename Event, typename... Args>
    class Observable
    {
        typedef std::map<Event, std::vector<std::function<void(Args...)>>> observers;

    public:
        Observable() = default;
        virtual ~Observable() = default;

        template <typename Observer>
        const FunctionInfo<Event> Register(const Event &event, Observer &&observer)
        {
            auto & v = m_observers[event];
            v.push_back(std::forward<Observer>(observer));

            FunctionInfo<Event> FunctionInfo{ event, v.size() - 1 };
            return FunctionInfo;
        }

        template <typename Observer>
        const FunctionInfo<Event> Register(const Event &&event, Observer &&observer)
        {
            auto & v = m_observers[std::move(event)];
            v.push_back(std::forward<Observer>(observer));

            FunctionInfo<Event> FunctionInfo{ event, v.size() - 1 };
            return FunctionInfo;
        }

        void Notify(const Event &event, Args... Params) const
        {
            auto it = m_observers.find(event);
            if (it != m_observers.end())
            {
                for (const auto &observer : it->second)
                {
                    observer(Params...);
                }
            }
        }

        const void Remove(const FunctionInfo<Event> &functionInfo)
        {
            auto it = m_observers.find(functionInfo.m_event);
            if (it != m_observers.end())
            {
                it->second.erase(it->second.begin() + functionInfo.m_vectorIndex);
            }
            else
            {
                LG(ERR, "Observable::Remove : attempt to remove a registration that doesn't exist");
                assert(0);
            }
        }

        Observable(const Observable &) = delete;
        Observable &operator=(const Observable &) = delete;

    private:
        observers m_observers;

    };
}