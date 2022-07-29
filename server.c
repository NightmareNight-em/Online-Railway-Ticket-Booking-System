
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 9870


struct train{
		int train_number;
		char train_name[50];
		int total_seats;
		int available_seats;
		};
struct user{
		int signin_id;
		char password[50];
		char name[50];
		int type;
		};

struct booking{
		int booking_id;
		int type;
		int uid;
		int tid;
		int seats;
		};


void client_call(int sock);
void signin(int client_sock);
void signup(int client_sock);
int menu(int client_sock,int type,int id);
void train_op(int client_sock);
void user_op(int client_sock);
int user_func(int client_sock,int choice,int type,int id);
   
int main(void) {										// FOR ESTABLISHING CONNECTION
 
    int socket_desc, client_sock, c; 
    struct sockaddr_in server, client; 
    char buf[100]; 
  
    socket_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_desc == -1) { 
        printf("Could not create socket"); 
    } 
  
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT); 
   
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) 
        perror("bind failed. Error"); 
   
 
    listen(socket_desc, 3);  
    c = sizeof(struct sockaddr_in); 
  
    while (1){

	    client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c); 
	  
	    if (!fork()){
		    close(socket_desc);
		    client_call(client_sock);								// CALLING CLIENT SERVICE
		    exit(0);
	    }
	    else
	    	close(client_sock);
    }
    return 0;
}

//-------------------- FOR SERVICING CLIENT-----------------------------//
void client_call(int sock){						   // MAIN MENU 
	int choice;
	printf("\n\tClient Connected\n");
	do{
		read(sock, &choice, sizeof(int));		
		if(choice==1)						
			signin(sock);					// CALLING SIGN IN
		if(choice==2)
			signup(sock);					// CALLING SIGN UP
		if(choice==3)
			break;
	}while(1);

	close(sock);
	printf("\n\tClient Disconnected\n");
}

//-------------------- SIGNING-----------------------------//

