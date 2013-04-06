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

/* ===  FUNCTION  ==============================================================
 *         Name:  main
 *  Description:  The pig's execution starts here
 * =============================================================================
 */
int main (int argc, char *argv[])
{
  memset(&ownNode, 0, sizeof(ownNode));
  if (argc < 2) {
    cout<<"Invalid number of arguments to pig"<<endl;
  }

  ownNode.port = atoi(argv[1]);
  cout<<"own Port :"<<ownNode.port<<endl;
  otherVectorLock.lock();
  for (int iter = 2; iter < argc; iter++) {
    OtherStruct otherNode;
    otherNode.port = atoi(argv[iter]);
    otherNode.posn = 0;
    cout<<otherNode.port<<"\t";
    otherVector.push_back(otherNode);
  }
  otherVectorLock.unlock();
  cout<<endl;
  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
