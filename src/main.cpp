#include "Selections.h"

#ifdef ESPNOW
#ifdef SENDER
#include "espnowSender.h"
#else
#include "espnowReceiver.h"
#endif
#else
#ifdef SENDER
#include "sender.h"
#else
#include "receiver.h"
#endif
#endif