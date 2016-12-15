#pragma once

#include <vector>
#include <stack>
#include <limits>

#include "os.log.h"

namespace imajuscule
{
    // for vector
    template<class T, class Elem>
    void make_room_for_index(T index, std::vector<Elem> & c)
    {
        if(c.size() < index+1) {
            c.resize(index+1);
        }
    }

    // for maps, lists, etc...
    template<class T, class Container>
    void make_room_for_index(T index, Container & c)
    {
    }
    
    /*
     * To keep track of which indexes are in-use in a container.
     *
     * Works only if :
     * - the container adds its elements at the index resulting of a call to Take
     * - the indexes of removed elements are Returned by calling Return
     * - THere is no overflow
     */
    template<typename T>
    struct AvailableIndexes {
        using index_t = T;
        
        template<typename Container>
        T Take(Container & container) {
            if (!availables.empty()) {
                auto key = availables.top();
                availables.pop();
                return key;
            }
            A(container.size() < std::numeric_limits<T>::max());
            T index = static_cast<T>(container.size());
            make_room_for_index(index, container);
            return index;
        }

        void Return(T index) {
            availables.push(index);
        }
        
        auto size() {
            return availables.size();
        }
    private:
        std::stack<T> availables;
    };

}