#pragma once

#include "os.log.h"

#include <functional>
#include <map>
#include <vector>
#include <stack>
#include <tuple>
#include <utility>

#define OBSERVABLE_LOG 0
# if OBSERVABLE_LOG
#  define OBS_LG( x , y, ...) LG( x , y ,##__VA_ARGS__)
# else
#  define OBS_LG(...) 
# endif

namespace imajuscule
{
    template <typename Event>
    struct FunctionInfo
    {
        Event m_event;
        unsigned int m_key;
    };

    template <typename Event, typename... Args>
    class Observable
    {
        enum TupleIndex
        {
            KEY = 0,
            ACTIVE,
            NTFSTAMP,
            FUNCTION
        };
        // map doesn't work when the callback modifies the map 
        // so we use a list instead : its iterators are not invalidated by insertion / removal
        typedef std::list<
            std::tuple<int /*key*/, bool /*active*/, int /*stamp*/, std::function<void(Args...) /*function*/ > 
            >
        > callbacksMap;

        typedef std::stack<int> availableKeys;
        typedef std::map<Event, std::pair<availableKeys, callbacksMap> *> observers;
        std::vector<std::pair<availableKeys, callbacksMap> *> m_allocatedPairs;

    public:
        Observable():
            m_bIsNotifying(false)
            , m_curNotifStamp(0)
        {
            //OBS_LG(INFO, "Observable::Observable()");
        }
        virtual ~Observable()
        {
            //OBS_LG(INFO, "Observable::~Observable() #%d : delete %d pairs", m_curNotifStamp, m_allocatedPairs.size());

            auto it = m_allocatedPairs.begin();
            auto end = m_allocatedPairs.end();
            for (; it != end; ++it)
            {
                delete *it;
            }
        }

        template <typename Observer>
        const FunctionInfo<Event> Register(const Event &evt, Observer&& observer)
        {
            //OBS_LG(INFO, "Observable::Register(%d) #%d", evt, m_curNotifStamp);

            std::pair<availableKeys, callbacksMap> * v;

            auto r = m_observers.find(evt);
            if (r == m_observers.end())
            {
                v = new std::pair < availableKeys, callbacksMap >();
                m_observers.insert(observers::value_type(evt, v));
                m_allocatedPairs.push_back(v);
            }
            else
            {
                v = r->second;
            }

            // take key from stack of available keys, or if it's empty, that means the next available key is the size of the map
            int key;
            if (v->first.empty())
                key = v->second.size();
            else
            {
                key = v->first.top();
                v->first.pop();
            }

            v->second.push_back(callbacksMap::value_type(key, true, 0, std::forward<Observer>(observer)));

            FunctionInfo<Event> FunctionInfo{ evt, key };
            return FunctionInfo;
        }

        void Notify(const Event &event, Args... Params)
        {
            //OBS_LG(INFO, "Observable::Notify(%d) #%d", event, m_curNotifStamp);
            auto it = m_observers.find(event);
            if ((it != m_observers.end()) && (! it->second->second.empty()))
            {
                size_t size1 = it->second->second.size();
                OBS_LG(INFO, "Observable(%x)::Notify(%d) : size0 %d", this, event, size1);

                m_notifyingEvent = event;
                m_bIsNotifying = true;
                m_curNotifStamp++;
                callbacksMap::iterator itM = it->second->second.begin();
                callbacksMap::iterator endM = it->second->second.end();

                for (; itM != endM;)
                {
                    if (std::get<ACTIVE>(*itM))
                    {
                        OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d", this, event, it->second->second.size());
                        std::get<FUNCTION>(*itM)(Params...);
                        OBS_LG(INFO, "Observable(%x)::Notify(%d) : size2 %d", this, event, it->second->second.size());

                        // set the timeStamp to allow this notification to be removed by a future notification call in this loop
                        std::get<NTFSTAMP>(*itM) = m_curNotifStamp;

                        ++itM;
                    }
                    else
                    {
                        // push the (future) removed key in stack of availableKeys
                        it->second->first.push(std::get<KEY>(*itM));
                        // erase and increment
                        itM = it->second->second.erase(itM);
                        OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d (removed a notification)", this, event, it->second->second.size());
                    }
                }

                m_bIsNotifying = false;
            }
        }

        const void Remove(const std::vector< FunctionInfo<Event> > & functionInfo)
        {
            for (auto it : functionInfo)
                Remove(it);
        }

        const void Remove(const FunctionInfo<Event> &functionInfo)
        {
            //OBS_LG(INFO, "Observable::Remove(%d) #%d", functionInfo.m_event, m_curNotifStamp);

            auto it1 = m_observers.find(functionInfo.m_event);
            if (it1 != m_observers.end())
            {
                callbacksMap::iterator it = it1->second->second.begin();
                callbacksMap::iterator end = it1->second->second.end();

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size before %d", this, functionInfo.m_event, it1->second->second.size());

                bool bFound = false;
                for (; it != end; ++it)
                {
                    if (std::get<KEY>(*it) == functionInfo.m_key)
                    {
                        bFound = true;

                        // perform delayed removal if :
                        // - this observable is currently notifying
                        // - AND the notified event is the same as functionInfo.m_event
                        // - AND the notification has not been executed yet

                        if (m_bIsNotifying && (m_notifyingEvent == functionInfo.m_event) && (std::get<NTFSTAMP>(*it) != m_curNotifStamp))
                        {
                            std::get<ACTIVE>(*it) = false;
                            OBS_LG(INFO, "Observable(%x)::Remove(%d) : tagged for removal", this, functionInfo.m_event);
                        }
                        else
                        {
                            // push the (future) removed key in stack of availableKeys
                            it1->second->first.push(std::get<KEY>(*it));
                            // erase
                            it1->second->second.erase(it);
                            OBS_LG(INFO, "Observable(%x)::Remove(%d) : size1 %d (removed a notification)", this, event, it->second->second.size());
                        }
                        break;
                    }
                }

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size after %d", this, functionInfo.m_event, it1->second->second.size());

                assert(bFound);
            }
            else
            {
                OBS_LG(ERR, "Observable::Remove : attempt to remove a registration that doesn't exist");
                assert(0);
            }
        }

        Observable(const Observable &) = delete;
        Observable &operator=(const Observable &) = delete;

    private:
        observers m_observers;
        bool m_bIsNotifying;
        Event m_notifyingEvent;
        int m_curNotifStamp;
    };
}