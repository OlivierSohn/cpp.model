#pragma once

#include <functional>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include <array>
#include <tuple>
#include <memory>

#include "os.log.h"

#include "meta.h"

#define OBSERVABLE_LOG 0
# if OBSERVABLE_LOG
#  define OBS_LG( x , y, ...) LG( x , y ,##__VA_ARGS__)
# else
#  define OBS_LG(...) 
# endif

namespace imajuscule
{
    template <typename E>
    constexpr typename std::underlying_type<E>::type to_underlying(E e) {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }

    template <typename Event>
    struct FunctionInfo
    {
        Event event : ceil_power_of_two(to_underlying(Event::SIZE_ENUM));
        uint16_t key;
    };

    template <typename Event, typename... Args>
    class Observable
    {
        struct Callback {
            template <typename Observer>
            Callback(bool active, unsigned char rec, uint16_t const key, Observer && f):
            active(active), recursive_level(rec), key(key), function(std::move(f)) {}
            
            bool active : 1; // false means scheduled for removal
            unsigned char recursive_level : 3; // > 0 means being sent
            uint16_t const key;
            std::function<void(Args...)> const function;
        };
        
        // map doesn't work when the callback modifies the map 
        // so we use a list instead : its iterators are not invalidated by insertion / removal
        using callbacksList = std::list<Callback>;

        using availableKeys = std::stack<uint16_t>;
        using eventNotification = std::tuple<availableKeys, callbacksList>;
        enum EvtNtfTupleIndex
        {
            AVAILABLE_KEYS = 0,
            CBS_LIST
        };

        using observers = std::array<eventNotification, to_underlying(Event::SIZE_ENUM)>;
        
        // constructor is private, call ::instantiate instead
        Observable():
            m_deinstantiate(false),
            m_iCurNotifyCalls(0)
        {}
        
        // destructor is private, call ::deinstantiate instead
        ~Observable() = default;
        
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

            rv.reserve(evts.size());
            
            for (auto&r : evts) {
                rv.push_back(Register(r, observer));
            }

            return rv;
        }

        template <typename Observer>
        FunctionInfo<Event> Register(Event const evt, Observer&& observer)
        {
            //OBS_LG(INFO, "Observable::Register(%d) #%d", evt, m_curNotifStamp);

            eventNotification & v = m_observers[to_underlying(evt)];

            // take key from stack of available keys, or if it's empty, that means the next available key is the size of the map
            uint16_t key;
            auto & avKeys = std::get<AVAILABLE_KEYS>(v);
            auto & cbslist = std::get<CBS_LIST>(v);
            if (avKeys.empty()) {
                A(cbslist.size() < std::numeric_limits<uint16_t>::max());
                key = static_cast<int16_t>(cbslist.size());
            }
            else {
                key = avKeys.top();
                avKeys.pop();
            }
            cbslist.emplace_back(true, 0, key, std::move(observer));
            return { evt, key };
        }

        void Notify(Event const event, Args... Params)
        {
            //OBS_LG(INFO, "Observable::Notify(%d) #%d", event, m_curNotifStamp);

            auto & obs = m_observers[to_underlying(event)];
            callbacksList & cbslist = std::get<CBS_LIST>(obs);
            if (cbslist.empty()) { return; }
            
            ++m_iCurNotifyCalls;
            A(m_iCurNotifyCalls); // else type too small
            
            OBS_LG(INFO, "Observable(%x)::Notify(%d) : size0 %d", this, event, cbslist.size());
            
            auto itM = cbslist.begin();
            auto endM = cbslist.end();
            
            for (; itM != endM;)
            {
                auto & cb = *itM;
                
                if (!cb.active) {
                    // push the (future) removed key in stack of availableKeys
                    std::get<AVAILABLE_KEYS>(obs).push(cb.key);
                    // erase and increment
                    itM = cbslist.erase(itM);
                    endM = cbslist.end();
                    OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d (removed a notification)", this, event, std::get<CBS_LIST>(*(it->second)).size());
                    continue;
                }
                
                // increment recursive level, to prevent this notification from being removed immediately
                ++cb.recursive_level;
                A(cb.recursive_level); // else type too small
                
                OBS_LG(INFO, "Observable(%x)::Notify(%d) : size1 %d", this, event, std::get<CBS_LIST>(*(it->second)).size());
                
                cb.function(Params...);
                
                if (m_deinstantiate) {
                    break;
                }
                
                OBS_LG(INFO, "Observable(%x)::Notify(%d) : size2 %d", this, event, std::get<CBS_LIST>(*(it->second)).size());
                
                --cb.recursive_level;
                
                ++itM;
            }
            
            --m_iCurNotifyCalls;
            deinstantiateIfNeeded();
        }

        const void Remove(const std::vector< FunctionInfo<Event> > & functionInfo)
        {
            for (auto it : functionInfo) {
                Remove(it);
            }
        }

        const bool Remove(FunctionInfo<Event> const functionInfo)
        {
            //OBS_LG(INFO, "Observable::Remove(%d) #%d", functionInfo.m_event, m_curNotifStamp);
            
            auto & obs = m_observers[to_underlying(functionInfo.event)];
            auto & cbslist = std::get<CBS_LIST>(obs);
            
            auto it = cbslist.begin();
            auto end = cbslist.end();
            
            OBS_LG(INFO, "Observable(%x)::Remove(%d) : size before %d", this, functionInfo.m_event, cbslist.size());
            
            for (; it != end; ++it)
            {
                auto & cb = *it;
                if (cb.key != functionInfo.key) { continue; }
                
                //LG(INFO, "  %d", std::get<KEY>(*it));
                
                if (cb.recursive_level > 0) {
                    // notification is being sent, we cannot delete it now.
                    cb.active = false;
                    OBS_LG(INFO, "Observable(%x)::Remove(%d) : tagged for removal", this, functionInfo.event);
                }
                else {
                    // push the (future) removed key in stack of availableKeys
                    std::get<AVAILABLE_KEYS>(obs).push(cb.key);
                    // erase
                    cbslist.erase(it);
                    OBS_LG(INFO, "Observable(%x)::Remove(%d) : size1 %d (removed a notification)", this, functionInfo.event, cbslist.size());
                }
                return true;
            }
            
            OBS_LG(INFO, "Observable(%x)::Remove(%d) : size after %d", this, functionInfo.m_event, cbslist.size());
            
            return false;
        }

        Observable(const Observable &) = delete;
        Observable &operator=(const Observable &) = delete;

    private:
        bool m_deinstantiate : 1;
        observers m_observers;
        unsigned int m_iCurNotifyCalls : 4;

        void deinstantiateIfNeeded()
        {
            if (m_deinstantiate && (0 == m_iCurNotifyCalls)) {
                delete this;
            }
        }
    };
}
