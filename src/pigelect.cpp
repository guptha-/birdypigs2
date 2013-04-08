/*==============================================================================
 *
 *       Filename:  pigelect.cpp
 *
 *    Description:  The election process for pigs as well as leader specific 
 *                  tasks.
 *
 * =============================================================================
 */
#include "../inc/piginc.h"

atomic<bool> winner(false);
vector<unsigned int> wallPosns;
mutex wallLock;

extern atomic <unsigned short int> birdPosn;

/* ===  FUNCTION  ==============================================================
 *         Name:  processAffectedPigs
 *  Description:  This calculates the affected pigs and sends them a status msg
 * =============================================================================
 */
void processAffectedPigs ()
{
  set<unsigned short int> affectedPigs;
  bool landOnSomething = false;

  // We hold the other vector and wall locs throughout the calculation. We have
  // to, because the algorithm does not tolerate changes to the DS in between
  otherVectorLock.lock();
  wallLock.lock();
  if (birdPosn == ownNode.posn) {
    affectedPigs.insert(ownNode.port);
    landOnSomething = true;
  }
  for (auto &otherNode : otherVector) {
    if (birdPosn == otherNode.posn) {
      // Bird lands directly on pig
      affectedPigs.insert(otherNode.port);
      landOnSomething = true;
    }
    if (landOnSomething == true) {
      unsigned short int tempLoc = birdPosn - 1;
      while (true) {
        if (tempLoc == 0) {
          break;
        }
        bool flag = false;
        if (tempLoc == ownNode.posn) {
          affectedPigs.insert(ownNode.port);
          flag = true;
        }
        for (auto &nextOtherNode : otherVector) {
          if (nextOtherNode.posn == tempLoc) {
            // The bird landed directly on a pig, and we are following the 
            // chain. If there is one empty slot in the chain, we can see the
            // other way.
            affectedPigs.insert(nextOtherNode.port);
            flag = true;
          }
        }
        if (flag == false) {
          // The chain has been broken because of the absence of a pig
          break;
        }
        tempLoc--;
      }
      tempLoc = birdPosn + 1;
      while (true) {
        if (tempLoc == MAX_POSN) {
          break;
        }
        bool flag = false;
        if (tempLoc == ownNode.posn) {
          affectedPigs.insert(ownNode.port);
          flag = true;
        }
        for (auto &nextOtherNode : otherVector) {
          if (nextOtherNode.posn == tempLoc) {
            // The bird landed directly on a pig, and we are following the 
            // chain. If there is one empty slot in the chain, we can break
            affectedPigs.insert(nextOtherNode.port);
            flag = true;
          }
        }
        if (flag == false) {
          // The chain has been broken because of the absence of a pig
          break;
        }
      }
    }
  }
  for (auto &wallLoc : wallPosns) {
    if (wallLoc == birdPosn) {
      // The bird lands directly on a wall. Note that because a pig cannot
      // topple a wall, this is the only way that a wall can affect pigs. Also,
      // if a pig is on a wall, and the pig adjacent the wall is hit, the pig on
      // the wall is shaken out and it goes on to hit any adjacent pigs. The 
      // wall, however, does not fall.
      unsigned short int tempLoc = birdPosn - 1;
      while (true) {
        if (tempLoc == 0) {
          break;
        }
        bool flag = false;
        if (tempLoc == ownNode.posn) {
          affectedPigs.insert(ownNode.port);
          flag = true;
        }
        for (auto &nextOtherNode : otherVector) {
          if (nextOtherNode.posn == tempLoc) {
            // The bird landed directly on a wall, and we are following the 
            // chain
            affectedPigs.insert(nextOtherNode.port);
            flag = true;
          }
        }
        if ((birdPosn - tempLoc > 2) && (flag == false)) {
          // The chain has been broken because of the absence of a pig. The
          // first condition is to prevent it from breaking out on the two
          // spots adjacent the wall
          break;
        }
        tempLoc--;
      }
      tempLoc = birdPosn + 1;
      while (true) {
        if (tempLoc == MAX_POSN) {
          break;
        }
        bool flag = false;
        if (tempLoc == ownNode.posn) {
          affectedPigs.insert(ownNode.port);
          flag = true;
        }
        for (auto &nextOtherNode : otherVector) {
          if (nextOtherNode.posn == tempLoc) {
            // The bird landed directly on a pig, and we are following the 
            // chain. If there is one empty slot in the chain, we can see the
            // other way.
            affectedPigs.insert(nextOtherNode.port);
            flag = true;
          }
        }
        if ((tempLoc - birdPosn > 2) && (flag == false)) {
          // The chain has been broken because of the absence of a pig
          break;
        }
      }
    }
  }
  wallLock.unlock();
  otherVectorLock.unlock();

  for (auto &ports : affectedPigs) {
    // Send status message
  }

  return;
}		/* -----  end of function processAffectedPigs  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  getWallPosns
 * =============================================================================
 */
static void getWallPosns ()
{
  int numberWalls = rand() % (MAX_WALLS + 1); // Limit number of walls
  // Storing wall positions
  wallLock.lock();
  while (numberWalls--) {
    unsigned int posn;
    while (true) {
      posn = (rand() % MAX_POSN) + 1;
      int duplicate = false;
      for (auto &num : wallPosns) {
        // making sure there is no overlap
        if (num == posn) {
          duplicate = true;
          break;
        }
      }
      if (duplicate == false) {
        break;
      }
    }
    wallPosns.push_back(posn);
  }
  cout<<"Wall posns\t";
  for (auto &wallLoc : wallPosns) {
    cout<<wallLoc<<"\t";
  }
  cout<<endl;
  wallLock.unlock();
}   /* -----  end of function getWallPosns  ----- */

/* ===  FUNCTION  ==============================================================
 *         Name:  performWinnerTasks
 *  Description:  We are the winner. We calculate wall posns, send winner mesg
 *                to bird, and wait for msgs from bird
 * =============================================================================
 */
void performWinnerTasks ()
{
  winner = true;
  
  cout<<"Winner "<<ownNode.port<<" at "<<ownNode.posn<<endl;
  pigSendWinnerMsg();
  getWallPosns();

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
