#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* Statistics 
 * Do NOT change the name/declaration of these variables
 * You need to set the value of these variables appropriately within your code.
 * */
int A_application = 0;
int A_transport = 0;
int B_application = 0;
int B_transport = 0;
float time = 0.000;// for time stamps values.
void starttimer(int, float);
void stoptimer(int);
void tolayer3(int,struct pkt);
void tolayer5(int,char[]);

/* Globals 
 * Do NOT change the name/declaration of these variables
 * They are set to zero here. You will need to set them (except WINSIZE) to some proper values.
 * */
float TIMEOUT = 30.0;// I have not used this value though I went with the hardcoding.  
int WINSIZE;         //This is supplied as cmd-line parameter; You will need to read this value but do NOT modify it's value; 
int SND_BUFSIZE = 0; //Sender's Buffer size
int RCV_BUFSIZE = 0; //Receiver's Buffer size
int base=0;
int packnum=0;
int A=0;
int B=1;
int counter=0;
struct pkt buffer[2000];
struct pkt recvbuffer[2000];
int doubleackcheck=-1;

int m=0;

int pp=0;
int l=0;
int lastsent=0;
struct pkt sendbuffer[2000];
float ACK_RCV[2000][2];

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
		A_application++;
		struct pkt send_packet;
		send_packet.seqnum=packnum;
		send_packet.acknum=send_packet.seqnum;
		send_packet.checksum=send_packet.seqnum+send_packet.acknum;
		int i=0;
		strncpy(send_packet.payload,message.data,20);
		for(i=0;i<20;i++)
		{
			send_packet.checksum=send_packet.checksum+(int) send_packet.payload[i];
		}
		printf("A checksum:%d \n",send_packet.checksum);
		sendbuffer[packnum].seqnum=send_packet.seqnum;
		sendbuffer[packnum].acknum=send_packet.acknum;
		sendbuffer[packnum].checksum=send_packet.checksum;
		strncpy(sendbuffer[packnum].payload,send_packet.payload,20);
		int j=0;
		for(i=lastsent;i<packnum;i++)
		{
			if((lastsent+1)<(base+WINSIZE) & sendbuffer[lastsent+1].seqnum!=0)
			{
				tolayer3(A,sendbuffer[lastsent+1]);
				A_transport++;
				printf("buffer sent\n");
				printf("last sent seqnum:%d \n",sendbuffer[lastsent+1].seqnum);
				printf("last sent add:%d \n",(lastsent+1));
				printf("time :%f \n",time);
				ACK_RCV[lastsent+1][1]=time;
				ACK_RCV[lastsent+1][0]=0;
				if(base==lastsent)
				starttimer(A,30);
				printf("time buffer:%f \n",ACK_RCV[send_packet.seqnum][1]);
				printf("ack status:%f \n",ACK_RCV[lastsent+1][0]);
				lastsent++;
			}
		}
		packnum++;
		
	if((packnum-1)<(base+WINSIZE))
	{
		tolayer3(A,send_packet);
		printf("base :%d\n",base);
		printf("packnum: %d\n",(packnum-1));
		if((packnum-1)==base)
		{
		starttimer(A,30);
		}
		ACK_RCV[send_packet.seqnum][1]=time+(5*(float)j);
		ACK_RCV[send_packet.seqnum][0]=0;
		
		//packnum++;
		lastsent=send_packet.seqnum;
		A_transport++;
	}
}



