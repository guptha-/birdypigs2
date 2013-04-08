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
 *         Name:  pigSendWinnerMsg
 * =============================================================================
 */
void pigSendWinnerMsg ()
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X----- Own Port ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, ELECT_WINNER_MSG);
  short unsigned int port = ownNode.port;
  addTwoBytes(outMsg, outMsgSize, port);

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
  <----- Own Posn ---->
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

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function pigSendOfferMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleBirdPosnMsg
 * =============================================================================
 */
void handleBirdPosnMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Bird Posn ---->
  */

  unsigned short int posn = getTwoBytes(inMsg, inMsgSize);

  cout<<ownNode.port<<": Got bird posn msg "<<posn<<endl;
  birdPosn = posn;
/* DO Something */ 
  return;
}		/* -----  end of function handleBirdPosnMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleBirdLandMsg
 * =============================================================================
 */
void handleBirdLandMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ---->
  */

  cout<<ownNode.port<<": Got bird land msg"<<endl;
  /* Do something */

  return;
}		/* -----  end of function handleBirdLandMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleOfferMsg
 * =============================================================================
 */
void handleOfferMsg (int inMsgSize, char *inMsg)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X------- Port ------>
  <------- Posn ------>
  */

  unsigned short int port = getTwoBytes(inMsg, inMsgSize);
  unsigned short int posn = getTwoBytes(inMsg, inMsgSize);

  otherVectorLock.lock();
  for (auto &otherNode : otherVector) {
    if (port == otherNode.port) {
      if (otherNode.posn == 0) {
        // This is the first time in this election that we are receiving an
        // offer from this pig
        otherNode.posn = posn;
        offerCount++;
      }
    }
  }

  if (offerCount == otherVector.size()) {
    // We have received offers from everyone
    otherVectorLock.unlock();
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
    case BIRD_POSN_MSG: {
      handleBirdPosnMsg(inMsgSize, inMsg);
      break;
    }
    case BIRD_LAND_MSG: {
      handleBirdLandMsg(inMsgSize, inMsg);
      break;
    }
    default:
    {
      cout<<"Invalid msg received at pig "<<ownNode.port<<endl;
      break;
    }
  }
  return;
}
