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

//main prediction program
int predict(int program, int currPage) {
    //If the current running program is program 1 (from the writeup)
    //Loops 0, 1, 2, 3, 4, 5
    //(with some randomness, but this catches most cases)
    if (program == 1) {
        if (currPage != 5) { 
            return currPage + 1;
        } else {
            return 0;
        }
    //If the current running program is program 2 (from the writeup)
    //Loops 0, 1, 2, 3, 4
    //(with some randomness, but this catches most cases)
    } else if (program == 2) {
        if (currPage != 4) {
            return currPage + 1;
        } else {
            return 0;
        }
    //If the current running program is program 3 (from the writeup)
    //Loops 4, 5, 6
    //(with some randomness, but this catches most cases)
    } else if (program == 3) {
        if (currPage != 6) {
            return currPage + 1;
        } else {
            return 4;
        }
    //If the current running program is program 5 (from the writeup)
    //Loops 0, 1
    //(with some randomness, but this catches most cases)
    } else if (program == 5) {
        if (currPage != 1) {
            return currPage + 1;
        } else {
            return 0;
        }
    }
    //default case, just guess that the next page is the one next in line. not perfect, but it's not bad
    return currPage + 1;
}


//Pagein program (based off simple pager from piazza)
void pageit(Pentry q[MAXPROCESSES]) {
    //Static Variables
    static int prevpc[MAXPROCESSES]; //track the previous program counter (used for detecting the program)
    static int program[MAXPROCESSES]; //track which program is running on each process.

    //loop through processes.
    for (int i = 0; i < MAXPROCESSES; i++) {

        //get current page for current process
        int currPc = q[i].pc;
        
        //program tell. this converts the program counter to the program counters given in programs.c
        int program_tell = prevpc[i] - currPc + 2; //not fully sure why + 2, but it makes everything match up.
        
        //debug print statement, don't mind this.
        //printf("Program Tell: %d\n", program_tell);

        //if the program tell has a value and the program hasn't already been determined.
        if (program_tell > 2 && program[i] == 0) { // >2 because of the +2 above
            if (program_tell == 1535) { //Program 1
                program[i] = 1;
            } else if (program_tell == 1131) { //Program 2
                program[i] = 2;
            } else if (program_tell == 1685 || program_tell == 518) { //Program 3 (second number because the program counter doesn't jump back to 0 in this case.)
                program[i] = 3;
            //no 4 since it's a linear program, there's also no
            } else if (program_tell == 505 || program_tell == 503) { //Program 5 (second number because the program counter doesn't jump back to 0 in this case.)
                program[i] = 5;
            }
        }

        //It appears that each process can run multiple programs...
        //When the program ends, reset the program tracker so we can make the correct recommendations.
        if (program[i] == 1 &&  currPc == 1534) { //last pc of program 1 is 1534
            program[i] = 0;
        } else if (program[i] == 2 &&  currPc == 1130) { //last pc of program 2 is 1130
            program[i] = 0;
        } else if (program[i] == 3 &&  currPc == 1684) { //last pc of program 3 is 1684
            program[i] = 0;
        } else if (program[i] == 0 &&  currPc == 1911) { //last pc of program 4 is 1911
            program[i] = 0;
        } else if (program[i] == 5 &&  currPc == 504) { //last pc of program 5 is 504
            program[i] = 0;
        }

        prevpc[i] = currPc; //update the previous program counter array.

        //get current page for current process
        int currPage = currPc / PAGESIZE;
        //get prediction from predication algo
        int prediction = predict(program[i], currPage);

        //Page out handler. Based heavlily on what was covered at the beginning of lecture on nov 11
        //Only allow 2 pages to be paged in for each process, the current page, and the next predicted page.

        for(int curr=0; curr < MAXPROCPAGES; curr++){

            //if the page is paged in, is not the current page, or the next predicted page, page it out.
            if(q[i].pages[curr] && curr != currPage && curr != prediction) {
                pageout(i, curr);
            }
        }
        //page in the current page (shouldn't do much most of the time)
		pagein(i, currPage);
        //page in our next predicted page
        pagein(i, prediction);
    }
}
