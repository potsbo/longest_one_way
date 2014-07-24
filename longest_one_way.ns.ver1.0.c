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
char LINE_NAME[N][N], JUNC_NAME[M][N];
int SECT_LENGTH[M], SECT_NUM[M][10], SECT[M][2], LINE[M], LINE_CNT=0, JUNC_CNT=0, SECT_CNT=0;
int sectRecordRoute[M], juncRecordRoute[M], recordLength = 0, recordListCnt = 0;
int TERMINAL_LIST_CNT;
long long int validRouteCnt = 0;//no need to be global
//JUNC[x][] are sections starting from x-th JUNC
//SECT[][0] is one side, SECT[][1] is the other
//BRANCH_CNT[x] is the numbers of SECTs starting x-th JUNC

//functions
char* getRouteData();
char* setDestFile();
void loadJunc(), loadTransfer();
int printJunc(), printSect(), printTerminal();
void setNewJunc(), setNewSect();
int juncSearch();
void countBranch();
void printRecord();
void printRoute();
void printRecord(), renewRecordRoute();
void archiveData();
int recordData();
void updateTempRoute();
void resetBranchStatus();
int getOpposit();
int bruteFroceSearch();

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
	printTerminal( BRANCH_CNT, TERMINAL_LIST );

	//printing JUNCs
	printJunc(BRANCH_CNT);
	/* END analizing and printing process */

	/* printing SECTs */
	printSect();
	//END analizing and printing process

	//calculation prosess
	for ( int presentTerminal = 0; presentTerminal < TERMINAL_LIST_CNT; presentTerminal++ ) {
		int startJunc = TERMINAL_LIST[presentTerminal];
		printf("\n\nNew Terminal:%d\n", presentTerminal);
		bruteFroceSearch( startJunc, BRANCH_CNT);
		//archiving data
		archiveData( presentTerminal, DEST_FILE, recordListCnt);
	}
	//END calculation process

	//print the result of calculation
	printRoute( recordListCnt, juncRecordRoute, sectRecordRoute);
	printRecord("Record Length: ", recordLength);

	return 0;
}



