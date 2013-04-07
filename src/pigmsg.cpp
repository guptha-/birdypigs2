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
 *         Name:  sendMsg
 *  Description:  This function sends the given msg to the give port
 * =============================================================================
 */
void sendMsg(char *outMsg, int outMsgSize, unsigned short int destPort)
{
  try
  {
    static UDPSocket sendSocket;
    sendSocket.sendTo(outMsg, outMsgSize, COM_IP_ADDR, destPort);
  }
  catch (SocketException &e)
  {
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

  short unsigned int msgType = ELECT_WINNER_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  short unsigned int port = ownNode.port;
  port = htons(port);
  memcpy (outMsg, &port, PORT_SIZE);
  outMsg += PORT_SIZE;
  outMsgSize += PORT_SIZE;

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

  short unsigned int msgType = ELECT_OFFER_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  short unsigned int port = ownNode.port;
  port = htons(port);
  memcpy (outMsg, &port, PORT_SIZE);
  outMsg += PORT_SIZE;
  outMsgSize += PORT_SIZE;

  short unsigned int posn = ownNode.posn;
  posn = htons(posn);
  memcpy (outMsg, &posn, POSN_SIZE);
  outMsg += POSN_SIZE;
  outMsgSize += POSN_SIZE;

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function pigSendOfferMsg  ----- */

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

  unsigned short int port;
  memcpy (&port, inMsg, PORT_SIZE);
  port = ntohs(port);
  inMsg += PORT_SIZE;
  inMsgSize -= PORT_SIZE;

  unsigned short int posn;
  memcpy (&posn, inMsg, POSN_SIZE);
  posn = ntohs(posn);
  inMsg += POSN_SIZE;
  inMsgSize -= POSN_SIZE;

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
  if (inMsgSize < 2)
  {
    cout<<"Corrupted message at pig "<<ownNode.port<<endl;
    return;
  }

  short unsigned int msgType;
  memcpy (&msgType, inMsg, 2);
  msgType = ntohs(msgType);

  inMsg += 2;
  inMsgSize -= 2;

  switch (msgType) {
    case ELECT_OFFER_MSG: {
      handleOfferMsg(inMsgSize, inMsg);
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
