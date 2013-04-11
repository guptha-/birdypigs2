#ifndef inc_PIG_PROT
#define inc_PIG_PROT

#include "piginc.h"
using namespace std;

void pigInitElection();
void pigMsgHandler(int inMsgSize, char *inMsg);
void pigFinishElection ();
void pigSendWinnerMsg ();
void pigSendOfferMsg (int destPort);
void pigSendStatusReqMsg (unsigned short int port, unsigned int);
void processAffectedPigs (int);
void pigSendPassMsg (unsigned short int port);
#endif
