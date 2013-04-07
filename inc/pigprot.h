#ifndef inc_PIG_PROT
#define inc_PIG_PROT

#include "piginc.h"
using namespace std;

void pigInitElection();
void pigMsgHandler(int inMsgSize, char *inMsg);
void pigFinishElection ();
void pigSendWinnerMsg ();
void pigSendOfferMsg (int destPort);
#endif
