/*==============================================================================
 *
 *       Filename:  birdmain.cpp
 *
 *    Description:  The bird is responsible for spawning pigs, sending the bird
 *                  landing coordinates and the bird land msg to the coordinator
 *
 * =============================================================================
 */

#include "../inc/birdinc.h"

static vector<unsigned int> pigPorts;

/* ===  FUNCTION  ==============================================================
 *         Name:  listenerFlow
 *  Description:  The bird listens for incoming messages here
 * =============================================================================
 */
static void listenerFlow ()
{
 UDPSocket listenSocket (BIRD_LISTEN_PORT);

  while (true) {
    // Block for msg receipt
    int inMsgSize;
    char *inMsg;
    inMsg = new char[MAX_MSG_SIZE]();
    try {
      inMsgSize = listenSocket.recv(inMsg, MAX_MSG_SIZE);
    } catch (SocketException &e) {
      cout<<"Bird: "<<e.what()<<endl;
    }
    inMsg[inMsgSize] = '\0';

    thread handlerThread (birdMsgHandler, inMsgSize, inMsg);
    handlerThread.detach();
  }
  
}   /* -----  end of function listenerFlow  ----- */


/* ===  FUNCTION  ==============================================================
 *         Name:  getPigPorts
 * =============================================================================
 */
static int getPigPorts ()
{
  ifstream portFile;
  string str;
  portFile.open("portConfig");
  if (!portFile.good()) {
    cout<<"No port config file found.\n";
    return EXIT_FAILURE;
  }
  cout<<"Port numbers \t";
  while (true) {
    getline(portFile, str);
    if (portFile.eof()) {
      break;
    }
    int portNum = atoi(str.c_str());
    if (portNum == BIRD_LISTEN_PORT) {
      cout<<portNum<<" is the port the coordinator listens at! Remove from"<<
        "port list!"<<endl;
      return EXIT_FAILURE;
    }
    pigPorts.push_back(portNum);
    cout<<portNum<<"\t";
    fflush(stdout);
  }
  cout<<endl;

  return EXIT_SUCCESS;
}   /* -----  end of function getPigPorts  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  spawnPigs
 * =============================================================================
 */ 
int spawnPigs ()
{
  int pigPortsCount = pigPorts.size();
  system("killall -q -9 pig");
  
  for (auto &curPort : pigPorts) {
    int child = fork();
    if (child < 0) {
      cout<<"Problem spawning pigs!"<<endl;
      return EXIT_FAILURE;
    } else if (child == 0) {
      // Child address space
      char **array = (char **) malloc (sizeof(char *) * (pigPortsCount + 2));
      char **actualArray = array;
      (*array) = new char [4]();
      sprintf((*array), "pig");
      array++;
      (*array) = new char [6]();
      sprintf((*array), "%d", curPort);
      array++;
      for (auto &otherPort : pigPorts) {
        if (otherPort == curPort) {
          continue;
        }

        (*array) = new char [6]();
        sprintf((*array), "%d", otherPort);
        array++;
      }

      execv ("./bin/pig", actualArray);
      cout<<"Problem spawning child"<<endl;
      cout<<errno;
      exit(0);
    }
  }

  return EXIT_SUCCESS;
}   /* -----  end of function spawnPigs  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  main
 * =============================================================================
 */
int main (int argc, char **argv) 
{
  srand(time(NULL));
  
  // Getting the ports of the pigs from the file
  if (EXIT_FAILURE == getPigPorts ()) {
    return EXIT_FAILURE;
  }

  // Listen on incoming port for messages
  thread handlerThread (listenerFlow);

  // Spawning the pigs
  if (EXIT_FAILURE == spawnPigs ()) {
    return EXIT_FAILURE;
  }
  sleep(10000);
}				/* ----------  end of function main  ---------- */
