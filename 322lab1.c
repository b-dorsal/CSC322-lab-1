#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


/*
	CSC-322 Lab 1
	322lab1.c - Game project in C
	Purpose: The project consists of building a small virtual world simulation that allows a human user to interact with
			it through a text-based, command-line interface.
 
	@author Brian Dorsey
	@version 2/19/2017

	Array column descriptions
	roomlist = {0state, 1n, 2s, 3e, 4w, 5occupancy}
	roomlist = {0type, 1room}
*/

int respect = 40;
int creaturect = 0;
int roomct = 0;

//DEBUG --------------------------------------------
//Prints all array and game state info for debug ony.
void printArrData(int** roomlist, int** creaturelist){
	printf("\n---------------DEBUG DATA---------------\n");
	printf("Respect: %d\n", respect);
	printf("creature ct: %d\n", creaturect);
	printf("room ct: %d\n", roomct);
	for(int x = 0; x<roomct; x++){
		printf("%d: State: %d",x, roomlist[x][0]);
		printf(" n: %d", roomlist[x][1]);
		printf(" s: %d", roomlist[x][2]);
		printf(" e: %d", roomlist[x][3]);
		printf(" w: %d", roomlist[x][4]);
		printf(" occu.: %d\n", roomlist[x][5]);
	}

	for(int x = 0; x<creaturect; x++){
		printf("%d: type: %d",x, creaturelist[x][0]);
		printf(" location: %d\n", creaturelist[x][1]);
	}
	printf("---------------DEBUG DATA---------------\n");
}
//DEBUG --------------------------------------------

//Print the help information.
void printHelp(){
	printf("\n\nInsructions:\nAll commands are one or two words. Two word commands are seperated by ':'.");
	printf("Single word commands control the player, and two word commands control the creatures around you.\n");
	printf("To move your player, type north, south, east, or west.");
	printf("To move a creature, type the creatures id and the direction seperated by a ':'. ex name:n\n");
	printf("To clean or dirty the current room, type \"clean\" or \"dirty\".");
	printf("Or, to make a creature clean the current room, type it's name followed by \"clean\" or \"dirty\", seperated by a ':'\n");
	printf("To look around your current room, and view its contents, type \"look\".");
	printf("Or, to make a creature look around the current room, type it's name followed by \"look\", seperated by a ':'. \n");
	printf("All commands are not case sensitive, creature names, however, are.\n");
	printf("For help, type \"help\"");
	printf("To quit, type \"quit\" or \"exit\".\n");
}

//Handle two-part commands------------------------
//Checks to see if a command contains a split.
int containsSplit(char string[256]){
	for(int i = 0; string[i] != '\0'; ++i){
		if(string[i] == ':'){
			return 1;
		}
	}
	return 0;
}

//Get the user ID from before the colon.
char* splitID(char command[256]){

	char id[128];
	int i = 0;
	for(i = 0; command[i] != '\0'; ++i){
		id[i] = command[i];
		if(command[i] == ':'){
			id[i] = '\0';
			break;
		}
	}
	return &id;
}

//Get the command type from behind the colon.
char* splitComm(char command[256]){

	char comm[128];
	int rec = 0;
	int pos = 0;
	int i = 0;
	for(i = 0; command[i] != '\0'; ++i){
		if(rec == 1){
			comm[pos] = command[i];
			pos++;
		}
		if(command[i] == ':'){
			rec = 1;
		}
	}
	comm[pos] = '\0';
	return &comm;
}
//--------------------------------------------

//Get the PC's ID
int getPlayerID(int** creaturelist){
	int x;
	for(x = 0; x < creaturect; x++){
		if(creaturelist[x][0] == 0){
			return x;
		}
	}
	return x;
}

//Returns the current location of the player.
int getPlayerRoom(int** creaturelist){
	int currRoom = -1;
	currRoom = creaturelist[getPlayerID(creaturelist)][1];	
	return currRoom;
}

