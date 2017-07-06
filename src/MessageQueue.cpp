#include "MessageQueue.h"
#include <iostream>

MessageQueue::MessageQueue() : m_msgReady(false), m_keypad("/dev/i2c-2", 0x20)
{
  std::cout << "Initializing message queue..." << std::endl;

  m_keypad.Initialize([&](char key)
  {
    this->PostMessage(MessageTypes::sm_key, key);
    return 0;
  });
}

MessageQueue::~MessageQueue()
{
  //dtor
}

void MessageQueue::PostMessage(MessageTypes msgType, int data)
{
  m_mux.lock();
  MSG msg = {msgType, data};
  m_msgQueue.push(msg);
  m_mux.unlock();
  m_msgReady = true;
  m_cv.notify_all();
}

bool MessageQueue::GetMessage(MSG& msg)
{
  std::unique_lock<std::mutex> lck(m_cvMux);

  while(!m_msgReady)
      m_cv.wait(lck);

  m_mux.lock();
  msg = m_msgQueue.front();
  m_msgQueue.pop();
  m_msgReady = !m_msgQueue.empty();
  m_mux.unlock();

  return true;
}
