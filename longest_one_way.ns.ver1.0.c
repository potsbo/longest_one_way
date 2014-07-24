//
//  longest_one_way.otbns.ver1.0.c
//
//
//  Created by Naoki OTSUBO on 13/09/22.
//  Edited by Shimpei OTSUBO on 13/09/28
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define M 1000  //array size
#define N 100   //string size

FILE *fin, *fout;
char LINE_NAME[N][N], JUNC_NAME[M][N], DATA_FILE[N];
int SECT_NUM[M][10], SECT[M][2], LINE[M], LINE_CNT=0, JUNC_CNT=0, SECT_CNT=0;
int SUM_SECT_LENGTH = 0;
int SECT_LENGTH[M];
int sect_record_route[M], junc_record_route[M], record_length = 0; 
int temp_route_length = 0, previous_junc, present_junc, next_junc;
int TERMINAL_LIST_CNT;
long long int valid_route_cnt = 0;//no need to be global
int record_list_cnt = 0;
//JUNC[x][] are sections starting from x-th JUNC
//SECT[][0] is one side, SECT[][1] is the other
//BRANCH_CNT[x] is the numbers of SECTs starting x-th JUNC

//functions
char* getRouteData();
char* setDestFile();
int junc_search();
void setNewJunc();
void printRecord();
void printRecordRoute();
void archiveData();
int backToPrevious();
void countBranch();
int printTerminals();
void loadJunc();
void loadTransfer();
void setNewSect();
void printJunc();
void printSect();
void renewRecord();
void renewRoute();
int recordData();
void updateTempRoute();
void resetBranchStatus();
int getOpposit();
int bruteForceTerminal();

int main(void){
	int devFlag = 1;

	/* data input process */
	char* ROUTE_DATA_FILE = getRouteData(devFlag);//opening fin
	char* DEST_FILE = setDestFile(devFlag);
	/* END data input process */

	/* loading process */
	fin = fopen( ROUTE_DATA_FILE, "r");
	loadJunc(); 	//loading and printing JUNCs data except transfers
	loadTransfer(); //loading and printing transfers data
	fclose(fin);
	/* END loading process */

	/* analizing and printing process */
	int BRANCH_CNT[M] = {};
	countBranch( BRANCH_CNT );

	//making and printing TERMINAL_LIST
	int TERMINAL_LIST[M];
	printTerminals( BRANCH_CNT, TERMINAL_LIST );

	//printing JUNCs
	printJunc(BRANCH_CNT);
	/* END analizing and printing process */

	/* printing SECTs */
	printSect();
	//END analizing and printing process

	//calculation prosess
	for ( int present_terminal = 0; present_terminal < TERMINAL_LIST_CNT; present_terminal++ ) {
		bruteForceTerminal(present_terminal, TERMINAL_LIST, BRANCH_CNT);
		//archiving data
		archiveData( present_terminal, DEST_FILE, record_list_cnt );
	}
	//END calculation process

	//print the result of calculation
	printRecordRoute(record_list_cnt);
	printRecord( record_length, valid_route_cnt);

	return 0;
}



char* getRouteData(int devFlag){
	static char ROUTE_DATA_FILE[N];
	printf("Input source data:");
	scanf("%s", ROUTE_DATA_FILE);
	if(devFlag == 1){
		strcpy(ROUTE_DATA_FILE,"route/fukuoka_city.txt");//for dev
	}

	//checking the file
	if ( (fin = fopen( ROUTE_DATA_FILE, "r")) == NULL ) {
		printf("\nFile not found\n");
		exit(1);
	}
	fclose(fin);
	return ROUTE_DATA_FILE;
}

char* setDestFile(int devFlag){
	static char DEST_FILE[N];
	printf("Input destination file:");
	scanf("%s", DEST_FILE);
	if(devFlag == 1){
		strcpy(DEST_FILE,"test");//for dev
	}
	return DEST_FILE;
}

//finding same junction, avoiding duplicate
int junc_search(char data[N]){
	for (int i = 0; i < JUNC_CNT; i++ ) {
		if( strcmp( data, JUNC_NAME[i] ) == 0 ) return i;
	}
	return -1;
}

