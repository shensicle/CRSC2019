#ifndef _MESSAGE_SENDER_H
#define _MESSAGE_SENDER_H

// Virtual base class to send a message to an external service
class MessageSenderClass
{
  private:

  public:
    MessageSenderClass (void) {};

    // Send a message. Return value indicates whether or not message was successfully sent
    virtual bool Send (String theMessage) = 0;
  
};

#endif
