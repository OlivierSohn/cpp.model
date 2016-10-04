#pragma once

#include <functional>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include <tuple>
#include "os.log.h"

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
        int m_key;
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
            int /*key*/,
            bool /*active (false means scheduled for removal)*/, 
            int /*recursive level (>0 means being sent)*/,
            std::function<void(Args...) /*function*/ > > > callbacksList;

        typedef std::stack<int> availableKeys;
        enum EvtNtfTupleIndex
        {
            AVAILABLE_KEYS = 0,
            CBS_LIST
        };
        typedef std::tuple<availableKeys, callbacksList> eventNotification;
        typedef std::map<Event, eventNotification *> observers;
        std::vector<std::unique_ptr<eventNotification>> m_allocatedPairs;

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
        }

    public:
        static Observable<Event, Args...> * instantiate()
        {
            return new Observable<Event, Args...>();
        }
        // this is publicly provided instead of destructor because it's not safe to delete
        // an observer when it is in one or multiple Notify calls
        void deinstantiate()
        {
            A(!m_deinstantiate);
            m_deinstantiate = true;
            deinstantiateIfNeeded();
        }

        template <typename Observer>
        std::vector<FunctionInfo<Event>> Register(std::vector<Event>&& evts, Observer&& observer)
        {
            return Register(evts, observer);
        }
        template <typename Observer>
        std::vector<FunctionInfo<Event>> Register(const std::vector<Event> & evts, Observer&& observer)
        {
            std::vector<FunctionInfo<Event>> rv;

            for (auto&r : evts)
                rv.push_back(Register(r, observer));

            return rv;
        }

        template <typename Observer>
        FunctionInfo<Event> Register(const Event &evt, Observer&& observer)
        {
                //OBS_LG(INFO, "Observable::Register(%d) #%d", evt, m_curNotifStamp);

            eventNotification * v;

            auto r = m_observers.find(evt);
            if (r == m_observers.end())
            {
                v = new eventNotification();
                m_observers.insert(typename observers::value_type(evt, v));
                m_allocatedPairs.emplace_back(std::unique_ptr<eventNotification>(v));
            }
            else
            {
                v = r->second;
            }

            // take key from stack of available keys, or if it's empty, that means the next available key is the size of the map
            int key;
            availableKeys & avKeys = std::get<AVAILABLE_KEYS>(*v);
            callbacksList & cbslist = std::get<CBS_LIST>(*v);
            if (avKeys.empty())
                key = (int)cbslist.size();
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

                    OBS_LG(INFO, "Observable(%x)::Notify(%d) : size0 %d", this, event, cbslist.size());

                    typename callbacksList::iterator itM = cbslist.begin();
                    typename callbacksList::iterator endM = cbslist.end();

                    for (; itM != endM;)
                    {
                        if (std::get<ACTIVE>(*itM))
                        {
                            // increment recursive level, to prevent this notification from being removed immediately
                            std::get<RECURSIVE_LEVEL>(*itM)++;

                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d", this, event, std::get<CBS_LIST>(*(it->second)).size());
                            
                            std::get<FUNCTION>(*itM)(Params...);
                            
                            if (m_deinstantiate)
                                break;

                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size2 %d", this, event, std::get<CBS_LIST>(*(it->second)).size());

                            std::get<RECURSIVE_LEVEL>(*itM)--;

                            ++itM;
                        }
                        else
                        {
                            // push the (future) removed key in stack of availableKeys
                            std::get<AVAILABLE_KEYS>(*(it->second)).push(std::get<KEY>(*itM));
                            // erase and increment
                            itM = cbslist.erase(itM);
                            endM = cbslist.end();
                            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d (removed a notification)", this, event, std::get<CBS_LIST>(*(it->second)).size());
                        }
                    }

                    m_iCurNotifyCalls--;
                    deinstantiateIfNeeded();
                }
            }
        }

        const void Remove(const std::vector< FunctionInfo<Event> > & functionInfo)
        {
            for (auto it : functionInfo) {
                Remove(it);
            }
        }

        const void Remove(const FunctionInfo<Event> &functionInfo)
        {
            //OBS_LG(INFO, "Observable::Remove(%d) #%d", functionInfo.m_event, m_curNotifStamp);

            auto it1 = m_observers.find(functionInfo.m_event);
            if (likely(it1 != m_observers.end()))
            {
                callbacksList & cbslist = std::get<CBS_LIST>(*(it1->second));
 
                typename callbacksList::iterator it = cbslist.begin();
                typename callbacksList::iterator end = cbslist.end();

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size before %d", this, functionInfo.m_event, std::get<CBS_LIST>(*(it1->second)).size());

                bool bFound = false;
                for (; it != end; ++it)
                {
                    if (std::get<KEY>(*it) == functionInfo.m_key)
                    {
                        bFound = true;
                        //LG(INFO, "  %d", std::get<KEY>(*it));

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
                            OBS_LG(INFO, "Observable(%x)::Remove(%d) : size1 %d (removed a notification)", this, functionInfo.m_event, std::get<CBS_LIST>(*(it1->second)).size());
                        }
                        break;
                    }
                }

                OBS_LG(INFO, "Observable(%x)::Remove(%d) : size after %d", this, functionInfo.m_event, std::get<CBS_LIST>(*(it1->second)).size());

                if (unlikely(!bFound))
                {
#ifndef NDEBUG
                    LG(ERR, "key %d not found. list of present keys : ", functionInfo.m_key);
                    for(auto const & cb : std::get<CBS_LIST>(*(it1->second)) ) {
                        LG(ERR, "  %d", std::get<KEY>(cb));
                    }
#endif
                    A(!"attempt to remove a registration that is not here");
                }
            }
            else
            {
                A(!"attempt to remove a registration that doesn't exist");
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
