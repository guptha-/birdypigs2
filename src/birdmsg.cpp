/*==============================================================================
 *
 *       Filename:  birdmsg.cpp
 *
 *    Description:  This sends/receives message for the bird.
 *
 * =============================================================================
 */

#include "../inc/birdinc.h"

static atomic<unsigned int> timeticks(0);

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
}   /* -----  end of function processTimeticks  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  getTwoBytes
 *  Description:  Gets two bytes from the message, and returns equiv. int
 * =============================================================================
 */
unsigned short int getTwoBytes (char *&msg, int &msgSize)
{
  unsigned short int reply;
  memcpy (&reply, msg, 2);
  reply = ntohs(reply);
  msg += 2;
  msgSize -= 2;

  return reply;
}		/* -----  end of function getTwoBytes  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  addTwoBytes
 *  Description:  Adds two bytes to the message from the given value
 * =============================================================================
 */
void addTwoBytes (char *&msg, int &msgSize, int value)
{
  value = htons(value);
  memcpy (msg, &value, 2);
  msg += 2;
  msgSize += 2;

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
  <--- Time stamp ----X---- Bird Time ---->
  */
  char msg[MAX_MSG_SIZE];
  char *outMsg = msg;
  memset(outMsg, 0, MAX_MSG_SIZE);
  char *permOutMsg = outMsg;
  int outMsgSize = 0;

  addTwoBytes(outMsg, outMsgSize, BIRD_POSN_MSG);
  short unsigned int birdLoc = (rand() % MAX_POSN) + 1;
  addTwoBytes(outMsg, outMsgSize, birdLoc);
  addTwoBytes(outMsg, outMsgSize, timeticks);
  addTwoBytes(outMsg, outMsgSize, AIRTIME);

  sendMsg(permOutMsg, outMsgSize, destPort);

  return;
}		/* -----  end of function sendBirdPosnMsg  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  handleWinnerMsg
 * =============================================================================
 */
void handleWinnerMsg (int inMsgSize, char *inMsg)
{
    /*
  |--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|
  <----- Msg Type ----X--- Winner Port --->
  <--- Time Stamp ---->
  */

  unsigned short int port = getTwoBytes(inMsg, inMsgSize);
  processTimeticks(getTwoBytes(inMsg, inMsgSize));

  sendBirdPosnMsg(port);

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
  if (inMsgSize < 2) {
    cout<<"Corrupted message at bird "<<endl;
    return;
  }

  short unsigned int msgType = getTwoBytes(inMsg, inMsgSize);;

  switch (msgType) {
    case ELECT_WINNER_MSG: {
      handleWinnerMsg(inMsgSize, inMsg);
      break;
    }
    default: {
      cout<<"Invalid msg received at bird "<<msgType<<endl;
      break;
    }
  }
  return;
}
