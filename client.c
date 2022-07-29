#include <string.h>
#include <sys/socket.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdio.h>
#include <stdlib.h> 

	 
#define PORT 9870


int client(int sock);
int menu(int sock,int type);
int user_func(int sock,int choice);
int train_op(int sock,int choice);
int user_op(int sock,int choice);

int main(void) { 
	int sock; 
    	struct sockaddr_in server; 
    	char server_reply[50],*server_ip;
	server_ip = "127.0.0.1"; 
     
    	sock = socket(AF_INET, SOCK_STREAM, 0); 
    	if (sock == -1) { 
       	printf("Could not create socket"); 
    	} 
    
    	server.sin_addr.s_addr = inet_addr(server_ip); 
    	server.sin_family = AF_INET; 
    	server.sin_port = htons(PORT); 
   
    	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
       	perror("connect failed. Error"); 
    
	while(client(sock)!=3);
    	close(sock); 
    	
	return 0; 
} 

//-------------------- First function which is called-----------------------------//

int client(int sock){
	int choice,valid;
	system("clear");
	printf("\t\t***************************************\n")	;
	
	printf("\n\t\t\tTICKET BOOKING SYSTEM\n\n");
	printf("\t\t****************************************\n\n")	;

	printf("\t* Press 1 for Sign In\n");
	printf("\t* Press 2 for Sign Up/ Register\n");
	printf("\t* Press 3 for Exit\n");
	printf("\t  Enter Your Choice: ");
	scanf("%d", &choice);
	write(sock, &choice, sizeof(choice));
	if (choice == 1){					// LOGGING IN
		int id,type,num;
		char password[50];
		printf("\tLOGIN ID:: ");
		scanf("%d", &id);
		strcpy(password,getpass("\tPASSWORD: "));
		write(sock, &id, sizeof(id));
		write(sock, &password, sizeof(password));
		read(sock, &valid, sizeof(valid));
		if(valid){
			printf("\tlogin successfully\n");
			read(sock,&type,sizeof(type));
			while(menu(sock,type)!=-1);
			
			return 1;
		}
		else{
			printf("\tLogin Failed : Incorrect password or login id\n");
			printf("\t Enter any kew to continue\n");
			scanf("%d", &num);
			return 1;
		}
	}

	else if(choice == 2){					// SIGNING UP
		int type,id,num;
		char name[50],password[50],secret_pin[6];
		system("clear");
		printf("\n\t\t\t\tSIGN UP \n");
		printf("\n\t\t\t\t******* \n");
		printf("\n\tDo you want to sign up as?: \n");
		printf("\t* Press 0 for Admin\n\t* Press 1 for Agent\n\t* Press 2 for Customer\n\t* Press 3 for Main Menu\n");
		printf("\tYour Response: ");
		scanf("%d", &type);
		if(type == 3 )
		{
		return 2;
		}
		printf("\tEnter Name:- ");
		scanf("%s", name);
		strcpy(password,getpass("\tEnter Password:- "));

		
		write(sock, &type, sizeof(type));
		write(sock, &name, sizeof(name));
		write(sock, &password, strlen(password));
		
		read(sock, &id, sizeof(id));
		printf("\tYOUR LOGIN ID IS: %d\n", id);
		printf("\n\tEnter any number for main menu ");
		scanf("%d",&num);
		return 2;
	}
	else							// LOGGING OUT
		return 3;
	
}

//-------------------- MAIN MENU-----------------------------//