void signin(int client_sock){
	int fd_user = open("db/db_user",O_RDWR);
	int id,type,valid=0,user_valid=0;
	char password[50];
	struct user u;
	read(client_sock,&id,sizeof(id));			//READING ID SENT BY USER
	read(client_sock,&password,sizeof(password));		// READING PASSWORD SENT BY USER
	
	struct flock lock;
	
	lock.l_start = (id-1)*sizeof(struct user);
	lock.l_len = sizeof(struct user);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	lock.l_type = F_WRLCK;
	fcntl(fd_user,F_SETLKW, &lock);			// PUTTING LOCK
	
	while(read(fd_user,&u,sizeof(u))){
		if(u.signin_id==id){				// MATCHING ID
			user_valid=1;
			if(!strcmp(u.password,password)){	// MATCHING PASSWORD
				valid = 1;
				type = u.type;
				break;
			}
			else{
				valid = 0;
				break;
			}	
		}		
		else{
			user_valid = 0;
			valid=0;				// ID / PASSWORD DIDN'T MATCH
		}
	}
	
	
	//---- THIS LET AGENT LOGIN FROM MULTIPLE ACCOUNT -----//
	if(type!=2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
	
	// SHOWING USER THE LOGIN MENU
	if(user_valid)
	{
		write(client_sock,&valid,sizeof(valid));
		if(valid){
			write(client_sock,&type,sizeof(type));
			while(menu(client_sock,type,id)!=-1);
		}
	}
	else
		write(client_sock,&valid,sizeof(valid));
	
	// THIS WILL NOT LET SAME USER LOGIN FROM ANOTHER TERMINAL 
	if(type==2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
} 

//--------------------SIGNING UP -------//
void signup(int client_sock){
	int fd_user = open("db/db_user",O_RDWR);
	int type,id=0;
	char name[50],password[50];
	struct user u,temp;

	read(client_sock, &type, sizeof(type));		// READING TYPE
	read(client_sock, &name, sizeof(name));		// READING NAME
	read(client_sock, &password, sizeof(password));	// READING PASSWORD

	int fp = lseek(fd_user, 0, SEEK_END);			// GOING TO END OF THE FILE

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();


	fcntl(fd_user,F_SETLKW, &lock);			// PUTTING LOCK


	if(fp==0){						// IF IT'S THE FIRST ENTRY
		u.signin_id = 1;
		strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=type;
		write(fd_user, &u, sizeof(u));
		write(client_sock, &u.signin_id, sizeof(u.signin_id));
	}
	else{							// FOR REST OF THE ENTERIES
		fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
		read(fd_user, &u, sizeof(u));
		u.signin_id++;
		strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=type;
		write(fd_user, &u, sizeof(u));
		write(client_sock, &u.signin_id, sizeof(u.signin_id));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);			// UNLOCKING FILE

	close(fd_user);
	
}

//-------------------- Main menu function-----------------------------//

int menu(int client_sock,int type,int id){
	int choice,ret;

	
	if(type==0){						// FOR ADMIN
		read(client_sock,&choice,sizeof(choice));
		if(choice==1){					// ADMIN OPERATIONS ON TRAIN
			train_op(client_sock);
			return menu(client_sock,type,id);	
		}
		else if(choice==2){				// ADMIN OPERATIONS ON USER
			user_op(client_sock);
			return menu(client_sock,type,id);
		}
		else if (choice ==3)				// Logout
			return -1;
	}
	else if(type==2 || type==1){				// FOR AGENT AND CLIENT
		read(client_sock,&choice,sizeof(choice));
		ret = user_func(client_sock,choice,type,id);
		if(ret!=5)
			return menu(client_sock,type,id);
		else if(ret==5)
			return -1;
	}		
}

//---------------------- ADMIN OPERATIONS ON TRAIN--------------------//

void train_op(int client_sock){
	int valid=0;	
	int choice;
	read(client_sock,&choice,sizeof(choice));
	if(choice==1){  					  	
		char tname[50];
		int tseats;
		int tid = 0;
		read(client_sock,&tname,sizeof(tname));	// READING TRAIN NAME
		read(client_sock,&tseats,sizeof(tseats));	// READING SEATS ADDED BY USER
		struct train tdb,temp;
		struct flock lock;
		int fd_train = open("db/db_train", O_RDWR);
		
		tdb.train_number = tid;
		strcpy(tdb.train_name,tname);
		tdb.total_seats = tseats;			
		tdb.available_seats = tseats;				// FOR A NEW TRAIN AVAILABLE SEATS ARE EQUAL TO BOOKED SEATS

		int fp = lseek(fd_train, 0, SEEK_END); 

		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);			// LOCKING FILE

		if(fp == 0){						// IF IT'S A FIRST TRAIN TO BE ADDED
			valid = 1;
			write(fd_train, &tdb, sizeof(tdb));
			lock.l_type = F_UNLCK;
			fcntl(fd_train, F_SETLK, &lock);
			close(fd_train);
			write(client_sock, &valid, sizeof(valid));
		}
		else{								// REST OF THE TRAIN
			valid = 1;
			lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
			read(fd_train, &temp, sizeof(temp));
			tdb.train_number = temp.train_number + 1;		//INCREASING TRAIN NUMBER 
			write(fd_train, &tdb, sizeof(tdb));
			write(client_sock, &valid,sizeof(valid));	
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);		// UNLOCKING FILE
		close(fd_train);
		
	}

	else if(choice==2){					// VIEW TRAIN ENTERIES
		struct flock lock;
		struct train tdb;
		int fd_train = open("db/db_train", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);		// LOCKING FILE
		int fp = lseek(fd_train, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);		// CALCULATING TOTAL ENTERIES
		write(client_sock, &no_of_trains, sizeof(int));	// SENDING BACK FOR WHILE LOOP

		lseek(fd_train,0,SEEK_SET);
		while(fp != lseek(fd_train,0,SEEK_CUR)){		// LOOP WILL RUN TILL THE END OF THE FILE
			read(fd_train,&tdb,sizeof(tdb));
			write(client_sock,&tdb.train_number,sizeof(int));
			write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			write(client_sock,&tdb.total_seats,sizeof(int));
			write(client_sock,&tdb.available_seats,sizeof(int));
		}
		valid = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);			// UNLOCKING FILE
		close(fd_train);
	}

	else if(choice==3){					// UPDATING TRAIN
		train_op(client_sock);
		int choice,valid=0,tid;
		struct flock lock;
		struct train tdb;
		int fd_train = open("db/db_train", O_RDWR);

		read(client_sock,&tid,sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);

		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);	// GOING TO THAT RECORD WHICH NEEDS TO BE UPDATED
		read(fd_train, &tdb, sizeof(struct train));
		
		read(client_sock,&choice,sizeof(int));
		if(choice==1){							// UPDATING TRAIN NAME
			write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			read(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			
		}
		else if(choice==2){						// UPDATING TOTAL SEATS
			write(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
			read(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
			tdb.available_seats= tdb.total_seats;			// UPDATING TOTAL SEATS WILL ALSO CHANGE THE AVAILABLE SEATS TO SAME VALUE
		}
	
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));			// WRITING UPDATED VALUE
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);				// UNLOCKING FILE
		close(fd_train);	
	}

	else if(choice==4){						// DELETE TRAIN
		train_op(client_sock);
		struct flock lock;
		struct train tdb;
		int fd_train = open("db/db_train", O_RDWR);
		int tid,valid=0;

		read(client_sock,&tid,sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);			// LOCKING FILE
		
		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		strcpy(tdb.train_name,"deleted");			// CHANGING NAME TO DELETED
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);			// UNLOCKING FILE
		close(fd_train);	
	}	
}