void setNewJunc(int junc_sub, char data[N]){
	if ( junc_sub == JUNC_CNT /*no matching junc*/) {
		strcpy( JUNC_NAME[JUNC_CNT], data);
		JUNC_CNT++;
	}
}

void printRecordRoute(int record_list_cnt){
	printf("\nroute:" );
	for (int i = 0; i < record_list_cnt; i++ ) {
		printf("%s", JUNC_NAME[ junc_record_route[i] ] );
		printf("-[%s]-", LINE_NAME[ LINE[ sect_record_route[i] ] ] );
	}
	printf("%s\n", JUNC_NAME[ junc_record_route[record_list_cnt] ] );

}

void printRecord( int record_length, long long int valid_route_cnt){
	printf("Record Length: %5.1fkm\n", record_length * 0.1);
	printf("valid_route_cnt = %lld\n", valid_route_cnt);
}

void archiveData(int present_terminal, char DEST_FILE[N], int record_list_cnt){
	if ( (fout = fopen( DEST_FILE, "w")) == NULL ) {
		printf("\nOutput file not created\n");
		exit(1);
	}

	fprintf( fout, "Terminal count:%d\n",present_terminal);
	fprintf( fout, "route:" );

	time_t now = time(NULL);
	struct tm *pnow = localtime(&now);

	for (int i = 0; i < record_list_cnt; i++ ) {
		fprintf( fout, "%s", JUNC_NAME[ junc_record_route[i] ] );
		fprintf( fout, "-[%s]-", LINE_NAME[ LINE[ sect_record_route[i] ] ] );
	}
	fprintf( fout, "%s\n", JUNC_NAME[ junc_record_route[record_list_cnt] ] );			
	fprintf( fout,"Record Length: %5.1fkm\n", record_length * 0.1);
	fprintf( fout,"valid_route_cnt = %lld\n", valid_route_cnt);
	fprintf( fout, "time:%02d/%02d/%02d %02d:%02d:%02d\n",
			pnow->tm_year+1900,
			pnow->tm_mon + 1,
			pnow->tm_mday,
			pnow->tm_hour,
			pnow->tm_min,
			pnow->tm_sec
		   );

	fclose(fout);
}

int backToPrevious(int junc_status[M], int temp_list_cnt, int junc_temp_route[M], int sect_temp_route[M]){
	junc_status[present_junc]--;
	temp_list_cnt--;
	present_junc = junc_temp_route[temp_list_cnt];
	previous_junc = junc_temp_route[temp_list_cnt - 1];
	temp_route_length -= SECT_LENGTH[ sect_temp_route[temp_list_cnt] ];
	return temp_list_cnt;
}


void countBranch(int BRANCH_CNT[M]){
	for (int i = 0; i < SECT_CNT; i++ ) {
		for (int j = 0; j < 2; j++ ) {
			int junc_sub = SECT[i][j];
			SECT_NUM[junc_sub][ BRANCH_CNT[junc_sub] ] = i;
			BRANCH_CNT[junc_sub]++;
		}
	}
}

int printTerminals( int BRANCH_CNT[M], int TERMINAL_LIST[M] ){
	printf("\n\nTERMINAL LIST:");
	for (int i = 0; i < JUNC_CNT; i++) {
		if (BRANCH_CNT[i] == 1) {
			TERMINAL_LIST[TERMINAL_LIST_CNT] = i;
			printf("\n%d %s %d", TERMINAL_LIST_CNT, JUNC_NAME[ TERMINAL_LIST[TERMINAL_LIST_CNT] ], i);
			TERMINAL_LIST_CNT++;
		}
	}
	printf("\nTERMINAL_LIST_CNT = %d",TERMINAL_LIST_CNT);
	return 0;
}

