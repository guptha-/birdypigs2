/*==============================================================================
 *
 *       Filename:  pigmsg.cpp
 *
 *    Description:  The message related functions for pig
 *
 * =============================================================================
 */
#include "../inc/piginc.h"

static atomic<unsigned int> offerCount(0);
atomic<unsigned int> score(0);
atomic<unsigned int> numberRounds(1);
extern atomic<unsigned int> numberAffected;
atomic<unsigned int> timeticks(0);

/* ===  FUNCTION  ==============================================================
 *         Name:  getTwoBytes
 *  Description:  Gets two bytes from the message, and returns equiv. value
 * =============================================================================
 */
unsigned short int getTwoBytes (char *&inMsg, int &inMsgSize)
{
  unsigned short int reply;
  memcpy (&reply, inMsg, 2);
  reply = ntohs(reply);
  inMsg += 2;
  inMsgSize -= 2;

  return reply;
}		/* -----  end of function getTwoBytes  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  addTwoBytes
 *  Description:  Adds two bytes to the message from the given value
 * =============================================================================
 */
void addTwoBytes (char *&inMsg, int &inMsgSize, int value)
{
  value = htons(value);
  memcpy (inMsg, &value, 2);
  inMsg += 2;
  inMsgSize += 2;

}		/* -----  end of function addTwoBytes  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  sendMsg
 *  Description:  This function sends the given msg to the give port
 * =============================================================================
 */
void sendMsg(char *outMsg, int outMsgSize, unsigned short int destPort)
{
  try {
    static UDPSocket sendSocket;
    sendSocket.sendTo(outMsg, outMsgSize, COM_IP_ADDR, destPort);
  } catch (SocketException &e) {
    cout<<ownNode.port<<": Cannot send msg"<<endl;
  }
  return;
}   /* -----  end of function sendMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigSendPassMsg
 * =============================================================================
 */
void pigSendPassMsg (unsigned short int port)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Prev score --->
  <--- Time Stamp ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, ELECT_PASS_MSG);
  short unsigned int oldScore = score;
  addTwoBytes(outMsg, outMsgSize, oldScore);
  addTwoBytes(outMsg, outMsgSize, timeticks);

  sendMsg(permOutMsg, outMsgSize, port);

  return;
}		/* -----  end of function pigSendPassMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigSendStatusReplyMsg
 *  Description:  This function sends the timestamp at which the bird is 
 *                expected to land, and requests a status from the recipient pig
 * =============================================================================
 */
void pigSendStatusReplyMsg (unsigned short int port, bool status)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Status ------->
  <--- Time stamp ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, STATUS_REPLY_MSG);
  if (status == true) {
    addTwoBytes(outMsg, outMsgSize, 1);
  } else {
    addTwoBytes(outMsg, outMsgSize, 0);
  }
  addTwoBytes(outMsg, outMsgSize, timeticks);

  sendMsg(permOutMsg, outMsgSize, port);

  return;
}		/* -----  end of function pigSendStatusReplyMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigSendStatusReqMsg
 *  Description:  This function sends the timestamp at which the bird is 
 *                expected to land, and requests a status from the recipient pig
 * =============================================================================
 */
void pigSendStatusReqMsg (unsigned short int port, unsigned int birdLandTime)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Winner port -->
  <--- Time stamp ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, STATUS_REQ_MSG);
  short unsigned int winnerPort = ownNode.port;
  addTwoBytes(outMsg, outMsgSize, winnerPort);
  addTwoBytes(outMsg, outMsgSize, timeticks + birdLandTime);

  sendMsg(permOutMsg, outMsgSize, port);

  return;
}		/* -----  end of function pigSendStatusReqMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigSendWinnerMsg
 * =============================================================================
 */
void pigSendWinnerMsg ()
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X----- Own Port ---->
  <--- Time Stamp ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, ELECT_WINNER_MSG);
  short unsigned int port = ownNode.port;
  addTwoBytes(outMsg, outMsgSize, port);
  addTwoBytes(outMsg, outMsgSize, timeticks);

  cout<<"Sending winner msg"<<endl;
  sendMsg(permOutMsg, outMsgSize, BIRD_LISTEN_PORT);

  return;
}		/* -----  end of function pigSendWinnerMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigSendOfferMsg
 * =============================================================================
 */
void pigSendOfferMsg (int destPort)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X----- Own Port ---->
  <----- Own Posn ----X--- Time Stamp ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, ELECT_OFFER_MSG);
  short unsigned int port = ownNode.port;
  addTwoBytes(outMsg, outMsgSize, port);
  short unsigned int posn = ownNode.posn;
  addTwoBytes(outMsg, outMsgSize, posn);
  addTwoBytes(outMsg, outMsgSize, timeticks);

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function pigSendOfferMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  processTimeticks
 *  Description:  This function compares the received timestamp against the 
 *                current stored timeticks value and updates the system 
 *                timeticks as needed
 * =============================================================================
 */
