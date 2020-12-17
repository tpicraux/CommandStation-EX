#ifndef _TIMEOUT__H_
#define _TIMEOUT__H_

#include <StateMachine.h>

class Timeout : public StateMachine
{
  private:
    bool m_timing;
    unsigned long m_timeout;

  public:
    Timeout() : 
      StateMachine(1, true), 
      m_timing(false),
      m_timeout(0L)
    {}

    virtual bool update()
    {
      if (!StateMachine::update()) return false;
      if (m_timing && ((long) (millis() - m_timeout) >= 0)) {
        m_timing = false;
      }
      return true;
    }

    void setTimeout(const unsigned long timeout)
    {
      m_timeout = millis() + timeout;
      m_timing = true;
    }

    bool isTimedOut() const
    {
      return !m_timing;
    }
};

#endif
