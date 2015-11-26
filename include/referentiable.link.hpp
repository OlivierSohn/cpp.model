using namespace imajuscule;

template<class T>
RefLink<T>::RefLink(Referentiable& source, T *target) :
m_source(source)
,m_target(NULL)
,m_bActive(true)
,m_bTargetIsUp(true)
{
    set(target);
}

template<class T>
RefLink<T>::RefLink(RefLink<T> && r) :
m_source(r.m_source)
,m_target(r.m_target)
,m_bActive(true)
,m_bTargetIsUp(true)
{
    //LG(INFO,"RefLink move constructor for source %x", &m_source);
    r.deactivate();
    RegisterTargetCb();
}

template<class T>
RefLink<T>::~RefLink()
{
    if(m_bActive)
        set(NULL);
}

template<class T>
void RefLink<T>::deactivate()
{
    A(m_bActive);
    m_bActive = false;
    UnregisterTargetCb();
}

template<class T>
void RefLink<T>::RegisterTargetCb()
{
    if(m_target)
    {
        m_targetRegs.push_back(m_target->observableReferentiable().Register(Referentiable::Event::DEACTIVATE_LINKS, [this](Referentiable*r){
            
            A(r==m_target);
            
            m_target->unRegisterSource(m_source);
            m_source.unRegisterTarget(*m_target);

            UnregisterTargetCb();
            
            m_bTargetIsUp = false;            
        }));
        
        m_bTargetIsUp = true;
    }
}

template<class T>
void RefLink<T>::UnregisterTargetCb()
{
    if(m_target && m_bTargetIsUp)
    {
        for(auto &i:m_targetRegs)
            m_target->observableReferentiable().Remove(i);        
    }
    m_targetRegs.clear();
}

template<class T>
RefLink<T>::operator T*()
{
    return edit();
}
template<class T>
RefLink<T>::operator const T*() const
{
    return get();
}
template<class T>
T* RefLink<T>::operator->()
{
    return edit();
}
template<class T>
const T* RefLink<T>::operator->() const
{
    return get();
}
template<class T>
T& RefLink<T>::operator*()
{
    return *edit();
}
template<class T>
const T& RefLink<T>::operator*() const
{
    return *get();
}
template<class T>
auto RefLink<T>::operator= ( RefLink<T> & other) -> RefLink &
{
    set(other.edit());
    return *this;
}
template<class T>
auto RefLink<T>::operator= (T * pointer) -> RefLink &
{
    set(pointer);
    return *this;
}
template<class T>
bool RefLink<T>::operator < (RefLink & other)
{
    T * t1 = *this;
    T * t2 = other;
    return (t1 < t2);
}

template<class T>
void RefLink<T>::set(T * target)
{
    if(target == m_target)
        return;
    
    UnregisterTargetCb();
    
    if(m_target&&m_bTargetIsUp)
    {
        m_target->unRegisterSource(m_source);
        m_source.unRegisterTarget(*m_target);
        
        size_t cs = m_target->countSources();
        if(cs == 0)
        {
            //LG(INFO, "(%s)RefLink<T>::set(%s) deinstantiate former target %s", m_source.sessionName().c_str(), target?target->sessionName().c_str():"NULL", m_target->sessionName().c_str());
            m_target->deinstantiate();
        } else {
            //LG(INFO, "(%s)RefLink<T>::set(%s) former target %s has %d sources", m_source.sessionName().c_str(), target?target->sessionName().c_str():"NULL", m_target->sessionName().c_str(), cs);
        }
    }
    m_target = target;

    if(m_target)
    {
        m_source.registerTarget(*m_target);
        m_target->registerSource(m_source);
    }
    
    RegisterTargetCb();
}

template<class T>
const T * RefLink<T>::get() const
{
    return m_target;
}

template<class T>
T * RefLink<T>::edit() const
{
    return m_target;
}


