/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {
    //Static
    static int init;
    static int tick;
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

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
    int oldest;
    int lruproc; //storage for the page to page out

    //loop through processes
    for (int i = 0; i < MAXPROCESSES; i++) {
        //get the current page to page in
        int curpage = q[i].pc / PAGESIZE;

        //update the timestamps data for the current page
        timestamps[i][curpage] = tick;

        //attempt to page in
        //either will return that it's paged in (or paging in) and we're good
        //or it will error and we need to page something out first.
        if (!pagein(i,curpage)) { 
            //if it doesn't complete, we need to page out
            //find and page the LRU page.
            oldest = tick; //every time has to be less than the current time
            //local LRU
            //loop through the pages for the current process
            for (int x = 0; x < q[i].npages; x++){
                //find the one with the longest since it's been used, and that's been paged in. (lowest timestamp saved)
                if (q[i].pages[x] == 1 && timestamps[i][x] < oldest){
                    //save the new minimum, and which process that is.
                    oldest = timestamps[i][x];
                    lruproc = x;
                }
            }
            //page it out, then we'll have to rerun the process to page in the new page.
            pageout(i,lruproc);
        }
    }
    /* advance time for next pageit iteration */
    tick++;
}
