/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void lru(int i, int curpage, Pentry q[], int timestamps[MAXPROCESSES][MAXPROCPAGES], int tick) {
    int min;
    int lruproc;
    //check if paged in
    if (!q[i].pages[curpage]) {
        //attempt to page in the current process and page
        if (!pagein(i,curpage)) { 
            //otherwise find and page the LRU page.
            min = tick;
            for (int x = 0; x < q[i].npages; x++){
                if (timestamps[i][x] < min && q[i].pages[x] == 1){
                    min = timestamps[i][x];
                    lruproc = x;
                }
            }
            pageout(i,lruproc);
        }
    }
}

void pageit(Pentry q[MAXPROCESSES]) {
    //Static
    static int init;
    static int tick;
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int prevpc[MAXPROCESSES];
    static int program[MAXPROCESSES];
    static int prevpage;

    //init if needed
    if (!init) {
        tick = 0;
        for (int x = 0; x < MAXPROCESSES; x++) {
            for (int y = 0; y < MAXPROCPAGES; y++) {
                timestamps[x][y] = 0;
            }
        }
        init = 1;
    }

    //Local Vars


    //loop through processes
    for (int i = 0; i < MAXPROCESSES; i++) {

        //get current page for current process
        int curpc = q[i].pc;
        int curpage = curpc / PAGESIZE;
        timestamps[i][curpage] = tick;
        
        
        int program_tell = prevpc[i] - curpc + 2; //not fully sure why + 2, but it makes everything match up.

        //debug print statement
        //printf("Program Tell: %d\n", program_tell);

        //if the program tell has a value and the program hasn't already been determined.
        if (program_tell > 0 && program[i] == 0) {
            if (program_tell == 1535) {
                if (i == 0){
                    printf("Program 1\n");
                }
                program[i] = 1;
            } else if (program_tell == 1131) {
                if (i == 0){
                    printf("Program 2\n");
                }
                program[i] = 2;
            } else if (program_tell == 1685 || program_tell == 518) {
                if (i == 0){
                    printf("Program 3\n");
                }
                program[i] = 3;
            //no 4 since it's a linear program.
            } else if (program_tell == 505 || program_tell == 503) {
                if (i == 0){
                    printf("Program 5\n");
                }
                program[i] = 5;
            }
        }


        //handle exits.
        if (  (program[i]==0 &&  curpc ==1911)//prog 4
				||(program[i]==1 &&  curpc ==1534) //prog 1
				||(program[i]==2 &&  curpc ==1130) //prog 2
				||(program[i]==3 &&  curpc ==1684) //prog 3
				||(program[i]==5 &&  curpc ==504) //prog 5
				) {
				program[i] = 0;
				prevpc[i] = -1;
		}

		pagein(i, curpage);
        
        if (prevpage != curpage && i == 0){
            printf("Page: %d\n", curpage);
            prevpage = curpage;
        }
        
            if (program[i] == 1) {
                switch (curpage) {
                case 0:
                    pagein(i, 1);
                    pageout(i, 5);
                    break;
                case 1:
                    pagein(i, 2);
                    pageout(i, 0);
                    break;
                case 2:
                    pagein(i, 3);
                    pageout(i, 1);
                    break;
                case 3:
                    pagein(i, 4);
                    pageout(i, 2);
                    break;
                case 4:
                    pagein(i, 5);
                    pageout(i, 3);
                    break;
                case 5:
                    pagein(i, 0);
                    pageout(i, 4);
                    break;
                default:
                    pagein(i, curpage+1);
                    pageout(i, curpage-1);
                    break;
                }
            } else if (program[i] == 2) {
                switch (curpage) {
                case 0:
                    pagein(i, 1);
                    pageout(i, 4);
                    break;
                case 1:
                    pagein(i, 2);
                    pageout(i, 0);
                    break;
                case 2:
                    pagein(i, 3);
                    pageout(i, 1);
                    break;
                case 3:
                    pagein(i, 4);
                    pageout(i, 2);
                    break;
                case 4:
                    pagein(i, 0);
                    pageout(i, 3);
                    break;
                default:
                    pagein(i, curpage+1);
                    pageout(i, curpage-1);
                    break;
                }
            } else if (program[i] == 3) {
                switch (curpage) {
                case 4:
                    pagein(i, 5);
                    pageout(i, 6);
                    break;
                case 5:
                    pagein(i, 6);
                    pageout(i, 4);
                    break;
                case 6:
                    pagein(i, 4);
                    pageout(i, 5);
                    break;
                default:
                    pagein(i, curpage+1);
                    pageout(i, curpage-1);
                    break;
                }
            } else if (program[i] == 5) {
                switch (curpage) {
                case 0:
                    pagein(i, 1);
                    break;
                case 1:
                    pagein(i, 0);
                    break;
                default:
                    pagein(i, curpage+1);
                    pageout(i, curpage-1);
                    break;
                }
            }

        //run the LRU
        lru(i, curpage, q, timestamps, tick);
        
        prevpc[i] = curpc;
    }
    tick++;
}
