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
char LINE_NAME[N][N], JUNC_NAME[M][N], data[N]={"0"}, DATA_FILE[N];
/* char DEST_FILE[N];  */
int time_start, time_now, time_mark[M];
int SECT_NUM[M][10], SECT[M][2], LINE[M], LINE_CNT=0, JUNC_CNT=0, SECT_CNT=0;
int list_cnt; 
int selected_sect, SUM_SECT_LENGTH=0;
long long int valid_route_cnt=0;
int SECT_LENGTH[M];

//functions
int junc_search();
int setNewJunc();
void printRecord();
void printRecordRoute();
void getRouteData();
char* setDestFile();
void archiveData();
void backToPrevious();
void analyzeBranch();
int printTerminals();

int main(void){

	time_start = clock();

	//JUNC[x][] are sections starting from x-th JUNC
	//SECT[][0] is one side, SECT[][1] is the other
	//BRANCH_CNT[x] is the numbers of SECTs starting x-th JUNC

	//data input process
	getRouteData();
	char* DEST_FILE = setDestFile();

	printf("\nStation List\n");
	fscanf( fin, "%s", LINE_NAME[LINE_CNT]);

	int BRANCH_CNT[M] = {};
	//loading and printing JUNCs data
	while ( strcmp( data, "乗換") != 0 ){
		int determinant = 0; //decision, length or next line , if 0 -> next line, not 0 -> length
		//next line
		printf( "%s: ", LINE_NAME[LINE_CNT] );

		//each JUNC
		while(1){ 
			//on one line
			fscanf( fin, "%s", data );
			int junc_sub = junc_search( JUNC_NAME, JUNC_CNT );//naming a junction number

			//setting a new JUNC
			//if this junc loaded before, do nothing
			JUNC_CNT = setNewJunc(junc_sub, JUNC_CNT, JUNC_NAME, data, BRANCH_CNT);

			//setting a new SECT
			if ( determinant > 0) SECT[SECT_CNT-1][1] = junc_sub;

			printf( "%s ", JUNC_NAME[junc_sub] );

			fscanf( fin, "%s", data );//the length (or the line name)

			if ( (determinant = atoi(data)) > 0 ) {
				SECT_LENGTH[SECT_CNT] = determinant;
				SECT[SECT_CNT][0] = junc_sub;
				LINE[SECT_CNT] = LINE_CNT;
				SECT_CNT++;
			}else{
				break;
			}
		} 

		LINE_CNT++;
		strcpy( LINE_NAME[LINE_CNT], data);

		printf("\n");
	} 

	printf( "%s: ", LINE_NAME[LINE_CNT] );
	fscanf( fin, "%s", data );

	//printing station list
	while ( strcmp( data, "END") != 0 ) {
		int junc_sub = junc_search( JUNC_NAME, JUNC_CNT );

		printf( "%s-", JUNC_NAME[junc_sub] );
		SECT[SECT_CNT][0] = junc_sub;
		SECT_LENGTH[SECT_CNT] = 0;
		LINE[SECT_CNT] = LINE_CNT;

		fscanf( fin, "%s", data);
		junc_sub = junc_search( JUNC_NAME, JUNC_CNT );
		printf( "%d:%s ",JUNC_CNT, JUNC_NAME[junc_sub] );
		SECT[SECT_CNT][1] = junc_sub;

		SECT_CNT++;

		fscanf( fin, "%s", data );
	}

	fclose(fin);
	//loading process END
	//analitics process
	//counting branch, asigning junc_sub <- ??
	analyzeBranch( BRANCH_CNT );

	//making and printing TERMINAL_LIST
	int TERMINAL_LIST[M];
	int TERMINAL_LIST_CNT = printTerminals( BRANCH_CNT, TERMINAL_LIST );

	//printing JUNCs
	printf("\n\nJUNC LIST:\n");
	for (int i = 0; i < JUNC_CNT; i++ ) {

		//printing one JUNC
		printf( "%d %s (BRANCH COUNT:%d BRANCHES:", i, JUNC_NAME[i], BRANCH_CNT[i] );

		//printing opposite JUNCs
		for (int j = 0; j < BRANCH_CNT[i]; j++ ) {
			selected_sect = SECT_NUM[i][j];
			if ( i == SECT[selected_sect][0] ) {
				printf( " %s", JUNC_NAME[ SECT[selected_sect][1] ]);
			} else {
				printf( " %s", JUNC_NAME[ SECT[selected_sect][0] ]);
			}
		}

		printf( ")\n");
	}
	printf("JUNC_COUNT = %d", JUNC_CNT);

	//printing SECTs
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

	time_mark[0] = clock();
	printf("time:%d",time_mark[0] - time_start);


	//calculation prosess
	int sect_temp_route[M], sect_record_route[M], junc_temp_route[M], junc_record_route[M];
	int record_length=0, temp_route_length=0, temp_list_cnt=0;

	for ( int present_terminal = 0; present_terminal < TERMINAL_LIST_CNT; present_terminal++ ) {
		int previous_junc, present_junc, next_junc;
		int junc_status[M], branch_status[M][M];

		//calculate routes starting from a terminal
		printf("\n\nA New Terminal:%d\n",present_terminal);
		//setting temp data
		present_junc = TERMINAL_LIST[present_terminal];//setting the first junction
		junc_status[present_junc] = 1;//this means i-th junction is traveled
		junc_temp_route[temp_list_cnt] = present_junc;

		previous_junc = present_junc;

		while( present_junc != junc_temp_route[0] || previous_junc == present_junc){
			//setting branchNum (indicator) and searching next junc
			int presentBranchCNT = BRANCH_CNT[present_junc];
			int branchNum;
			for ( branchNum= 0; branchNum < presentBranchCNT; branchNum++ ) {
				//searching a free branch
				if ( branch_status[present_junc][branchNum] == 0 ) {

					//setting next_junc
					selected_sect = SECT_NUM[present_junc][branchNum];
					if ( present_junc == SECT[selected_sect][0] ) {
						next_junc = SECT[selected_sect][1];
					} else {
						next_junc = SECT[selected_sect][0];
					}

					if ( next_junc != previous_junc ) break;
				}

			}

			if ( branchNum < presentBranchCNT && junc_status[present_junc] < 2 ) {
				int previous_length=0;
				//updating status
				junc_status[next_junc]++;
				branch_status[present_junc][branchNum] = 1;

				//updating temp data
				sect_temp_route[temp_list_cnt] = SECT_NUM[present_junc][branchNum];
				junc_temp_route[temp_list_cnt + 1] = next_junc;

				previous_length = SECT_LENGTH[SECT_NUM[present_junc][branchNum]];
				temp_route_length += previous_length;

				temp_list_cnt++;
				previous_junc = present_junc;
				present_junc = next_junc;

			} else {
				valid_route_cnt++;

				//checking if recordable
				if ( temp_route_length > record_length ) {
					//saving the length, sections, junctions of the temp route
					record_length = temp_route_length;
					list_cnt = temp_list_cnt;

					//renewing record SECTs and JUNCs
					for (int i = 0; i < list_cnt; i++ ) {
						sect_record_route[i] = sect_temp_route[i];
						junc_record_route[i] = junc_temp_route[i];
					}

					//renewing the record
					junc_record_route[list_cnt] = junc_temp_route[list_cnt];
					time_now = clock();
					printf("\nlongest route updated: %5.1fkm - checking %d of %d TERMINAL(s) - %dms passed"
							,record_length*0.1 ,present_terminal,TERMINAL_LIST_CNT, time_now-time_start);
					printf("\nvalid_route_cnt = %lld", valid_route_cnt );

					//archiving data
					archiveData(present_terminal, junc_record_route, sect_record_route, record_length, DEST_FILE);
				}

				//checking if coming present_junc for the first time
				if (junc_status[present_junc] == 1 ) {
					//resetting branch_status starting from present_junc
					for (int i = 0; i < BRANCH_CNT[present_junc]; i++ ) {
						branch_status[present_junc][i] = 0;
					}
				}

				//backing to the previous JUNC
				/* backToPrevious(); */
				junc_status[present_junc]--;
				temp_list_cnt--;
				present_junc = junc_temp_route[temp_list_cnt];
				previous_junc = junc_temp_route[temp_list_cnt - 1];
				temp_route_length -= SECT_LENGTH[ sect_temp_route[temp_list_cnt] ];

			}

		} 

	}

	//print the result of calculation
	printRecordRoute(JUNC_NAME, junc_record_route, LINE_NAME, LINE, sect_record_route);
	printRecord( time_mark, time_start, record_length, valid_route_cnt);

	return 0;

}