void B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int check=packet.acknum+packet.seqnum;
	int i=0;
	for(i=0;i<20;i++)
	{
		check=check+(int) packet.payload[i];
	}
	printf("A input checksum: %d\n",check);
	printf("doubleackcheck: %d \n",doubleackcheck);
	printf("A input packet checksum:%d \n",packet.checksum);
	printf("A input packet acknum: %d \n",packet.acknum);
	if(packet.checksum==check)
	{
		int temp=base;
		int flag=0;
		if(packet.seqnum==base)
		{
			ACK_RCV[base][0]=1;
			base=base+1;
			if(base>(packnum-1)){
			stoptimer(A);
			flag=1;
			}
			for(i=0;i<WINSIZE;i++)
			{
				if(ACK_RCV[base][0]==1 & sendbuffer[base].seqnum!=0)
				{
				if(base>(packnum-1) && flag==0)
				{
					stoptimer(A);
					break;
				}
				else
					base++;
				}
			else 
			break;
			}
			printf("base: %d \n",base);
		}
		else{
		ACK_RCV[packet.seqnum][0]=1;
		}
		
		}
	else
	{
		printf("ACK is corrupted.\n");
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	int i=0;
	int flag=0;
	int flag1=0;
	for(i=0;i<WINSIZE;i++)
	{
		printf("before loop ack time: %f\n",ACK_RCV[(base+i)][1]);
		printf("ackvalue:%f\n",ACK_RCV[base+i][0]);
		printf("seqnum:%d \n",sendbuffer[(base+i)].seqnum);
		printf("base: %d \n",base);
		if(time>=(ACK_RCV[(base+i)][1]+29) && (ACK_RCV[(base+i)][1]!=-1) && (ACK_RCV[(base+i)][0]==0) && sendbuffer[(base+i)].seqnum>=base)
		{
			//printf("yammmm\n");
			if((time>=(ACK_RCV[(base+i)][1]+29) && flag==0 && (ACK_RCV[(base+i)][1]!=0))){
			tolayer3(A,sendbuffer[(base+i)]);
			A_transport++;
		
			printf("interrupt seq num: %d \n",sendbuffer[(base+i)].seqnum);
			ACK_RCV[(base+i)][1]=time;
			ACK_RCV[(base+i)][0]=0;
			printf("time: %f\n",time);
			flag=1;
			}
		}
			else if(ACK_RCV[(base+i)][0]==0 && flag==1 && (ACK_RCV[(base+i)][1]!=0) && sendbuffer[(base+i)].seqnum>=base)
			{
			//printf("this is happening.\n");
			printf("ack time: %f\n",ACK_RCV[(base+i)][1]);
			flag1=1;
			printf("next ack time: %f\n",ACK_RCV[(base+i)][1]);
			starttimer(A,ACK_RCV[(base+i)][1]+30-time);
			break;
			}
		}
		if(flag1==0 && ACK_RCV[(base)][0]==0 && (ACK_RCV[(base)][1]!=0) && sendbuffer[(base)].seqnum>=base){
		//printf("You are a sucker.\n");
		starttimer(A,ACK_RCV[(base)][1]+30-time);
		}
	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	int i=0;
	int j=0;
	for(i=0;i<2000;i++)
	{
		ACK_RCV[i][0]=-1;
		ACK_RCV[i][1]=0;
		sendbuffer[i].acknum=-1;
		sendbuffer[i].seqnum=-1;
	}
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    B_transport++;
	int check=packet.seqnum+packet.acknum;
	int i=0;
	for(i=0;i<20;i++)
	{
		check=check+(int) packet.payload[i];
	}
	printf("check:%d\n",check);
	printf("packet checksum: %d \n",packet.checksum);
	printf("packet seqnum:%d \n",packet.seqnum);
	printf("packet acknum:%d \n",packet.acknum);
	if(check==packet.checksum)
	{
		recvbuffer[0].seqnum=-1;
		struct pkt ackpkt;
		int k=0;
		if(recvbuffer[packet.seqnum].seqnum!=0)
		k=1;
		ackpkt.acknum=packet.seqnum;
		ackpkt.seqnum=packet.seqnum;
		ackpkt.checksum=ackpkt.seqnum+ackpkt.acknum;
		for(i=0;i<20;i++)
		{
			ackpkt.checksum=ackpkt.checksum+(int) ackpkt.payload[i];
		}
		struct msg receivedData;
		recvbuffer[ackpkt.seqnum].seqnum=ackpkt.seqnum;
		recvbuffer[ackpkt.acknum].seqnum=ackpkt.acknum;
		strncpy(recvbuffer[ackpkt.seqnum].payload,packet.payload,20);
		recvbuffer[ackpkt.seqnum].checksum=recvbuffer[ackpkt.seqnum].seqnum+recvbuffer[ackpkt.seqnum].acknum;
		
		//strncpy(lastack.payload,packet.payload,20);
		strncpy(receivedData.data,packet.payload,20);
		for(i=0;i<20;i++)
		{
			recvbuffer[ackpkt.seqnum].checksum=recvbuffer[ackpkt.seqnum].checksum+(int) recvbuffer[ackpkt.seqnum].payload[i];
		}
		int count=0;
		
		for(count=counter;count<2000;count++)
		{
			if(recvbuffer[count].seqnum==counter)
			{
				tolayer5(B,recvbuffer[count].payload);
				counter++;
			}
			else
			{
				break;
			}
		}
		//tolayer5(B,receivedData.data);
		tolayer3(B,ackpkt);
		if(k==0)
		B_application++;
		

	}
	else{

	}
	


}

