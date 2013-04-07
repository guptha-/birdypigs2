#ifndef inc_COM_CONST
#define inc_COM_CONST

#define BIRD_LISTEN_PORT 11200
#define COM_IP_ADDR "127.0.0.1"
#define MAX_WALLS 3
#define MAX_POSN 20
#define MAX_SLEEP 3

// Messages
#define MSG_TYPE_SIZE 2
#define PORT_SIZE 2
#define POSN_SIZE 2
#define MAX_MSG_SIZE 1000

enum MessageTypes {
  ELECT_OFFER_MSG = 1,
  ELECT_WINNER_MSG,
  BIRD_POSN_MSG,
  BIRD_LAND_MSG
};
#endif