char* getRouteData(int devFlag){
	static char ROUTE_DATA_FILE[N];
	printf("Input route data:");
	scanf("%s", ROUTE_DATA_FILE);
	if(devFlag == 1){
		strcpy(ROUTE_DATA_FILE,"route/JR_kyushu.txt");//for dev
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

void loadJunc(){
	printf("\nStation List");
	char data[N];
	fscanf( fin, "%s", data );//the length (or the line name)

	//each line
	while ( strcmp( data, "乗換") != 0 ){
		strcpy( LINE_NAME[LINE_CNT], data);
		//next line
		printf( "\n%s: ", LINE_NAME[LINE_CNT] );

		//each JUNC
		while(1){ 
			static int determinant = 0; //decision, length or next line , if 0 -> next line, not 0 -> length
			//on one line
			fscanf( fin, "%s", data );
			int juncSub = juncSearch(data);//naming a junction number

			//setting a new JUNC
			//if this junc loaded before, do nothing
			if ( juncSub == -1/*no matching junc*/) {
				strcpy( JUNC_NAME[JUNC_CNT], data);
				juncSub = JUNC_CNT;
				JUNC_CNT++;
			}

			//setting a new SECT
			if ( determinant > 0) SECT[SECT_CNT-1][1] = juncSub;

			printf( "%s ", JUNC_NAME[juncSub] );

			fscanf( fin, "%s", data );//the length (or the line name)

			if ( (determinant = atoi(data)) > 0 ) {
				setNewSect(juncSub,determinant);
				SECT_CNT++;
			}else{
				break;
			}
		} 
		LINE_CNT++;
	} 
}

void loadTransfer(){
	strcpy(LINE_NAME[LINE_CNT],"乗換");
	printf("乗換:");
	char data[N];
	fscanf( fin, "%s", data );

	while ( strcmp( data, "END") != 0 ) {
		int juncSub = juncSearch(data);
		if(juncSub == -1){
			printf("Error: no matching junc");
			exit(1);
		}
		setNewSect(juncSub,0);

		printf( "%s", JUNC_NAME[juncSub] );

		fscanf( fin, "%s", data);
		juncSub = juncSearch(data);
		if(juncSub == -1){
			printf("Error: no matching junc");
			exit(1);
		}
		printf( ":%s ", JUNC_NAME[juncSub] );
		SECT[SECT_CNT][1] = juncSub;
		SECT_CNT++;

		fscanf( fin, "%s", data );
	}
}

int juncSearch(char data[N]){
	for (int i = 0; i < JUNC_CNT; i++ ) {
		if( strcmp( data, JUNC_NAME[i] ) == 0 ) return i;
	}
	return -1;
}

void setNewSect(int juncSub, int length){
	SECT_LENGTH[SECT_CNT] = length;
	SECT[SECT_CNT][0] = juncSub;
	LINE[SECT_CNT] = LINE_CNT;
}

void countBranch(int BRANCH_CNT[M]){
	for (int i = 0; i < SECT_CNT; i++ ) {
		for (int j = 0; j < 2; j++ ) {
			int juncSub = SECT[i][j];
			SECT_NUM[juncSub][ BRANCH_CNT[juncSub] ] = i;
			BRANCH_CNT[juncSub]++;
		}
	}
}

void printRoute(int listCnt, int juncRoute[M], int sectRoute[M]){
	printf("\nroute:" );
	for (int i = 0; i < listCnt; i++ ) {
		printf("%s", JUNC_NAME[ juncRoute[i] ] );
		printf("-[%s]-", LINE_NAME[ LINE[ sectRoute[i] ] ] );
	}
	printf("%s\n", JUNC_NAME[juncRoute[listCnt] ] );
}


void archiveData(int presentTerminal, char DEST_FILE[N], int recordListCnt){
	if ( (fout = fopen( DEST_FILE, "w")) == NULL ) {
		printf("\nOutput file not created\n");
		exit(1);
	}

	fprintf( fout, "Terminal count:%d\n", presentTerminal);
	fprintf( fout, "route:" );

	time_t now = time(NULL);
	struct tm *pnow = localtime(&now);

	for (int i = 0; i < recordListCnt; i++ ) {
		fprintf( fout, "%s", JUNC_NAME[ juncRecordRoute[i] ] );
		fprintf( fout, "-[%s]-", LINE_NAME[ LINE[ sectRecordRoute[i] ] ] );
	}
	fprintf( fout, "%s\n", JUNC_NAME[ juncRecordRoute[recordListCnt] ] );			
	fprintf( fout,"Record Length: %5.1fkm\n", recordLength * 0.1);
	fprintf( fout,"validRouteCnt = %lld\n", validRouteCnt);
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

int printTerminal( int BRANCH_CNT[M], int TERMINAL_LIST[M] ){
	printf("\n\nTERMINAL LIST:");
	for (int i = 0; i < JUNC_CNT; i++) {
		if (BRANCH_CNT[i] == 1) {
			TERMINAL_LIST[TERMINAL_LIST_CNT] = i;
			printf("\n%2d %s %d", TERMINAL_LIST_CNT, JUNC_NAME[i], i);
			TERMINAL_LIST_CNT++;
		}
	}
	printf("\nTERMINAL_LIST_CNT = %d",TERMINAL_LIST_CNT);
	return 0;
}

int printJunc(int BRANCH_CNT[M]){
	printf("\n\nJUNC LIST:\n");
	for (int i = 0; i < JUNC_CNT; i++ ) {

		//printing one JUNC
		printf( "%d %s (BRANCH COUNT:%d BRANCHES:", i, JUNC_NAME[i], BRANCH_CNT[i] );

		//printing connected JUNCs
		for (int j = 0; j < BRANCH_CNT[i]; j++ ) {
			printf( " %s", JUNC_NAME[ getOpposit(i,j) ]);
		}
		printf( ")\n");
	}
	printf("JUNC_COUNT = %d", JUNC_CNT);
	return 0;
}

int printSect(){
	int SUM_SECT_LENGTH = 0;
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
	return 0;
}

void printRecord(char lengthMessage[M], int recordLength){
	printf("%s%5.1fkm - ", lengthMessage, recordLength*0.1);
	printf("validRouteCnt = %lld\n", validRouteCnt );
}

void printRecordRoute(int sectTempRoute[M], int juncTempRoute[M], int tempListCnt){
	for (int i = 0; i < tempListCnt+1; i++ ) {
		sectRecordRoute[i] = sectTempRoute[i];
		juncRecordRoute[i] = juncTempRoute[i];
	}
}

int recordData( int tempListCnt, int tempRouteLength, int sectTempRoute[M], int juncTempRoute[M]){
	//saving the length, sections, junctions of the temp route
	recordLength = tempRouteLength;
	recordListCnt = tempListCnt;

	//renewing record SECTs and JUNCs
	printRecordRoute( sectTempRoute, juncTempRoute, tempListCnt);

	//renewing the record
	printRecord("Record updated: ", recordLength);

	return recordListCnt;
}

void updateTempRoute( int tempListCnt, int branchNum, int sectTempRoute[M], int juncTempRoute[M], int nextJunc, int presentJunc){
	sectTempRoute[tempListCnt] = SECT_NUM[presentJunc][branchNum];
	juncTempRoute[tempListCnt + 1] = nextJunc;
}

void resetBranchStatus( int presentJunc, int BRANCH_CNT[M], int branchStatus[M][M]){
	for (int i = 0; i < BRANCH_CNT[presentJunc]; i++ ) {
		branchStatus[presentJunc][i] = 0;
	}
}

int getOpposit( int juncNum, int branchNum){
	int selectedSect = SECT_NUM[juncNum][branchNum];
	if(juncNum == SECT[selectedSect][0]){
		return SECT[selectedSect][1];
	}else{
		return SECT[selectedSect][0];
	}
}

int bruteFroceSearch( int startJunc, int BRANCH_CNT[M]){
	int sectTempRoute[M], juncTempRoute[M], tempListCnt = 0, tempRouteLength = 0, juncStatus[M], branchStatus[M][M] = {};
	int previousJunc, presentJunc, nextJunc;

	//setting temp data
	presentJunc = startJunc;//setting the first junction
	juncStatus[presentJunc] = 1;//this means i-th junction is traveled
	juncTempRoute[tempListCnt] = presentJunc;

	previousJunc = presentJunc;

	while( previousJunc == presentJunc || tempListCnt > 0){
		//setting branchNum (indicator) and searching next junc
		int presentBranchCNT = BRANCH_CNT[presentJunc], branchNum;

		for ( branchNum = 0; branchNum < presentBranchCNT; branchNum++ ) {
			//searching a free branch
			if ( branchStatus[presentJunc][branchNum] == 0 ) {
				//setting nextJunc
				nextJunc = getOpposit(presentJunc, branchNum);
				if ( nextJunc != previousJunc ) break;
			}
		}

		if ( branchNum < presentBranchCNT && juncStatus[presentJunc] < 2 ) {
			//able to make this route longer
			int previousLength = 0;
			//updating status
			juncStatus[nextJunc]++;
			branchStatus[presentJunc][branchNum] = 1;

			//updating temp data
			updateTempRoute( tempListCnt, branchNum, sectTempRoute, juncTempRoute, nextJunc, presentJunc);

			previousLength = SECT_LENGTH[SECT_NUM[presentJunc][branchNum]];
			tempRouteLength += previousLength;

			tempListCnt++;
			previousJunc = presentJunc;
			presentJunc = nextJunc;
			/* printRoute(tempListCnt,juncTempRoute,sectTempRoute); */

		} else {
			//end of a route
			validRouteCnt++;

			//checking if recordable
			if ( tempRouteLength > recordLength ){
				recordListCnt = recordData( tempListCnt, tempRouteLength, sectTempRoute, juncTempRoute);
			}

			if ( juncStatus[presentJunc] == 1 ) { //checking if coming presentJunc for the first time
				resetBranchStatus(presentJunc, BRANCH_CNT, branchStatus); //resetting branchStatus starting from presentJunc
			}

			//backing to the previous JUNC
			juncStatus[presentJunc]--;
			tempListCnt--;
			presentJunc = juncTempRoute[tempListCnt];
			previousJunc = juncTempRoute[tempListCnt - 1];
			tempRouteLength -= SECT_LENGTH[ sectTempRoute[tempListCnt] ];
		}

	}

	return recordListCnt;
}