//---------------------- ADMIN OPERATIONS ON USER--------------------//
void user_op(int client_sock){
	int valid=0;	
	int choice;
	read(client_sock,&choice,sizeof(choice));
	if(choice==1){    						// ADDING USER
		char name[50],password[50];
		int type;
		read(client_sock, &type, sizeof(type));		// READING TYPE
		read(client_sock, &name, sizeof(name));		// READING NAME
		read(client_sock, &password, sizeof(password));	// READING PASSWORD
		
		struct user udb;
		struct flock lock;
		int fd_user = open("db/db_user", O_RDWR);
		int fp = lseek(fd_user, 0, SEEK_END);
		
		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);			// LOCKING FILE

		if(fp==0){						// FOR FIRST ENTRY
			udb.signin_id = 1;
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			udb.type=type;
			write(fd_user, &udb, sizeof(udb));		// WRITING IN USER FILE
			valid = 1;
			write(client_sock,&valid,sizeof(int));
			write(client_sock, &udb.signin_id, sizeof(udb.signin_id)); // SENDING ID TO USER FOR FUTURE PREFERENCES
		}
		else{									// FOR REST OF THE ENTERIES
			fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
			read(fd_user, &udb, sizeof(udb));
			udb.signin_id++;						// UPDATING ID BY 1 
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			udb.type=type;
			write(fd_user, &udb, sizeof(udb));
			valid = 1;
			write(client_sock,&valid,sizeof(int));			// WRITING IN USER FILE
			write(client_sock, &udb.signin_id, sizeof(udb.signin_id));	// SENDING ID TO USER FOR FUTURE PREFERENCES
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);					// UNLOCKING FILE
		close(fd_user);
		
	}

	else if(choice==2){					// VIEWING CURRENT USERS LIST
		struct flock lock;
		struct user udb;
		int fd_user = open("db/db_user", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);		// UNLOCKING FILE
		int fp = lseek(fd_user, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);		// CALCULATING NUMBER OF USERS
		no_of_users--;
		write(client_sock, &no_of_users, sizeof(int));

		lseek(fd_user,0,SEEK_SET);
		while(fp != lseek(fd_user,0,SEEK_CUR)){			
			read(fd_user,&udb,sizeof(udb));
			if(udb.type!=0){					// DISPLAYING ALL BUT ADMIN
				write(client_sock,&udb.signin_id,sizeof(int));
				write(client_sock,&udb.name,sizeof(udb.name));
				write(client_sock,&udb.type,sizeof(int));
			}
		}
		valid = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);				// UNLOCKING
		close(fd_user);
	}

	else if(choice==3){					// UPDATING USER
		user_op(client_sock);
		int choice,valid=0,uid;
		char pass[50];
		struct flock lock;
		struct user udb;
		int fd_user = open("db/db_user", O_RDWR);

		read(client_sock,&uid,sizeof(uid));

		lock.l_type = F_WRLCK;
		lock.l_start =  (uid-1)*sizeof(struct user);	
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);

		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);	 // REACHING THAT USER
		read(fd_user, &udb, sizeof(struct user));			 // READING UID
		
		read(client_sock,&choice,sizeof(int));
		if(choice==1){							// UPDATING NAME 
			write(client_sock,&udb.name,sizeof(udb.name));
			read(client_sock,&udb.name,sizeof(udb.name));		// READING NEW NAME
			valid=1;
			write(client_sock,&valid,sizeof(valid));		
		}
		else if(choice==2){						// UPDATING PASSWORD
			read(client_sock,&pass,sizeof(pass));
			if(!strcmp(udb.password,pass))
				valid = 1;
			write(client_sock,&valid,sizeof(valid));
			read(client_sock,&udb.password,sizeof(udb.password)); // READING NEW PASSWORD
		}
	
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		if(valid)
			write(client_sock,&valid,sizeof(valid));		// WRITING UPDATED DATA
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);				// UNLOCKING FILE
		close(fd_user);	
	}

	else if(choice==4){						// DELETING ANY USER
		user_op(client_sock);
		struct flock lock;
		struct user udb;
		int fd_user = open("db/db_user", O_RDWR);
		int uid,valid=0;

		read(client_sock,&uid,sizeof(uid));			// READING CLIENT ID

		lock.l_type = F_WRLCK;
		lock.l_start =  (uid-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);		// PUTTING LOCK 
		
		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		strcpy(udb.name,"deleted");			// RENAMING ENTRY NAME TO DELETED
		strcpy(udb.password,"");			// REMOVING PASSSWORD
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);		//UNLOCKING 
		close(fd_user);	
	}
}


