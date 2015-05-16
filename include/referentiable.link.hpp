#include "referentiable.h"

using namespace imajuscule;

template<class T>
RefLink<T>::RefLink(Referentiable& source, T *target) :
m_source(source),
m_target(NULL)
{
    set(target);
}

template<class T>
RefLink<T>::RefLink(RefLink<T> && r) :
m_source(r.m_source),
m_target(r.m_target)
{}

template<class T>
RefLink<T>::~RefLink()
{
    set(NULL);
}

template<class T>
RefLink<T>::operator T*()
{
    return get();
}
template<class T>
RefLink<T>::operator const T*() const
{
    return get();
}
template<class T>
T* RefLink<T>::operator->()
{
    return get();
}
template<class T>
const T* RefLink<T>::operator->() const
{
    return get();
}

template<class T>
T& RefLink<T>::operator*()
{
    return *get();
}
template<class T>
const T& RefLink<T>::operator*() const
{
    return *get();
}
template<class T>
auto RefLink<T>::operator= ( RefLink<T> & other) -> RefLink &
{
    set(other.get());
    return *this;
}
template<class T>
auto RefLink<T>::operator= (T * pointer) -> RefLink &
{
    set(pointer);
    return *this;
}

template<class T>
void RefLink<T>::set(T * target)
{
    if(target == m_target)
        return;
    
    if(m_target)
    {
        m_target->unRegisterSource(m_source);
        m_source.unRegisterTarget(*m_target);
    }
    m_target = target;
    if(m_target)
    {
        m_source.registerTarget(*m_target);
        m_target->registerSource(m_source);
    }
}

template<class T>
const T * RefLink<T>::get() const
{
    return m_target;
}
template<class T>
T * RefLink<T>::getConst() const
{
    return m_target;
}

template<class T>
T * RefLink<T>::get()
{
    return m_target;
}
