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

//finding same junction, avoiding duplicate
int junc_search( char data[N], char JUNC_NAME[M][N], int JUNC_CNT ){
	int i;

	for ( i = 0; i < JUNC_CNT; i++ ) {
		if( strcmp( data, JUNC_NAME[i] ) == 0 ) break;
	}

	return i;
}

int main(void){

	//junction:junc, sect:section
	int i, j, k, jmax, kmax;
	int SECT_NUM[M][10], SECT[M][2], LINE[M], LINE_CNT=0, JUNC_CNT=0, SECT_CNT=0, SECT_LENGTH[M], BRANCH_CNT[M], determinant, junc_sub, selected_sect, SUM_SECT_LENGTH=0;
	char LINE_NAME[N][N], JUNC_NAME[M][N], data[N], DATA_FILE[N],DEST_FILE[N];
	int time_start, time_now, time_end, time_mark[M];
    FILE *fin;//loafing a file, fin is the file
    
	time_start = clock();

	//JUNC[x][] are sections starting from x-th JUNC
	//SECT[][0] is one side, SECT[][1] is the other
	//BRANCH_CNT[x] is the numbers of SECTs starting x-th JUNC

	//data input process

	printf("Input source data:");
	scanf("%s", DATA_FILE);
	
	//checking the file
	if ( (fin = fopen( DATA_FILE, "r")) == NULL ) {
		printf("\nFile not found\n");
		exit(1);
	}

	printf("Input destination file:");
	scanf("%s", DEST_FILE);



	fscanf( fin, "%s", LINE_NAME[LINE_CNT]);

	printf("\nStation List\n");

	//loading and printing JUNCs data
	do {
		//next line
		printf( "%s: ", LINE_NAME[LINE_CNT] );

		determinant = 0; //decision, length or next line

		//each JUNC
		do {
			//on one line
			fscanf( fin, "%s", data );
			junc_sub = junc_search( data, JUNC_NAME, JUNC_CNT );//naming a junction number

			//setting a new JUNC
			if ( junc_sub == JUNC_CNT ) {
				strcpy( JUNC_NAME[JUNC_CNT], data);
				BRANCH_CNT[JUNC_CNT] = 0;
				JUNC_CNT++;
			}

			//setting a new SECT
			if ( determinant > 0) SECT[SECT_CNT-1][1] = junc_sub;

			printf( "%s ", JUNC_NAME[junc_sub] );

			fscanf( fin, "%s", data );//the length (or the line name)

			if ( (determinant = atoi(data)) > 0 ) {
				SECT_LENGTH[SECT_CNT] = determinant;
				SECT[SECT_CNT][0] = junc_sub;
				LINE[SECT_CNT] = LINE_CNT;

				SECT_CNT++;
			}

		} while ( determinant > 0 );


		LINE_CNT++;
		strcpy( LINE_NAME[LINE_CNT], data);

		printf("\n");

	} while ( strcmp( data, "乗換") != 0 );

	printf( "%s: ", LINE_NAME[LINE_CNT] );//printf( "%s: ", "乗換" );
	fscanf( fin, "%s", data );

	//printing station list
	while ( strcmp( data, "END") != 0 ) {
		junc_sub = junc_search( data, JUNC_NAME, JUNC_CNT );

		printf( "%s-", JUNC_NAME[junc_sub] );
		SECT[SECT_CNT][0] = junc_sub;
		SECT_LENGTH[SECT_CNT] = 0;
		LINE[SECT_CNT] = LINE_CNT;

		fscanf( fin, "%s", data);
		junc_sub = junc_search( data, JUNC_NAME, JUNC_CNT );
		printf( "%d:%s ",JUNC_CNT, JUNC_NAME[junc_sub] );
		SECT[SECT_CNT][1] = junc_sub;

		SECT_CNT++;

		fscanf( fin, "%s", data );

	}

	fclose(fin);
	//loading process END

	//analitics process
	int TERMINAL_LIST[M], TERMINAL_LIST_CNT= 0;
	for ( i = 0; i < JUNC_CNT; i++ ) BRANCH_CNT[i] = 0;//initialize

	//counting branch, asigning junc_sub
	for ( i = 0; i < SECT_CNT; i++ ) {
		for ( j = 0; j < 2; j++ ) {
			junc_sub = SECT[i][j];
			SECT_NUM[junc_sub][ BRANCH_CNT[junc_sub] ] = i;
			BRANCH_CNT[junc_sub]++;
		}
	}

	//making and printing TERMINAL_LIST
	printf("\n\nTERMINALS:");
	for (i = 0; i < JUNC_CNT; i++) {
		if (BRANCH_CNT[i] == 1) {
			TERMINAL_LIST[TERMINAL_LIST_CNT] = i;
			printf("\n%d %s %d", TERMINAL_LIST_CNT, JUNC_NAME[ TERMINAL_LIST[TERMINAL_LIST_CNT] ], i);
			TERMINAL_LIST_CNT++;
		}
	}
	printf("\nTERMINAL_LIST_CNT = %d",TERMINAL_LIST_CNT);

	//printing JUNCs
	printf("\n\nJUNCS:\n");
	for ( i = 0; i < JUNC_CNT; i++ ) {

		//printing one JUNC
		printf( "%d %s (BRANCH COUNT:%d BRANCHES:", i, JUNC_NAME[i], BRANCH_CNT[i] );

		//printing opposite JUNCs
		for ( j = 0; j < BRANCH_CNT[i]; j++ ) {
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
	printf("\n\nSECTs:\n");
	for ( i = 0; i < SECT_CNT; i++ ) {
		printf( "%d ", i );
		printf( "%s-%s ", JUNC_NAME[SECT[i][0]], JUNC_NAME[SECT[i][1]] );
		printf( "[%s] ", LINE_NAME[LINE[i]]);
		printf( "%5.1fkm\n", SECT_LENGTH[i]*0.1 );
		SUM_SECT_LENGTH += SECT_LENGTH[i];
	}
	printf( "SECT_COUNT = %d\n", SECT_CNT );
	printf( "SUM_SECT_LENTGH = %5.1fkm\n", SUM_SECT_LENGTH * 0.1 );

	time_mark[0] = clock();
	printf("%d",time_mark[0] - time_start);



	//calculation prosess
	int junc_status[M], branch_status[M][M];
	int sect_temp_route[M], sect_record_route[M], junc_temp_route[M], junc_record_route[M];
	int record_length=0, temp_route_length=0, list_cnt, temp_list_cnt=0;
	int previous_junc, present_junc, next_junc;
	int previous_length=0;
	long long int valid_route_cnt=0;
	FILE *fout;


	for ( i = 0; i < TERMINAL_LIST_CNT; i++ ) {

		//calculate routes starting from a terminal
		printf("\nA New Terminal:%d\n",i);
		//setting temp data
		present_junc = TERMINAL_LIST[i];//setting the first junction
		junc_status[present_junc] = 1;//this means i-th junction is traveled
		junc_temp_route[temp_list_cnt] = present_junc;

		previous_junc = present_junc;


		do {
			//setting previous_junc

			//setting j (indicator) and searching next junc
			jmax = BRANCH_CNT[present_junc];
			for ( j = 0; j < jmax; j++ ) {
				//searching a free branch
				if ( branch_status[present_junc][j] == 0 ) {

					//setting next_junc
					selected_sect = SECT_NUM[present_junc][j];
					if ( present_junc == SECT[selected_sect][0] ) {
						next_junc = SECT[selected_sect][1];
					} else {
						next_junc = SECT[selected_sect][0];
					}

					if ( next_junc != previous_junc ) break;
				}

			}



			if ( j < jmax && junc_status[present_junc] < 2 ) {
				//updating status
				junc_status[next_junc]++;
				branch_status[present_junc][j] = 1;

				//updating temp data
				sect_temp_route[temp_list_cnt] = SECT_NUM[present_junc][j];
				junc_temp_route[temp_list_cnt + 1] = next_junc;

				previous_length = SECT_LENGTH[SECT_NUM[present_junc][j]];
				temp_route_length += previous_length;

				temp_list_cnt++;
				previous_junc = present_junc;
				present_junc = next_junc;

			} else {
				//checking if recordable

				valid_route_cnt++;

				if ( temp_route_length > record_length ) {
					//saving the length, sections, junctions of the temp route
					record_length = temp_route_length;
					list_cnt = temp_list_cnt;

					//renewing record SECTs and JUNCs
					for ( k = 0; k < list_cnt; k++ ) {
						sect_record_route[k] = sect_temp_route[k];
						junc_record_route[k] = junc_temp_route[k];
					}

					//renewing the record
					junc_record_route[list_cnt] = junc_temp_route[list_cnt];
					time_now = clock();
					printf("\nlongest route updated: %5.1fkm - checking %d of %d TERMINAL(s) - %dms passed"
							,record_length*0.1 ,i,TERMINAL_LIST_CNT, time_now-time_start);
					printf("\nvalid_route_cnt = %lld", valid_route_cnt );


					//archiving data
					if ( (fout = fopen( DEST_FILE, "w")) == NULL ) {
						printf("\nFile not created\n");
						exit(1);
					}

					fprintf( fout, "Terminal count:%d\n",i);

					fprintf( fout, "route:" );

					time_t now = time(NULL);
					struct tm *pnow = localtime(&now);

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
							pnow->tm_sec);
	

					fclose(fout);

				}

				//checking if coming present_junc for the first time
				if (junc_status[present_junc] == 1 ) {
					//resetting branch_status starting from present_junc
					kmax = BRANCH_CNT[present_junc];
					for ( k = 0; k < kmax; k++ ) {
						branch_status[present_junc][k] = 0;
					}
				}


				//backing to the previous JUNC
				junc_status[present_junc]--;
				temp_list_cnt--;
				present_junc = junc_temp_route[temp_list_cnt];
				previous_junc = junc_temp_route[temp_list_cnt - 1];
				temp_route_length -= SECT_LENGTH[ sect_temp_route[temp_list_cnt] ];

			}


		} while ( present_junc != junc_temp_route[0] );


	}

	//print the result of calculation
	printf("\nroute:" );
	for ( i = 0; i < list_cnt; i++ ) {
		printf("%s", JUNC_NAME[ junc_record_route[i] ] );
		printf("-[%s]-", LINE_NAME[ LINE[ sect_record_route[i] ] ] );
	}
	printf("%s\n", JUNC_NAME[ junc_record_route[list_cnt] ] );
	time_end = clock();
	printf("\n cal proccess %dms\n",time_end - time_mark[0]);
	double time_passed;
	time_passed = 1000*(time_end - time_start)/CLOCKS_PER_SEC;
	printf("Record Length: %5.1fkm\n %fs passed", record_length * 0.1, time_passed);
	printf("valid_route_cnt = %lld\n", valid_route_cnt);

	return 0;

}