//---------------------- USER AND AGENT FUNCTIONS -----------------------//
int user_func(int client_sock,int choice,int type,int id){
	int valid=0;
	if(choice==1){						// TICKET BOOKING
		train_op(client_sock);
		struct flock lockt;				
		struct flock lockb;
		struct train tdb;				// FOR TRAIN DATA BASE
		struct booking bdb;				// FOR BOOKING DATA BASE
		int fd_train = open("db/db_train", O_RDWR);
		int fd_book = open("db/db_booking", O_RDWR);
		int tid,seats;
		read(client_sock,&tid,sizeof(tid));		// READING TRAIN ID
				
		lockt.l_type = F_WRLCK;
		lockt.l_start = tid*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();
		
		lockb.l_type = F_WRLCK;
		lockb.l_start = 0;
		lockb.l_len = 0;
		lockb.l_whence = SEEK_END;
		lockb.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,tid*sizeof(struct train),SEEK_SET);
		
		read(fd_train,&tdb,sizeof(tdb));
		read(client_sock,&seats,sizeof(seats));		// READING NUMBERS OF SEATS NEEDED TO BE BOOKED

		if(tdb.train_number==tid)
		{		
			if(tdb.available_seats>=seats){		// CHECKING IF AVAILABLE SEATS ARE MORE THAN ASKED SEATS
				valid = 1;
				tdb.available_seats -= seats;
				fcntl(fd_book, F_SETLKW, &lockb);
				int fp = lseek(fd_book, 0, SEEK_END);
				
				if(fp > 0){				// FOR REST OF THE ENTERIES
					lseek(fd_book, -1*sizeof(struct booking), SEEK_CUR);
					read(fd_book, &bdb, sizeof(struct booking));
					bdb.booking_id++;
				}
				else 
					bdb.booking_id = 0;		// FOR FIRST ENTRY

				bdb.type = type;			// UPDATING BOOKING RECORD
				bdb.uid = id;
				bdb.tid = tid;
				bdb.seats = seats;
				write(fd_book, &bdb, sizeof(struct booking));
				lockb.l_type = F_UNLCK;
				fcntl(fd_book, F_SETLK, &lockb);	// UNLOCKING
			 	close(fd_book);
			}
		
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(tdb));			// UPDATING TRAIN ENTRY
		}

		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);
		write(client_sock,&valid,sizeof(valid));
		return valid;		
	}
	
	else if(choice==2){							// VIEW BOOKINGS 
		struct flock lock;
		struct booking bdb;
		int fd_book = open("db/db_booking", O_RDONLY);
		int no_of_bookings = 0;
	
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lock);				// LOCKING FILE
	
		while(read(fd_book,&bdb,sizeof(bdb))){
			if (bdb.uid==id)
				no_of_bookings++;				// COUNTING  TOTAL BOOKINGS
		}

		write(client_sock, &no_of_bookings, sizeof(int));
		lseek(fd_book,0,SEEK_SET);

		while(read(fd_book,&bdb,sizeof(bdb))){
			if(bdb.uid==id){						// NOT DISPLAYING UID AND TID
				write(client_sock,&bdb.booking_id,sizeof(int));
				write(client_sock,&bdb.tid,sizeof(int));
				write(client_sock,&bdb.seats,sizeof(int));
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lock);					// UNLOCKING
		close(fd_book);
		return valid;
	}

	else if (choice==3){							// UPDATE BOOKING
		int choice = 2,bid,val;
		user_func(client_sock,choice,type,id);			// DISPLAYING CURRENT BOOKINGS
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		int fd_book = open("db/db_booking", O_RDWR);			// OPENING BOOKING DATABASE
		int fd_train = open("db/db_train", O_RDWR);			// OPENING TRAIN DATABASE
		read(client_sock,&bid,sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid*sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid)*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		read(client_sock,&choice,sizeof(choice));
	
		if(choice==1){							// INCREASING NUMBER OF SEATS
			read(client_sock,&val,sizeof(val));
			if(tdb.available_seats>=val){
				valid=1;
				tdb.available_seats -= val;
				bdb.seats += val;
			}
		}
		else if(choice==2){						// DECREASING NUMBER OF SEATS
			int temp_seat;
			valid=1;
			read(client_sock,&val,sizeof(val));
			temp_seat=bdb.seats;
			bdb.seats -= val;
			if(bdb.seats<0)					// TO HANDLE IF BOOKING SEATS GOING TO BE NEGATIVE
			{
			bdb.seats=0;
			tdb.available_seats += temp_seat;
			}
			else
			{
			tdb.available_seats += val;
			}
				
		}
		
		write(fd_train,&tdb,sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);			// UNLOCKING BOTH FILE
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);
		
		write(client_sock,&valid,sizeof(valid));
		return valid;
	}
	else if(choice==4){							// CANCEL BOOKING
		int choice = 2,bid;
		user_func(client_sock,choice,type,id);
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		int fd_book = open("db/db_booking", O_RDWR);
		int fd_train = open("db/db_train", O_RDWR);
		read(client_sock,&bid,sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid*sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid)*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		tdb.available_seats += bdb.seats;				// INCREASING AVAILABLE
		bdb.seats = 0;
		valid = 1;

		write(fd_train,&tdb,sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);
		
		write(client_sock,&valid,sizeof(valid));
		return valid;
		
	}
	else if(choice==5)										// LOGGING OUT
		return 5;

}