void loadJunc(){
	printf("\nStation List\n");
	char data[N];
	fscanf( fin, "%s", data );//the length (or the line name)

	while ( strcmp( data, "乗換") != 0 ){
		int determinant = 0; //decision, length or next line , if 0 -> next line, not 0 -> length
		strcpy( LINE_NAME[LINE_CNT], data);
		//next line
		printf( "%s: ", LINE_NAME[LINE_CNT] );

		//each JUNC
		while(1){ 
			//on one line
			fscanf( fin, "%s", data );
			int junc_sub = junc_search(data);//naming a junction number

			//setting a new JUNC
			//if this junc loaded before, do nothing
			if ( junc_sub == -1/*no matching junc*/) {
				strcpy( JUNC_NAME[JUNC_CNT], data);
				junc_sub = JUNC_CNT;
				JUNC_CNT++;
			}

			//setting a new SECT
			if ( determinant > 0) SECT[SECT_CNT-1][1] = junc_sub;

			printf( "%s ", JUNC_NAME[junc_sub] );

			fscanf( fin, "%s", data );//the length (or the line name)

			if ( (determinant = atoi(data)) > 0 ) {
				setNewSect(determinant,junc_sub);
				SECT_CNT++;
			}else{
				break;
			}
		} 

		LINE_CNT++;
		printf("\n");
	} 
	strcpy(LINE_NAME[LINE_CNT],data);
}

void loadTransfer(){
	printf( "%s: ", LINE_NAME[LINE_CNT] );
	char data[N];
	fscanf( fin, "%s", data );

	while ( strcmp( data, "END") != 0 ) {
		int junc_sub = junc_search(data);
		if(junc_sub == -1){
			printf("Error: no matching junc");
			exit(1);
		}
		setNewSect(0,junc_sub);

		printf( "%s", JUNC_NAME[junc_sub] );

		fscanf( fin, "%s", data);
		junc_sub = junc_search(data);
		if(junc_sub == -1){
			printf("Error: no matching junc");
			exit(1);
		}
		printf( ":%s ", JUNC_NAME[junc_sub] );
		SECT[SECT_CNT][1] = junc_sub;

		SECT_CNT++;

		fscanf( fin, "%s", data );
	}
}

void setNewSect(int length, int junc_sub){
	SECT_LENGTH[SECT_CNT] = length;
	SECT[SECT_CNT][0] = junc_sub;
	LINE[SECT_CNT] = LINE_CNT;
}

void printJunc(int BRANCH_CNT[M]){
	printf("\n\nJUNC LIST:\n");
	for (int i = 0; i < JUNC_CNT; i++ ) {

		//printing one JUNC
		printf( "%d %s (BRANCH COUNT:%d BRANCHES:", i, JUNC_NAME[i], BRANCH_CNT[i] );

		//printing connected JUNCs
		for (int j = 0; j < BRANCH_CNT[i]; j++ ) {
			printf( " %s", JUNC_NAME[ getOpposit(i,j)]);
		}

		printf( ")\n");
	}
	printf("JUNC_COUNT = %d", JUNC_CNT);
}

void printSect(){
	printf("\n\nSECT LIST:\n");
	for (int i = 0; i < SECT_CNT; i++ ) {
		printf( "%d ", i );
		printf( "%s-%s ", JUNC_NAME[SECT[i][0]], JUNC_NAME[SECT[i][1]] );
		printf( "[%s] ", LINE_NAME[LINE[i]]);
		printf( "%5.1fkm\n", SECT_LENGTH[i]*0.1 );
		SUM_SECT_LENGTH += SECT_LENGTH[i];
	}
	printf( "SECT_COUNT = %d\n", SECT_CNT );
	printf( "SUM_SECT_LENTGH = %5.1fkm\n", SUM_SECT_LENGTH * 0.1 );
}

void renewRecord( int junc_temp_route[M], int record_length, int present_terminal, int record_list_cnt, long long int valid_route_cnt){
	junc_record_route[record_list_cnt] = junc_temp_route[record_list_cnt];
	printf("Record updated: %5.1fkm - checking %d of %d TERMINAL(s)"
			,record_length*0.1 ,present_terminal,TERMINAL_LIST_CNT);
	printf("\nvalid_route_cnt = %lld\n", valid_route_cnt );
}

