#pragma once

#include "os.log.h"

#include <functional>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include <tuple>
#include <cassert>

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
            RECURSIVE_LEVEL,
            FUNCTION
        };
        // map doesn't work when the callback modifies the map 
        // so we use a list instead : its iterators are not invalidated by insertion / removal
        typedef std::list< std::tuple<
            unsigned int /*key*/, 
            bool /*active (false means scheduled for removal)*/, 
            int /*recursive level (>0 means being sent)*/,
            std::function<void(Args...) /*function*/ > > > callbacksList;

        typedef std::stack<unsigned int> availableKeys;
        enum EvtNtfTupleIndex
        {
            AVAILABLE_KEYS = 0,
            CBS_LIST
        };
        typedef std::tuple<availableKeys, callbacksList> eventNotification;
        typedef std::map<Event, eventNotification *> observers;
        std::vector<eventNotification*> m_allocatedPairs;

        // constructor is private, please call ::instantiate instead
        Observable():
            m_deinstantiate(false),
            m_iCurNotifyCalls(0)
        {
            //OBS_LG(INFO, "Observable::Observable()");
        }
        // destructor is private, please call ::deinstantiate instead
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

    public:
        static Observable<Event, Args...> * instantiate()
        {
            return new Observable<Event, Args...>();
        }
        // this is publically provided instead of destructor because it's not safe to delete
        // an observer when it is in one or multiple Notify calls
        void deinstantiate()
        {
            assert(!m_deinstantiate);
            m_deinstantiate = true;
            deinstantiateIfNeeded();
        }

        template <typename Observer>
        void Register(std::vector<Event>&& evts, Observer&& observer)
        {
            Register(evts, observer);
        }
        template <typename Observer>
        void Register(const std::vector<Event> & evts, Observer&& observer)
        {
            for (auto&r : evts)
                Register(r, observer);
        }

        template <typename Observer>
        const FunctionInfo<Event> Register(const Event &evt, Observer&& observer)
        {
                //OBS_LG(INFO, "Observable::Register(%d) #%d", evt, m_curNotifStamp);

            eventNotification * v;

            auto r = m_observers.find(evt);
            if (r == m_observers.end())
            {
                v = new eventNotification();
                m_observers.insert(typename observers::value_type(evt, v));
                m_allocatedPairs.push_back(v);
            }
            else
            {
                v = r->second;
            }

            // take key from stack of available keys, or if it's empty, that means the next available key is the size of the map
            unsigned int key;
            availableKeys & avKeys = std::get<AVAILABLE_KEYS>(*v);
            callbacksList & cbslist = std::get<CBS_LIST>(*v);
            if (avKeys.empty())
                key = cbslist.size();
            else
            {
                key = avKeys.top();
                avKeys.pop();
            }

            cbslist.push_back(typename callbacksList::value_type(key, true, 0, std::forward<Observer>(observer)));

            FunctionInfo<Event> FunctionInfo{ evt, key };
            return FunctionInfo;
        }

        void Notify(const Event &event, Args... Params)
        {
            //OBS_LG(INFO, "Observable::Notify(%d) #%d", event, m_curNotifStamp);
            auto it = m_observers.find(event);
            if (it != m_observers.end())
            {
                callbacksList & cbslist = std::get<CBS_LIST>(*(it->second));
                if (!cbslist.empty())
                {
                    m_iCurNotifyCalls++;

                    size_t size1 = cbslist.size();
                    OBS_LG(INFO, "Observable(%x)::Notify(%d) : size0 %d", this, event, size1);

                    typename callbacksList::iterator itM = cbslist.begin();
                    typename callbacksList::iterator endM = cbslist.end();

                    for (; itM != endM;)
                    {
                        if (std::get<ACTIVE>(*itM))
                        {
                            // increment recursive level, to prevent this notification from being removed immediately
                            std::get<RECURSIVE_LEVEL>(*itM)++;

                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d", this, event, it->second->second.size());
                            
                            std::get<FUNCTION>(*itM)(Params...);
                            
                            if (m_deinstantiate)
                                break;

                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size2 %d", this, event, it->second->second.size());

                            std::get<RECURSIVE_LEVEL>(*itM)--;

                            ++itM;
                        }
                        else
                        {
                            // push the (future) removed key in stack of availableKeys
                            std::get<AVAILABLE_KEYS>(*(it->second)).push(std::get<KEY>(*itM));
                            // erase and increment
                            itM = cbslist.erase(itM);
                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d (removed a notification)", this, event, it->second->second.size());
                        }
                    }

                    m_iCurNotifyCalls--;
                    deinstantiateIfNeeded();
                }
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
                callbacksList & cbslist = std::get<CBS_LIST>(*(it1->second));
 
                typename callbacksList::iterator it = cbslist.begin();
                typename callbacksList::iterator end = cbslist.end();

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size before %d", this, functionInfo.m_event, it1->second->second.size());

                bool bFound = false;
                for (; it != end; ++it)
                {
                    if (std::get<KEY>(*it) == functionInfo.m_key)
                    {
                        bFound = true;

                        if (std::get<RECURSIVE_LEVEL>(*it) > 0)
                        {
                            // notification is being sent, we cannot delete it now.
                            std::get<ACTIVE>(*it) = false;
                            OBS_LG(INFO, "Observable(%x)::Remove(%d) : tagged for removal", this, functionInfo.m_event);
                        }
                        else
                        {
                            // push the (future) removed key in stack of availableKeys
                            std::get<AVAILABLE_KEYS>(*(it1->second)).push(std::get<KEY>(*it));
                            // erase
                            cbslist.erase(it);
                            OBS_LG(INFO, "Observable(%x)::Remove(%d) : size1 %d (removed a notification)", this, event, it->second->second.size());
                        }
                        break;
                    }
                }

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size after %d", this, functionInfo.m_event, it1->second->second.size());

                if (!bFound)
                {
                    LG(ERR, "Observable::Remove : attempt to remove a registration that is not here");
                    assert(0);
                }
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
        bool m_deinstantiate;
        int m_iCurNotifyCalls;

        void deinstantiateIfNeeded()
        {
            if (m_deinstantiate && (0 == m_iCurNotifyCalls))
                delete this;
        }
    };
}