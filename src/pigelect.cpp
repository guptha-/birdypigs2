/*==============================================================================
 *
 *       Filename:  pigelect.cpp
 *
 *    Description:  The election process for pigs
 *
 * =============================================================================
 */
#include "../inc/piginc.h"

atomic<bool> winner(false);

/* ===  FUNCTION  ==============================================================
 *         Name:  getWallPosns
 * =============================================================================
 */
static void getWallPosns (vector<unsigned int> &wallPosns)
{
  int numberWalls = rand() % (MAX_WALLS + 1); // Limit number of walls
  // Storing wall positions
  while (numberWalls--)
  {
    unsigned int posn;
    while (true)
    {
      posn = (rand() % MAX_POSN) + 1;
      int duplicate = false;
      for (auto &num : wallPosns)
      {
        // making sure there is no overlap
        if (num == posn)
        {
          duplicate = true;
          break;
        }
      }
      if (duplicate == false)
      {
        break;
      }
    }
    wallPosns.push_back(posn);
  }
  cout<<"Wall posns\t";
  for (auto &wallLoc : wallPosns)
  {
    cout<<wallLoc<<"\t";
  }
  cout<<endl;
}   /* -----  end of function getWallPosns  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  performWinnerTasks
 *  Description:  We are the winner. We calculate wall posns, send winner mesg
 *                to bird, and wait for msgs from bird
 * =============================================================================
 */
void performWinnerTasks ()
{
  vector<unsigned int> wallPosns;
  getWallPosns(wallPosns);

  winner = true;
  
  cout<<"Winner "<<ownNode.port<<" at "<<ownNode.posn<<endl;
  pigSendWinnerMsg();
  return;
}		/* -----  end of function performWinnerTasks  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigInitElection
 *  Description:  This initiates the subsequent elections
 * =============================================================================
 */
void pigInitElection ()
{
  ownNode.posn = (rand() % (MAX_POSN + 1));
  otherVectorLock.lock();
  for (auto &otherNode : otherVector) {
    pigSendOfferMsg (otherNode.port);
  }
  otherVectorLock.unlock();

  return;
}		/* -----  end of function pigInitElection  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  pigFinishElection
 *  Description:  All offer msgs have been received. Make a local decision about
 *                the outcome of the election
 * =============================================================================
 */
void pigFinishElection ()
{
  unsigned int lowestPosn = MAX_POSN;
  otherVectorLock.lock();
  for (auto &otherNode : otherVector) {
    if (otherNode.posn < lowestPosn) {
      lowestPosn = otherNode.posn;
    }
  }
  if (lowestPosn > ownNode.posn) {
    // We are the winner
    performWinnerTasks();
    otherVectorLock.unlock();
    return;
  }
  unsigned int highestPort = 0;
  for (auto &otherNode : otherVector) {
    if (otherNode.posn == lowestPosn) {
      if (otherNode.port > highestPort) {
        highestPort = otherNode.port;
      }
    }
  }

  if (lowestPosn == ownNode.posn) {
    if (highestPort < ownNode.port) {
      // We are the winner
      performWinnerTasks();
      otherVectorLock.unlock();
      return;
    }
  }

  winner = false;
  cout<<"Loser "<<ownNode.port<<" at "<<ownNode.posn<<endl;
 
  otherVectorLock.unlock();

  return;
}		/* -----  end of function pigFinishElection  ----- */