//Checks to see if a move is valid.
int checkMove(int dir, int** roomlist, int currRoom){

	int retVal = 1;
	if(roomlist[currRoom][dir] == -1){
		retVal = 0;
	}

	if(roomlist[currRoom][5] >= 10){
		retVal = 0;
	}
	return retVal;
}

//Finds a legal move for the creature.
int findGoodMove(int** roomlist, int currRoom){

	int dir = randomMove();
	
	for(int x = 0; x < 4; x++){
		if(checkMove(dir, roomlist, currRoom) == 1){
			return dir;
		}else{
			dir++;
		}

		if(dir > 4){
			dir = 1;
		}
	}

	return -1;

}

//Handles a creature leaving the house completely.
void escapeHouse(int creatureID, int** creaturelist, int** roomlist){
	printf("\n%d drills a hole and leaves the house.\n", creatureID);

	int currRoom = creaturelist[creatureID][1];
	roomlist[currRoom][5] = roomlist[currRoom][5] - 1;
	//creaturect--;

	for(int x = 0; x < 2; x++){
		creaturelist[creatureID][x] = -1;
	}

	//Cause reaction from other creatures in room.
	for(int x = 0; x < creaturect; x++){
		if(creaturelist[x][0] != 0){
			if(creaturelist[x][1] == currRoom){
				if(creaturelist[x][0] == 1){
					printf("Animal %d growls\n", x);
					respect = respect - 1;
					printf("Respect is now %d\n", respect);
				}else{
					printf("Human %d growls\n", x);
					respect = respect - 1;
					printf("Respect is now %d\n", respect);
				}
				
			}
		}
	}
}

//Handles a creature leaving a room because of state change.
void leaveRoom(int actor, int** roomlist, int** creaturelist){
	int dir = findGoodMove(roomlist, creaturelist[actor][1]);
	if(dir == -1){
		//escape house
		escapeHouse(actor, creaturelist, roomlist);
	}else{
		move(actor, dir, roomlist, creaturelist);
		printf("\n%d leaves to the ", actor);
		if(dir == 1){
			printf("north");
		}else if(dir == 2){
			printf("south");
		}else if(dir == 3){
			printf("east");
		}else if(dir == 4){
			printf("west");
		}

	}
}

//Cause a reaction to a room state change.
void causeReaction(int actor, int cleandirty, int** roomlist, int** creaturelist, int currRoom){
	for(int c = 0; c < creaturect; c++){
		if(creaturelist[c][1] == currRoom){
			if(creaturelist[c][0] == 1){
				//animal
				int scoreval = 1;
				if(actor == c){	
					scoreval = scoreval * 3;
				}

				if(cleandirty == 0){
					//happy
					printf("Animal %d licks your face.", c);

					if(actor == c){
						printf(" a lot");
					}
					respect+=scoreval;
				}else{
					//sad
					printf("Animal %d growls", c);
					if(actor == c){
						printf(" a lot");
					}

					respect-=scoreval;
					//leave room
					leaveRoom(c, roomlist, creaturelist);
				}

			}else if(creaturelist[c][0] == 2){
				//NPC
				int scoreval = 1;
				if(actor == c){	
					scoreval = scoreval * 3;
				}
				
				if(cleandirty == 2){
					//happy
					printf("Human %d smiles", c);

					if(actor == c){
						printf(" a lot");
					}


					respect+=scoreval;
				}else{
					//sad
					printf("Human %d grumbles", c);
					respect-=scoreval;

					if(actor == c){
						printf(" a lot");
					}

					//leave room
					leaveRoom(c, roomlist, creaturelist);
				}

				
			}
			printf("\n");
		}
	}
}

//Clean the room with the selected character.
void clean(int actor, int** roomlist, int** creaturelist){
	int currRoom = creaturelist[actor][1];

	if(roomlist[currRoom][0] == 2){
		roomlist[currRoom][0] = 1;
	}else if(roomlist[currRoom][0] == 1){
		roomlist[currRoom][0] = 0;
		causeReaction(actor, 0, roomlist, creaturelist, currRoom);
		printf("Your respect is now %d\n", respect);
	}else if(roomlist[currRoom][0] == 0){
		//do nothing, already clean.
	}
	
}

