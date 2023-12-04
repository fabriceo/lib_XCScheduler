/*
 * demo_ep0_tasks.c
 *
 *  Created on: 1 déc. 2023
 *      Author: fabriceo
 *
 *      this is an example of 2 cooperative tasks runing in the core
 *      allocated to Endpoint0 in the usb audio application (tested 7.3.1)
 *
 *      file is compatible with evaluation kit or multichannel board xu316
 */

#include <XS1.h>
#include <platform.h>

#ifdef XSCOPE
#include <stdio.h>
#define xprintf(...) printf(__VA_ARGS__)
#else
#define xprintf(...)
#endif

#include "XCscheduler.h"
#include "XCgettime.h"

// hard code for accessing port ressource, "a" is the port ressource adresse defined in XS1.h
#define setc(a,b)    {__asm__ __volatile__("setc res[%0], %1": : "r" (a) , "r" (b));}
#define portin(a,b)  {__asm__ __volatile__("in %0, res[%1]": "=r" (b) : "r" (a));}
#define portout(a,b) {__asm__ __volatile__("out res[%0], %1": : "r" (a) , "r" (b));}
#define portpeek(a,b){__asm__ __volatile__("peek %0, res[%1]": "=r" (b) : "r" (a));}

#if defined(PORT_USB_FLAG2) //typicaly for MC_XU216 from xk-audio-216-mc.xn
unsigned pbuttons = {XS1_PORT_4B};
unsigned pleds_row= {XS1_PORT_4C};  //row
unsigned pleds    = {XS1_PORT_4D};  //column
#elif defined(PORT_PLL_REF) //typically for MC_XU316 in xk-audio-316-mc.xn
unsigned pbuttons = {XS1_PORT_4E};
unsigned pleds    = {XS1_PORT_4F};
#else //typically for EVK_XU316 in XCORE-AI-EXPLORER.xn
unsigned pbuttons = {PORT_BUTTONS};
unsigned pleds    = {PORT_LEDS};
#endif

void ep0_task2(){
    unsigned leds=0, buttons, mask=0;
    setc(pbuttons,  XS1_SETC_INUSE_ON);
    setc(pleds,     XS1_SETC_INUSE_ON);
    setc(pleds,     XS1_SETC_DRIVE_DRIVE);
#if defined(PORT_USB_FLAG2)
    setc(pleds_row, XS1_SETC_INUSE_ON);
    setc(pleds_row, XS1_SETC_DRIVE_DRIVE);
    portout(pleds_row, 14);//row 0 activated
    mask = 15;
#else
#endif
    while (1) {
        portpeek( pbuttons, buttons );
        buttons &= 3; buttons ^= 3;
        leds &= 12;
        leds |= buttons;
        leds ^= 8;  //switch led3
        portout( pleds, leds ^ mask );
        yield_milliseconds(100);
    }
}


void ep0_task1(){
    for (int i=0; i<10; i++) {
        xprintf("%lld hello world ep0_task1 %d\n",XCmillis(),i);
        yield_milliseconds(1000);  //1sec
    }
    xprintf("ep0_task1 ends after 10 borring prints\n");
}


//this globally defined function will overwrite the weak version in XUD_user.c
void XUD_UserYieldChanend( unsigned ch ) {
    static unsigned task_created;
    if ( task_created == 0 ) {
        XCSchedulerCreateTask(ep0_task1);
        XCSchedulerCreateTask(ep0_task2);
        xprintf("%lld 2 tasks created\n",XCmillis());
        task_created = 1;
    } else
        yieldChanend(ch);
}
