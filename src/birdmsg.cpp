/*==============================================================================
 *
 *       Filename:  birdmsg.cpp
 *
 *    Description:  This sends/receives message for the bird.
 *
 * =============================================================================
 */

#include "../inc/birdinc.h"

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
    cout<<"Bird: Cannot send msg"<<endl;
  }
  return;
}   /* -----  end of function sendMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  sendBirdPosnMsg
 * =============================================================================
 */
void sendBirdPosnMsg (int destPort)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X---- Bird Posn ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  short unsigned int msgType = BIRD_POSN_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  short unsigned int birdLoc = rand() % (MAX_POSN + 1);
  birdLoc = htons(birdLoc);
  memcpy (outMsg, &birdLoc, PORT_SIZE);
  outMsg += PORT_SIZE;
  outMsgSize += PORT_SIZE;

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function sendBirdPosnMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  sendBirdLandMsg
 * =============================================================================
 */
void sendBirdLandMsg (int destPort)
{
  /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  short unsigned int msgType = BIRD_LAND_MSG;
  msgType = htons(msgType);
  memcpy (outMsg, &msgType, MSG_TYPE_SIZE);
  outMsg += MSG_TYPE_SIZE;
  outMsgSize += MSG_TYPE_SIZE;

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function sendBirdLandMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleWinnerMsg
 * =============================================================================
 */
void handleWinnerMsg (int inMsgSize, char *inMsg)
{
    /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X--- Winner Port --->
  */

  unsigned short int port;
  memcpy (&port, inMsg, PORT_SIZE);
  port = ntohs(port);
  inMsg += PORT_SIZE;
  inMsgSize -= PORT_SIZE;

  sendBirdPosnMsg(port);

  sleep (rand() % (MAX_SLEEP + 1));

  sendBirdLandMsg(port);

  return;
}		/* -----  end of function handleWinnerMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  birdMsgHandler
 *  Description:  This function accepts all the messages, finds out their type,
 *                and calls their respective handlers.
 * =============================================================================
 */
void birdMsgHandler (int inMsgSize, char *inMsg)
{
  if (inMsgSize < 2)
  {
    cout<<"Corrupted message at bird "<<endl;
    return;
  }

  short unsigned int msgType;
  memcpy (&msgType, inMsg, 2);
  msgType = ntohs(msgType);

  inMsg += 2;
  inMsgSize -= 2;

  switch (msgType) {
    case ELECT_WINNER_MSG: {
      handleWinnerMsg(inMsgSize, inMsg);
      break;
    }
    default: {
      cout<<"Invalid msg received at bird "<<endl;
      break;
    }
  }
  return;
}