void processTimeticks (unsigned int tempTimeticks)
{
  if (tempTimeticks <= timeticks) {
    timeticks++;
  } else {
    timeticks = ++tempTimeticks;
  }
  return;
}		/* -----  end of function processTimeticks  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handlePassMsg
 * =============================================================================
 */
void handlePassMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Prev score --->
  <--- Time Stamp ---->
  */

  score = getTwoBytes(inMsg, inMsgSize);
  processTimeticks(getTwoBytes(inMsg, inMsgSize));

  cout<<"Calling init from handlePassMsg"<<endl;
  pigInitElection();

}		/* -----  end of function handlePassMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleStatusReqMsg
 * =============================================================================
 */
void handleStatusReqMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Winner port -->
  <--- Time stamp ---->
  */

  unsigned short int winnerPort = getTwoBytes(inMsg, inMsgSize);
  unsigned int tempTimeticks = getTwoBytes(inMsg, inMsgSize);

  timeticks += (rand() % MAX_AIRTIME) + 1;

  cout<<"ownTime "<<timeticks<<" othertime "<<tempTimeticks<<endl;
  /* A is introduced to account for the fact that many more message
   * exchanges involve the leader */
  otherVectorLock.lock();
  int biasSize = (otherVector.size() + 1) / 2;
  otherVectorLock.unlock();
  if (timeticks + biasSize > tempTimeticks) {
    // We are dead
    cout<<ownNode.port<<": I am hit!"<<endl;
    pigSendStatusReplyMsg(winnerPort, true);
  } else {
    cout<<ownNode.port<<": I am not hit!"<<endl;
    pigSendStatusReplyMsg(winnerPort, false);
  }

  return;
}		/* -----  end of function handleStatusReqMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleBirdPosnMsg
 * =============================================================================
 */
void handleBirdPosnMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Bird Posn ---->
  <--- Time stamp ----X---- Bird Time ---->
  */

  unsigned short int posn = getTwoBytes(inMsg, inMsgSize);
  processTimeticks(getTwoBytes(inMsg, inMsgSize));
  unsigned short int birdTime = getTwoBytes(inMsg, inMsgSize);

  cout<<ownNode.port<<": Got bird posn msg "<<posn<<endl;
  birdPosn = posn;

  processAffectedPigs (birdTime);
  return;
}		/* -----  end of function handleBirdPosnMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleStatusReplyMsg
 * =============================================================================
 */
void handleStatusReplyMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Status ------->
  <--- Time stamp ---->
  */

  unsigned short int status = getTwoBytes(inMsg, inMsgSize);
  processTimeticks(getTwoBytes(inMsg, inMsgSize));

  if (status == 1) {
    score++;
  }
  numberAffected--;
  if (numberAffected == 0) {
    otherVectorLock.lock();
    for (auto &otherNode : otherVector) {
      pigSendPassMsg(otherNode.port);
    }
    otherVectorLock.unlock();
    cout<<"The score is "<<score<<" in "<<numberRounds<<" rounds!!!"<<endl<<endl;
    cout<<"Calling init from handleStatusReplyMsg"<<endl;
    pigInitElection();
  }
  return;
}		/* -----  end of function handleStatusReplyMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleOfferMsg
 * =============================================================================
 */
void handleOfferMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X------- Port ------>
  <------- Posn ------X--- Time Stamp ---->
  */

  unsigned short int port = getTwoBytes(inMsg, inMsgSize);
  unsigned short int posn = getTwoBytes(inMsg, inMsgSize);

  otherVectorLock.lock();
  for (auto &otherNode : otherVector) {
    if (port == otherNode.port) {
      otherNode.posn = posn;
      offerCount++;
    }
  }

  if (offerCount == otherVector.size()) {
    // We have received offers from everyone
    otherVectorLock.unlock();
    offerCount = 0;
    pigFinishElection();
  }
  otherVectorLock.unlock();
  
  return;
}		/* -----  end of function handleOfferMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigMsgHandler
 *  Description:  This function accepts all the messages, finds out their type,
 *                and calls their respective handlers.
 * =============================================================================
 */
void pigMsgHandler (int inMsgSize, char *inMsg)
{
  if (inMsgSize < 2) {
    cout<<"Corrupted message at pig "<<ownNode.port<<endl;
    return;
  }

  short unsigned int msgType = getTwoBytes(inMsg, inMsgSize);;

  switch (msgType) {
    case ELECT_OFFER_MSG: {
      handleOfferMsg(inMsgSize, inMsg);
      break;
    }
    case ELECT_PASS_MSG: {
      handlePassMsg(inMsgSize, inMsg);
      break;
    }
    case BIRD_POSN_MSG: {
      handleBirdPosnMsg(inMsgSize, inMsg);
      break;
    }
    case STATUS_REQ_MSG: {
      handleStatusReqMsg(inMsgSize, inMsg);
      break;
    }
    case STATUS_REPLY_MSG: {
      handleStatusReplyMsg(inMsgSize, inMsg);
      break;
    }
    default:
    {
      cout<<"Invalid msg received at pig "<<ownNode.port<<endl;
      break;
    }
  }
  return;
}		/* -----  end of function pigMsgHandler  ----- */