/* called when B's timer goes off */
void B_timerinterrupt()
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

//forward declarations
void init();
void generate_next_arrival();
void insertevent(struct event*);

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float lossprob = 0.0;	   /* probability that a packet is dropped */
float corruptprob = 0.0;   /* probability that one bit is packet is flipped */
float lambda = 0.0; 	   /* arrival rate of messages from layer 5 */
int ntolayer3 = 0; 	   /* number sent into layer 3 */
int nlost = 0; 	  	   /* number lost in media */
int ncorrupt = 0; 	   /* number corrupted by media*/

/**
 * Checks if the array pointed to by input holds a valid number.
 *
 * @param  input char* to the array holding the value.
 * @return TRUE or FALSE
 */
int isNumber(char *input)
{
    while (*input){
        if (!isdigit(*input))
            return 0;
        else
            input += 1;
    }

    return 1;
}

int main(int argc, char **argv)
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;
   
   int i,j;
   char c;

   int opt;
   int seed;

   //Check for number of arguments
   if(argc != 5){
   	fprintf(stderr, "Missing arguments\n");
	printf("Usage: %s -s SEED -w WINDOWSIZE\n", argv[0]);
   	return -1;
   }

   /* 
    * Parse the arguments 
    * http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html 
    */
   while((opt = getopt(argc, argv,"s:w:")) != -1){
       switch (opt){
           case 's':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -s\n");
                           return -1;
                       }
                       seed = atoi(optarg);
                       break;

           case 'w':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -w\n");
                           return -1;
                       }
                       WINSIZE = atoi(optarg);
                       break;

           case '?':   break;

           default:    printf("Usage: %s -s SEED -w WINDOWSIZE\n", argv[0]);
                       return -1;
       }
   }
  
   init(seed);
   A_init();
   B_init();
   
   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++) 
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A) 
               A_output(msg2give);  
             else
               B_output(msg2give);  
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) 
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
	//Do NOT change any of the following printfs
	printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
	
	printf("\n");
	printf("Protocol: SR\n");
	printf("[PA2]%d packets sent from the Application Layer of Sender A[/PA2]\n", A_application);
	printf("[PA2]%d packets sent from the Transport Layer of Sender A[/PA2]\n", A_transport);
	printf("[PA2]%d packets received at the Transport layer of Receiver B[/PA2]\n", B_transport);
	printf("[PA2]%d packets received at the Application layer of Receiver B[/PA2]\n", B_application);
	printf("[PA2]Total time: %f time units[/PA2]\n", time);
	printf("[PA2]Throughput: %f packets/time units[/PA2]\n", B_application/time);
	return 0;
}



void init(int seed)                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();
  
  
   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(seed);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" ); 
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   //time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() 
{
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */ 
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}  

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
    //char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
 
   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
} 


void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime); 
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q; 
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;
 //char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)  
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }
 
/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
void tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 //char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)    
	printf("          TOLAYER3: packet being lost\n");
      return;
    }  

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */ 
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();
 


 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)    
	printf("          TOLAYER3: packet being corrupted\n");
    }  

  if (TRACE>2)  
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
} 

void tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)  
        printf("%c",datasent[i]);
     printf("\n");
   }
  
}
