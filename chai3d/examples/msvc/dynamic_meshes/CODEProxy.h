
// A slightly modified proxy algorithm is required to support ODE;
// we override the relevant functions in a subclass...
class cODEProxy : public cProxyPointForceAlgo
{    

protected:
    //! Remove the approximate test for whether the proxy has reached the goal point; use exact distances
    virtual bool goalAchieved(const cVector3d& a_proxy, const cVector3d& a_goal) const
    {
        // Always fail this test to force the proxy to continue...
        return false;
    }

    //! Remove the offsetting of the goal to account for proxy volume; use exact collision vectors
    virtual void offsetGoalPosition(cVector3d& a_goal, const cVector3d& a_proxy) const
    {
        // Leave the goal where it is...
        return;
    }

    //! Update dynamic contact state
    virtual void updateDynamicContactState()
    {
        // Update "last-state" dynamic contact information
        cGenericObject* savedTouchingObject = m_touchingObject;
        m_touchingObject = m_defaultObject;
        cProxyPointForceAlgo::updateDynamicContactState();
        m_touchingObject = savedTouchingObject;
    }

    //! Correct for object motion
    virtual void correctProxyForObjectMotion()
    {  
        m_touchingObject = m_defaultObject;
        cProxyPointForceAlgo::correctProxyForObjectMotion(); 
    }

    //! Compute next best proxy position
    virtual void computeNextBestProxyPosition(cVector3d a_goal)
    {
        m_world->setHapticEnabled(0, 1);
        m_defaultObject->setHapticEnabled(1, 1);
        cProxyPointForceAlgo::computeNextBestProxyPosition(a_goal);
        m_world->setHapticEnabled(1, 1);
    }

public:
    //! Default object
    cGenericObject* m_defaultObject;

    //! A constructor that copies relevant initialization state from another proxy
    cODEProxy(cProxyPointForceAlgo* a_oldProxy)
    {
        m_deviceGlobalPos = a_oldProxy->getDeviceGlobalPosition();
        m_proxyGlobalPos = a_oldProxy->getProxyGlobalPosition();
        m_lastGlobalForce.zero();
        m_world = a_oldProxy->getWorld();
        m_radius = a_oldProxy->getProxyRadius();
    }
};