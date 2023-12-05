/*
 * XCgettime.c
 *
 *  Created on: 27 oct. 2023
 * Copyright (C) 2023, fabrice oudert
 * https://github.com/fabriceo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "XCgettime.h"
#include <XS1.h>    //for get_logical_core_id

static volatile XCtime_t XCtime;

//macro to create a thread-safe code section { } with aquire and release of a given volatile mutex
#define XC_SWLOCK(lock) { unsigned ID = get_logical_core_id() + 1; \
        do { while ( lock ) { }; lock = ID; \
        asm volatile("nop;nop;nop;nop;nop;nop;nop"); } while( lock != ID ); } \
        for (int x = 1 ; x ; lock = --x )

//extend the XC 32bits timer to 64bits
inline long long XCgetTime() { asm volatile("#XCgetTime:");
    XCtime_t previous = { .ll = XCtime.ll };
    static volatile unsigned lock;
    XC_SWLOCK(lock)
    {
    asm ("maccu %0,%1,%2,%3"
         :"+r"(previous.ulh.hi),"+r"(previous.ulh.lo)
         : "r"(XC_GET_TIME() - previous.ulh.lo),"r"(1) );
    XCtime.ll = previous.ll;
    }
    return previous.ll;
}

#define MUL_2P32_DIV_100    42949673
#define MUL_2P40_DIV_100000 10995116

//return the real time timer value divided by 100.
//return as "int" is a choice in order to be abble to compare futur and actual easily
inline long long XCmicros(){ asm volatile ("#XCmicros:");
    XCtime_t local = { .ll = XCgetTime() };
    unsigned unused;
    asm("lmul %0,%1,%2,%3,%4,%5":"=r"(local.ulh.lo),"=r"(unused):"r"(local.ulh.lo),"r"(MUL_2P32_DIV_100),"r"(0),"r"(0));
    asm("lmul %0,%1,%2,%3,%4,%5":"=r"(local.ulh.hi),"=r"(local.ulh.lo):"r"(local.ulh.hi),"r"(MUL_2P32_DIV_100),"r"(0),"r"(local.ulh.lo));
    return local.ll;
}


inline long long XCmillis(){ asm volatile ("#XCmillis:");
    XCtime_t local = { .ll = XCgetTime() };
    //unsigned mandatory for shl 8
    local.ull >>= 8;
    unsigned unused;
    asm("lmul %0,%1,%2,%3,%4,%5":"=r"(local.ulh.lo),"=r"(unused):"r"(local.ulh.lo),"r"(MUL_2P40_DIV_100000),"r"(0),"r"(0));
    asm("lmul %0,%1,%2,%3,%4,%5":"=r"(local.ulh.hi),"=r"(local.ulh.lo):"r"(local.ulh.hi),"r"(MUL_2P40_DIV_100000),"r"(0),"r"(local.ulh.lo));
    return local.ll;
}

long long XCgetTime();
long long XCmicros();
long long XCmillis();