//finding same junction, avoiding duplicate
int junc_search( char JUNC_NAME[M][N], int JUNC_CNT ){
	int i;

	for ( i = 0; i < JUNC_CNT; i++ ) {
		if( strcmp( data, JUNC_NAME[i] ) == 0 ) break;
	}

	return i;
}

int setNewJunc(int junc_sub, int JUNC_CNT, char JUNC_NAME[M][N],char data[N], int BRANCH_CNT[M]){
	if ( junc_sub == JUNC_CNT /*no matching junc*/) {
		strcpy( JUNC_NAME[JUNC_CNT], data);
		/* BRANCH_CNT[JUNC_CNT] = 0; */
		JUNC_CNT++;
	}
	return JUNC_CNT;
}

void printRecordRoute( char JUNC_NAME[M][N], int junc_record_route[M], char LINE_NAME[N][N], int LINE[M],int sect_record_route[M]){
	int i;

	printf("\nroute:" );
	for ( i = 0; i < list_cnt; i++ ) {
		printf("%s", JUNC_NAME[ junc_record_route[i] ] );
		printf("-[%s]-", LINE_NAME[ LINE[ sect_record_route[i] ] ] );
	}
	printf("%s\n", JUNC_NAME[ junc_record_route[list_cnt] ] );

}

