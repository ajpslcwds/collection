#ifndef DATA_RECEIVER_I_H
#define DATA_RECEIVER_I_H

#include "keyvalue.h"

class DataReceiverI : public DSF::DataReceiver
{
  public:
    virtual void sendData(const DSF::DataUnitSeq &dataSeq, const Ice::Current &) override;
};

#endif