//Dirty the room with the selected character.
void dirty(int actor, int** roomlist, int** creaturelist){
	int currRoom = creaturelist[actor][1];
	if(roomlist[currRoom][0] == 0){
		roomlist[currRoom][0] = 1;
	}else if(roomlist[currRoom][0] == 1){
		roomlist[currRoom][0] = 2;
		causeReaction(actor, 2, roomlist, creaturelist, currRoom);
		printf("Your respect is now %d\n", respect);
	}else if(roomlist[currRoom][0] == 2){
		//do nothing, already dirty.
	}

}

//Move the player in a direction.
int move(int actor, int dir, int** roomlist, int** creaturelist){

	int currRoom = creaturelist[actor][1];
	int actorType = creaturelist[actor][0];

	//printf("currRoom: %d",currRoom);

	if(checkMove(dir, roomlist, currRoom) == 0){
		return 0;
	}else{
		int destination = roomlist[currRoom][dir];
		creaturelist[actor][1] = destination;

		roomlist[currRoom][5] = roomlist[currRoom][5] - 1;
		roomlist[destination][5] = roomlist[destination][5] + 1;

		if(actorType == 1){
			//animal
			if(roomlist[destination][0] == 2){
				clean(actor, roomlist, creaturelist);
			}
			//check to see if we need to change state.
		}else if(actorType == 2){
			//NPC
			if(roomlist[destination][0] == 0){
				dirty(actor, roomlist, creaturelist);
			}
		}
		return 1;
	}
}


//Return a random direction to move between [1, 4]
int randomMove(){
	srand(time(NULL));
	int r = (rand() % 4) + 1;
	return r;
}