int menu(int sock,int type){
	int choice;
	system("clear");
	if(type==2 || type==1){					// FOR AGENT AND CUSTOMER
		if(type==2){
		printf("\n\t\t\t\tWELCOME USER \n");
		}
		if(type==1)
		{
		printf("\n\t\t\t\tWELCOME AGENT\n");
		}
		printf("\n\t\t\t\t*************\n");
		
		
		printf("\t* Press 1 for Ticket Booking\n");
		printf("\t* Press 2 for View Bookings\n");
		printf("\t* Press 3 for Update Booking\n");
		printf("\t* Press 4 for Cancel booking\n");
		printf("\t* Press 5 to Logout\n");
		printf("\tEnter Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
		return user_func(sock,choice);
	}
	else if(type==0){
							// FOR ADMIN
		printf("\n\t\t\t\tWELCOME ADMIN \n");
		printf("\t\t\t\t*************\n");
		printf("\n\t* Press 1 for Train Operations\n");
		printf("\t* Press 2 for User Operations\n");
		printf("\t* Press 3 for Main Menu\n");
		printf("\t Enter Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
			if(choice==1){
				printf("\n\t\t\t\tTRAIN OPERATIONS MENU \n");
				printf("\t\t\t\t***********************\n");
				printf("\t* Press 1 for Add Train\n");
				printf("\t* Press 2 for View Train\n");
				printf("\t* Press 3 for Modify Train Entry\n");
				printf("\t* Press 4 for Delete Train Entry\n");
				printf("\t* Press 5 for Admin Menu\n");
				
				printf("\t Enter Choice: ");
				scanf("%d",&choice);	
				if(choice ==5)
				{
				menu(sock,type);
				}
				else
				{
				write(sock,&choice,sizeof(choice));
				return train_op(sock,choice);
				}
			}
			else if(choice==2){
				printf("\n\t\t\t\tUSER OPERATIONS MENU\n");
				printf("\t\t\t\t**********************\n");
				printf("\t* Press 1 for Add User\n");
				printf("\t* Press 2 for View Users\n");
				printf("\t* Press 3 for Modify User\n");
				printf("\t* Press 4 for Delete User\n");
				printf("\t* Press 5 for Admin Menu\n");
				printf("\t Enter Choice: ");
				scanf("%d",&choice);
				if(choice ==5)
				{
				menu(sock,type);
				}
				else
				{
				write(sock,&choice,sizeof(choice));
				return user_op(sock,choice);
				}
			
			}
			else if(choice==3)
				return -1;
	}	
	
}

//-------------------------------- ADMIN OPERATIONS ON TRAIN-----------------------------//
int train_op(int sock,int choice){
	int valid = 0;
	int num=0;
	if(choice==1){				// ADDING TRAIN
		char tname[50];
		int number;
		int tseats;
		printf("\n\tEnter Name  ");
		scanf("%s",tname);
		printf("\n\tEnter Total seats  ");
		scanf("%d",&tseats);
		
		write(sock, &tname, sizeof(tname));
		write(sock, &tseats, sizeof(tseats));
		
		read(sock,&valid,sizeof(valid));
			
		if(valid)
			printf("\n\tTrain added successfully\n");
		printf("\tEnter any key number to return ");
		scanf("%d",&num);
		return valid;	
	}
	
	else if(choice==2){			// VIEWING TRAIN
		int no_of_trains;
		int tno;
		char tname[50];
		int tseats;
		int aseats;
		read(sock,&no_of_trains,sizeof(no_of_trains));
		printf("\n\t AVAILABLE TRAINS \n");
		
		printf("\tT_no\tT_name\tT_seats\tA_seats\n");
		while(no_of_trains--){
			read(sock,&tno,sizeof(tno));
			read(sock,&tname,sizeof(tname));
			read(sock,&tseats,sizeof(tseats));
			read(sock,&aseats,sizeof(aseats));
			
			if(strcmp(tname, "deleted")!=0)
				printf("\t%d\t%s\t%d\t%d\n",tno,tname,tseats,aseats);
		}
		printf("\tEnter any Number to continue ");
		scanf("%d",&num);
		return valid;	
	}
	
	else if (choice==3){			// UPDATING TRAIN
		int tseats,choice=2,valid=0,tid;
		char tname[50];
		write(sock,&choice,sizeof(int));
		train_op(sock,choice);
		printf("\n\tEnter the train number : ");
		scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
		
		printf("\n\t* Press 1 to modify Train Name\n\t* Press 2 to modify Total Seats\n");
		printf("\t Your Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
		
		if(choice==1){
			read(sock,&tname,sizeof(tname));
			printf("\n\t Current name: %s",tname);
			printf("\n\t Updated name:");
			scanf("%s",tname);
			write(sock,&tname,sizeof(tname));
		}
		else if(choice==2){
			read(sock,&tseats,sizeof(tseats));
			printf("\n\t Current seats: %d",tseats);
			printf("\n\t Updated updated:");
			scanf("%d",&tseats);
			write(sock,&tseats,sizeof(tseats));
		}
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\t DATA HAS BEEN UPDATED\n");
		printf("\tEnter any number to return ");
		scanf("%d",&num);
		return valid;
	}

	else if(choice==4){				// DELETING TRAIN
		int choice=2,tid,valid=0;
		write(sock,&choice,sizeof(int));
		train_op(sock,choice);
		
		printf("\n\t Enter the train number : ");
		scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\t Train deleted successfully\n");
		printf("Enter any key to return ");
		scanf("%d",&num);
		return valid;
	}
	
}

//-------------------------------- ADMIN OPERATIONS ON USER-----------------------------//

int user_op(int sock,int choice){
	int valid = 0,num;
	char ch;
	if(choice==1){							// ADDING USER
		int type,id;
		
		char name[50],password[50];
		printf("\n\tEnter The Type Of Account: \n");
		printf("\t* Press 1 to add an Agent\n\t* Press 2 to add Customer\n");
		printf("\tEnter Choice: ");
		scanf("%d", &type);
		printf("\tEnter Name: ");
		scanf("%s", name);
		strcpy(password,getpass("\tPassword: "));
		write(sock, &type, sizeof(type));
		write(sock, &name, sizeof(name));
		write(sock, &password, strlen(password));
		read(sock,&valid,sizeof(valid));	
		if(valid){
			read(sock,&id,sizeof(id));
			printf("\tYOUR LOGIN ID IS %d\n", id);
			printf("\tEnter any  number to return ");
			scanf("%d",&num);
		}
		return valid;	
	}
	
		else if(choice==2){						// VIEWING CURRENT USERS
		int no_of_users;
		int id,type;
		char uname[50],typechar[50];
		read(sock,&no_of_users,sizeof(no_of_users));

		printf("\tU_id\tU_name\tU_type\n");
		while(no_of_users--){
			read(sock,&id,sizeof(id));
			read(sock,&uname,sizeof(uname));
			read(sock,&type,sizeof(type));
			if(type==2)
			{
			strcpy(typechar,"Customer");
			}
			if(type==1)
			{
			strcpy(typechar,"Admin");
			}
			if(strcmp(uname, "deleted")!=0)
				printf("\t%d\t%s\t%s\n",id,uname,typechar);
		}
		printf("\tEnter any  number to continue ");
			scanf("%d",&num);
		return valid;	
	}

	else if (choice==3){						// UODATING USERS
		int choice=2,valid=0,uid;
		char name[50],pass[50];
		write(sock,&choice,sizeof(int));
		user_op(sock,choice);
		printf("\n\tEnter the U_id you want to modify: ");
		scanf("%d",&uid);
		write(sock,&uid,sizeof(uid));
		
		printf("\n\t* Press 1 to change User Name\n\t* Press 2 to update Password\n");
		printf("\t Your Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
		
		if(choice==1){
			read(sock,&name,sizeof(name));
			printf("\n\t Current User name is:- %s",name);
			printf("\n\t Enter New name: ");
			scanf("%s",name);
			write(sock,&name,sizeof(name));
			read(sock,&valid,sizeof(valid));
		}
		else if(choice==2){
			printf("\n\t Enter Current password: ");
			scanf("%s",pass);
			write(sock,&pass,sizeof(pass));
			read(sock,&valid,sizeof(valid));
			if(valid){
				//printf("\n\t Enter new password:");
				//scanf("%s",pass);
				strcpy(pass,getpass("\tEnter new password: "));
			}
			else
				printf("\n\tEnter Correct password\n");
			
			write(sock,&pass,sizeof(pass));
		}
		if(valid){
			read(sock,&valid,sizeof(valid));
			if(valid)
				printf("\n\t DATA HAS BEEN UPDATED\n");
		}
				printf("\tEnter any  number to return ");
			scanf("%d",&num);
		return valid;
	}

	else if(choice==4){						// DELETING A USER
		int choice=2,uid,valid=0;
		write(sock,&choice,sizeof(int));
		user_op(sock,choice);
		
		printf("\n\t Enter the id you want to delete: ");
		scanf("%d",&uid);
		write(sock,&uid,sizeof(uid));
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\t User deleted successfully\n");
			printf("\tEnter any  number to return ");
			scanf("%d",&num);
		return valid;
	}
}

//-------------------------------- USER FUNCTION TO BOOK TICETS -----------------------------//
int user_func(int sock,int choice){
	int valid =0,num;
	if(choice==1){										// BOOKING TICKETS
		int view=2,tid,seats;
		write(sock,&view,sizeof(int));
		train_op(sock,view);
		printf("\n\tEnter the Train NUmber: ");
		scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
				
		printf("\n\tEnter the no. of seats: ");
		scanf("%d",&seats);
		write(sock,&seats,sizeof(seats));
	
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tTicket booked successfully.\n");
		else
			printf("\n\tSeats were not available.\n");

		printf("\tEnter any  number to return ");
			scanf("%d",&num);
		return valid;
	}
	
	else if(choice==2){									// VIEWING THE BOOKINGS
		int no_of_bookings;
		int id,tid,seats;
		read(sock,&no_of_bookings,sizeof(no_of_bookings));

		printf("\tB_id\tT_no\tSeats\n");
		while(no_of_bookings--){
			read(sock,&id,sizeof(id));
			read(sock,&tid,sizeof(tid));
			read(sock,&seats,sizeof(seats));
			
			if(seats!=0)
				printf("\t%d\t%d\t%d\n",id,tid,seats);
		}
			printf("\tEnter any  number to continue ");
			scanf("%d",&num);
		return valid;
	}

	else if(choice==3){									// UPDATING A BOOKING
		int choice = 2,bid,val,valid;
			printf("\n\t YOUR CURRENT BOOKINGS \n");
		user_func(sock,choice);
		
	
		
		
		printf("\n\t Enter the B_id you want to modify: ");
		scanf("%d",&bid);
		write(sock,&bid,sizeof(bid));

		printf("\n\t* Press 1 to increase number of seats\n\t* Press 2 to  Decrease number of seats\n");
		printf("\t Enterl Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));

		if(choice==1){
			printf("\n\tNo. of tickets to increase ");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
		}
		else if(choice==2){
			printf("\n\tNo. of tickets to decrease ");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
		}
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tBooking updated successfully.\n");
		else
			printf("\n\tUpdation failed. No more seats available.\n");
		
		printf("\tEnter any  number to return ");
			scanf("%d",&num);
		return valid;
	}
	
	else if(choice==4){									// CANCEL BOOKING
		int choice = 2,bid,valid;
		printf("\n\t YOUR CURRENT BOOKINGS \n");
		user_func(sock,choice);
	
		
		printf("\n\t Enter the B_id you want to cancel: ");
		scanf("%d",&bid);
		write(sock,&bid,sizeof(bid));
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tBooking cancelled successfully.\n");
		else
			printf("\n\tCancellation failed.\n");
		printf("\tEnter any  number to return ");
			scanf("%d",&num);
		return valid;
	}
	else if(choice==5)									// LOGGING OUT
		return -1;
	
}
