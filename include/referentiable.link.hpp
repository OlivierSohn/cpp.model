using namespace imajuscule;

template<class T>
RefLink<T>::RefLink(Referentiable& source, T *target) :
m_source(source)
,m_target(NULL)
,m_bActive(true)
,m_bTargetIsUp(true)
{
    //LG(INFO, "%p link construct for source %x", this, &m_source);
    set(target);
}

template<class T>
RefLink<T>::RefLink(RefLink<T> && r) :
m_source(r.m_source)
,m_target(r.m_target)
,m_bActive(true)
,m_bTargetIsUp(true)
{
    //LG(INFO, "%p link move construct from %p for source %x", this, &r, &m_source);
    r.deactivate();
    RegisterTargetCb();
}

template<class T>
RefLink<T> & RefLink<T>::operator=(RefLink<T> && r) {
    m_source = std::move(r.m_source);
    m_target = std::move(r.m_target);
    m_bActive = true;
    m_bTargetIsUp = true;
    r.deactivate();
    m_targetRegs.clear();
    RegisterTargetCb();
    
    //LG(INFO, "%p link move assign from %p for source %x", this, &r, &m_source);
    return *this;
}

template<class T>
RefLink<T>::~RefLink()
{
    if(m_bActive) {
        //LG(INFO, "%p destruct active for source %x", this, &m_source);
        set(NULL);
    } else {
        //LG(INFO, "%p destruct inactive for source %x", this, &m_source);
    }
}

template<class T>
void RefLink<T>::deactivate()
{
    //LG(INFO, "%p deactivate", this);
    A(m_bActive);
    m_bActive = false;
    UnregisterTargetCb();
}

template<class T>
void RefLink<T>::RegisterTargetCb()
{
    if(m_target)
    {
        //LG(INFO, "    register target %x", m_target);
        
        A(m_targetRegs.empty());

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
        //LG(INFO, "    unregister target %x", m_target);
        for(auto &i:m_targetRegs)
            m_target->observableReferentiable().Remove(i);        
    }
    m_targetRegs.clear();
}

template<class T>
RefLink<T>::operator T*() const
{
    return ptr();
}
template<class T>
T* RefLink<T>::operator->() const
{
    return ptr();
}
template<class T>
T& RefLink<T>::operator*() const
{
    return *ptr();
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
T * RefLink<T>::ptr() const
{
    return m_target;
}