//Displays the room's information of the user.
void look(int actor, int** roomlist, int** creaturelist){
	//Find the current room using the creature list.
	int currRoom = creaturelist[actor][1];


	//Display the state of the current room.
	printf("\nRoom %d, ", currRoom);
	if(roomlist[currRoom][0] == 0){
		printf("clean, ");
	}else if(roomlist[currRoom][0] == 1){
		printf("half-dirty, ");
	}else {
		printf("dirty, ");
	}

	//Display the neighbors of the current room.
	printf("neighbors:");
	if(roomlist[currRoom][1] != -1){
		printf(" %d to the north,", roomlist[currRoom][1]);
	}
	if(roomlist[currRoom][2] != -1){
		printf(" %d to the south,", roomlist[currRoom][2]);	
	}
	if(roomlist[currRoom][3] != -1){
		printf(" %d to the east,", roomlist[currRoom][3]);	
	}
	if(roomlist[currRoom][4] != -1){
		printf(" %d to the west,", roomlist[currRoom][4]);	
	}

	//Display the creatures in the current room.
	printf(" contains:");
	for(int x = 0; x < creaturect; x++){
		if(creaturelist[x][1] == currRoom){
			if(creaturelist[x][0] == 0){
				//PC
				printf("\nPC");
			}else if(creaturelist[x][0] == 1){
				//Animal
				printf("\nAnimal %d", x);
			}else{
				//NPC
				printf("\nHuman %d", x);
			}
		}
	}
	printf("\n\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void){

	int **roomlist;
	int **creaturelist;

	//Get number of rooms
	printf("# of rooms: "); //n
	scanf("%d", &roomct);

	//Allocate memory for 2D room array
    roomlist = (int **)malloc(roomct * sizeof(int *));
    for (int i = 0; i<roomct; i++){	
    	roomlist[i] = (int *)malloc(6 * sizeof(int));
    }

   	//Collect initialization data using stdin
	for (int rm = 0 ; rm < roomct; rm++) {

		printf("ROOM#: %d\n", rm);
		printf("state: ");
		int state;
		scanf("%d", &state);
		roomlist[rm][0] = state;

		printf("north: ");
		int north;
		scanf("%d", &north);
		roomlist[rm][1] = north;

		printf("south: ");
		int south;
		scanf("%d", &south);
		roomlist[rm][2] = south;

		printf("east: ");
		int east;
		scanf("%d", &east);
		roomlist[rm][3] = east;

		printf("west: ");
		int west;
		scanf("%d", &west);
		roomlist[rm][4] = west;

		//room occupancy
		roomlist[rm][5] = 0;
	}

	//Get number of creatures
	printf("# of creatures: "); //m
	scanf("%d", &creaturect);

	//Allocate memory for 2D creature array
    creaturelist = (int **)malloc(creaturect * sizeof(int *));
    for (int i = 0; i<creaturect; i++){	
    	creaturelist[i] = (int *)malloc(2 * sizeof(int));
    }

   	//Load creature info from stdin
	for (int cc = 0 ; cc < creaturect; cc++) {

		printf("Creature#: %d\n", cc);
		
		printf("creature type: ");
		int type;
		scanf("%d", &type);

		creaturelist[cc][0] = type;

		printf("location: ");
		int loc;
		scanf("%d", &loc);

		creaturelist[cc][1] = loc;

		roomlist[loc][5] = roomlist[loc][5] + 1;
	}

	//Arrays loaded
	//Start the game.


	
    char string[256];
 	while(1==1){
 		printf("~");
 		scanf("%s", string);

 		char command[256] = "";
 		int actorID = -1;
 		int currRoom = creaturelist[getPlayerID(creaturelist)][1];
		
		if(containsSplit(string) == 1){
 			//command for another creature
 			actorID = atoi(splitID(string));
 			char* comm = splitComm(string);
 			strcpy(command, comm);
 		}else{
			strcpy(command, string);
			actorID = getPlayerID(creaturelist);
 		}

 		//printf("actor: %d action: %s\n", actorID, command);
 		

 		if(strcmp(command, "look") == 0){
 			look(actorID, roomlist, creaturelist);
 		}else if(strcmp(command, "quit") == 0){
 			break;
 		}else if(strcmp(command, "north") == 0){
 			if(move(actorID, 1, roomlist, creaturelist) == 0){
 				printf("%d cannot move north!", actorID);
 			}
 		}else if(strcmp(command, "south") == 0){
 			if(move(actorID, 2, roomlist, creaturelist) == 0){
 				printf("%d cannot move south!", actorID);
 			}
 		}else if(strcmp(command, "east") == 0){
 			if(move(actorID, 3, roomlist, creaturelist) == 0){
 				printf("%d cannot move east!", actorID);
 			}
 		}else if(strcmp(command, "west") == 0){
 			if(move(actorID, 4, roomlist, creaturelist) == 0){
 				printf("%d cannot move west!", actorID);
 			}
 		}else if(strcmp(command, "clean") == 0){
 			clean(actorID, roomlist, creaturelist);
 		}else if(strcmp(command, "dirty") == 0){
 			dirty(actorID, roomlist, creaturelist);
 		}else if(strcmp(command, "respect") == 0){
 			printf("Respect is %d\n", respect);
 		}else{
 			printf("\n");
 		}
 		printf("\n");
 		//printArrData(roomlist, creaturelist);

 		if(respect >= 80){
 			printf("Congratulations! You Won!\nRespect Points: %d\n\n", respect);
 			break;
		}
		if(respect <= 0){
			printf("Too Bad! You Lost!\nRespect Points: %d\n\n", respect);
			break;
 		}
	}



 	//Deallocate memory for rooms and creatures.
 	for(int i = 0; i < roomct; i++){
    	free(roomlist[i]);
 	}
	free(roomlist);

	for(int i = 0; i < creaturect; i++){
    	free(creaturelist[i]);
	}
	free(creaturelist);

	return 0;
}