void renewRoute(int sect_temp_route[M], int junc_temp_route[M], int record_list_cnt){
	for (int i = 0; i < record_list_cnt; i++ ) {
		sect_record_route[i] = sect_temp_route[i];
		junc_record_route[i] = junc_temp_route[i];
	}
}

int recordData( int temp_list_cnt, int sect_temp_route[M], int junc_temp_route[M], int present_terminal, int record_list_cnt, long long int valid_route_cnt){
	//saving the length, sections, junctions of the temp route
	record_length = temp_route_length;
	record_list_cnt = temp_list_cnt;

	//renewing record SECTs and JUNCs
	renewRoute( sect_temp_route, junc_temp_route, record_list_cnt );

	//renewing the record
	renewRecord( junc_temp_route, record_length, present_terminal, record_list_cnt, valid_route_cnt);

	return record_list_cnt;
}

void updateTempRoute(int temp_list_cnt, int branchNum, int sect_temp_route[M], int junc_temp_route[M]){
	sect_temp_route[temp_list_cnt] = SECT_NUM[present_junc][branchNum];
	junc_temp_route[temp_list_cnt + 1] = next_junc;
}

void resetBranchStatus(int BRANCH_CNT[M], int branch_status[M][M]){
	for (int i = 0; i < BRANCH_CNT[present_junc]; i++ ) {
		branch_status[present_junc][i] = 0;
	}
}

int getOpposit(int juncNum, int branchNum){
	int selected_sect = SECT_NUM[juncNum][branchNum];
	if(juncNum == SECT[selected_sect][0]){
		return SECT[selected_sect][1];
	}else{
		return SECT[selected_sect][0];
	}
}

int bruteForceTerminal(int present_terminal, int TERMINAL_LIST[M],int BRANCH_CNT[M]){

		int sect_temp_route[M], junc_temp_route[M], temp_list_cnt = 0, junc_status[M], branch_status[M][M];

		//calculate routes starting from a terminal
		printf("\n\nNew Terminal:%d\n", present_terminal);
		//setting temp data
		present_junc = TERMINAL_LIST[present_terminal];//setting the first junction
		junc_status[present_junc] = 1;//this means i-th junction is traveled
		junc_temp_route[temp_list_cnt] = present_junc;

		previous_junc = present_junc;

		while( present_junc != junc_temp_route[0] || previous_junc == present_junc){
			//setting branchNum (indicator) and searching next junc
			int presentBranchCNT = BRANCH_CNT[present_junc], branchNum;

			for ( branchNum = 0; branchNum < presentBranchCNT; branchNum++ ) {
				//searching a free branch
				if ( branch_status[present_junc][branchNum] == 0 ) {
					//setting next_junc
					next_junc = getOpposit(present_junc, branchNum);
					if ( next_junc != previous_junc ) break;
				}
			}

			if ( branchNum < presentBranchCNT && junc_status[present_junc] < 2 ) {
				//able to make this route longer
				int previous_length = 0;
				//updating status
				junc_status[next_junc]++;
				branch_status[present_junc][branchNum] = 1;

				//updating temp data
				updateTempRoute(temp_list_cnt, branchNum, sect_temp_route, junc_temp_route);

				previous_length = SECT_LENGTH[SECT_NUM[present_junc][branchNum]];
				temp_route_length += previous_length;

				temp_list_cnt++;
				previous_junc = present_junc;
				present_junc = next_junc;

			} else {
				//end of a route
				valid_route_cnt++;

				//checking if recordable
				if ( temp_route_length > record_length ){
					record_list_cnt = recordData( temp_list_cnt, sect_temp_route, junc_temp_route, present_terminal, record_list_cnt, valid_route_cnt);
				}

				if (junc_status[present_junc] == 1 ) { //checking if coming present_junc for the first time
					resetBranchStatus(BRANCH_CNT, branch_status); //resetting branch_status starting from present_junc
				}

				//backing to the previous JUNC
				temp_list_cnt = backToPrevious(junc_status, temp_list_cnt, junc_temp_route, sect_temp_route);
			}

		}


	return record_list_cnt;
}