void printRecord(  int time_mark[M], int time_start, int record_length, long long int valid_route_cnt){
	int time_end;

	time_end = clock();
	printf("\n cal proccess %dms\n",time_end - time_mark[0]);
	double time_passed;
	time_passed = 1000*(time_end - time_start)/CLOCKS_PER_SEC;
	printf("Record Length: %5.1fkm\n %fs passed", record_length * 0.1, time_passed);
	printf("valid_route_cnt = %lld\n", valid_route_cnt);

}

void getRouteData(){
	char ROUTE_DATA_FILE[N];
	printf("Input source data:");
	scanf("%s", ROUTE_DATA_FILE);
	strcpy(ROUTE_DATA_FILE,"route/fukuoka_city.txt");

	//checking the file
	if ( (fin = fopen( ROUTE_DATA_FILE, "r")) == NULL ) {
		printf("\nFile not found\n");
		exit(1);
	}
}


char* setDestFile(){
	static char DEST_FILE[N];
	printf("Input destination file:");
	scanf("%s", DEST_FILE);
	strcpy(DEST_FILE,"test");
	return DEST_FILE;
}

void archiveData(int present_terminal, int junc_record_route[M], int sect_record_route[M], int record_length, char DEST_FILE[N]){
	if ( (fout = fopen( DEST_FILE, "w")) == NULL ) {
		printf("\nOutput file not created\n");
		exit(1);
	}

	fprintf( fout, "Terminal count:%d\n",present_terminal);
	fprintf( fout, "route:" );

	time_t now = time(NULL);
	struct tm *pnow = localtime(&now);

	int k;
	for ( k = 0; k < list_cnt; k++ ) {
		fprintf( fout, "%s", JUNC_NAME[ junc_record_route[k] ] );
		fprintf( fout, "-[%s]-", LINE_NAME[ LINE[ sect_record_route[k] ] ] );
	}
	fprintf( fout, "%s\n", JUNC_NAME[ junc_record_route[list_cnt] ] );			
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

void backToPrevious(int junc_status[M],int previous_junc, int  present_junc, int temp_list_cnt, int junc_temp_route[M], int temp_route_length, int SECT_LENGTH[M], int sect_temp_route[M]){
				junc_status[present_junc]--;
				temp_list_cnt--;
				present_junc = junc_temp_route[temp_list_cnt];
				previous_junc = junc_temp_route[temp_list_cnt - 1];
				temp_route_length -= SECT_LENGTH[ sect_temp_route[temp_list_cnt] ];
}


void analyzeBranch(int BRANCH_CNT[M]){
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
	int TERMINAL_LIST_CNT = 0;
	for (int i = 0; i < JUNC_CNT; i++) {
		if (BRANCH_CNT[i] == 1) {
			TERMINAL_LIST[TERMINAL_LIST_CNT] = i;
			printf("\n%d %s %d", TERMINAL_LIST_CNT, JUNC_NAME[ TERMINAL_LIST[TERMINAL_LIST_CNT] ], i);
			TERMINAL_LIST_CNT++;
		}
	}
	printf("\nTERMINAL_LIST_CNT = %d",TERMINAL_LIST_CNT);
	return TERMINAL_LIST_CNT;
}
