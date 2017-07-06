#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Keypad.h"

enum MessageTypes
{
  sm_none,
  sm_key,
  sm_timer
};

struct MSG
{
  MessageTypes  type;
  int           data;
};

class MessageQueue
{
public:
  MessageQueue();
  virtual ~MessageQueue();
  void PostMessage(MessageTypes msgType, int data);
  bool GetMessage(MSG& msg);

private:
  bool                    m_msgReady;
  std::mutex              m_cvMux;
  std::condition_variable m_cv;

  std::mutex m_mux;
  std::queue<MSG> m_msgQueue;
  CKeypad m_keypad;
};
