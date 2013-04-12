/*==============================================================================
 *
 *       Filename:  pigmain.cpp
 *
 *    Description:  This starts off the pig's execution
 *
 * =============================================================================
 */


#include "../inc/piginc.h"

OwnStruct ownNode;
vector<OtherStruct> otherVector;
mutex otherVectorLock;
atomic<unsigned short int> birdPosn;

/* ===  FUNCTION  ==============================================================
 *         Name:  listenerFlow
 *  Description:  The pig listens for incoming messages here
 * =============================================================================
 */
static void listenerFlow ()
{

  UDPSocket listenSocket (COM_IP_ADDR, ownNode.port);

  while (true) {
    // Block for msg receipt
    int inMsgSize;
    char *inMsg;
    inMsg = new char[MAX_MSG_SIZE]();
    try {
      inMsgSize = listenSocket.recv(inMsg, MAX_MSG_SIZE);
    } catch (SocketException &e) {
      cout<<ownNode.port<<": "<<e.what()<<endl;
    }
    inMsg[inMsgSize] = '\0';

    thread handlerThread (pigMsgHandler, inMsgSize, inMsg);
    handlerThread.detach();
  }
}   /* -----  end of function listenerFlow  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  main
 *  Description:  The pig's execution starts here
 * =============================================================================
 */
int main (int argc, char *argv[])
{
  if (argc < 2) {
    cout<<"Invalid number of arguments to pig"<<endl;
    return EXIT_FAILURE;
  }

  ownNode.port = atoi(argv[1]);
  unsigned int randSeed = time(NULL);
  srand(randSeed + ownNode.port);
  ownNode.posn = (rand() % MAX_POSN) + 1;
  otherVectorLock.lock();

  for (int iter = 2; iter < argc; iter++) {
    OtherStruct otherNode;
    otherNode.port = atoi(argv[iter]);
    otherNode.posn = 0;
    otherVector.push_back(otherNode);
  }
  otherVectorLock.unlock();

  // Listen on incoming port for messages
  thread handlerThread(listenerFlow);

  // Wait for the other pigs to be spawned
  sleep(1);

  // Start off the election process
  otherVectorLock.lock();
  for (auto &otherNode : otherVector) {
    pigSendOfferMsg (otherNode.port);
  }
  otherVectorLock.unlock();
  offerSent = true;

  handlerThread.join();
  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
