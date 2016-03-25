/*
   e_hello_world.c

   Copyright (C) 2012 Adapteva, Inc.
   Contributed by Yaniv Sapir <yaniv@adapteva.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
   */

// This is the DEVICE side of the Hello World example.
// The host may load this program to any eCore. When
// launched, the program queries the CoreID and prints
// a message identifying itself to the shared external
// memory buffer.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_lib.h"

#define E_WAND_INT (0x8)

int trace_start_wait_all();
void __attribute__((interrupt)) wand_trace_isr(int signum);

int main(void) {
	const char		  ShmName[] = "hello_shm"; 
	const char        Msg[] = "Hello World from core 0x%03x!";
	char              buf[256] = { 0 };
	e_coreid_t		  coreid;
	e_memseg_t   	  emem;
	unsigned          my_row;
	unsigned          my_col;
	
	/////////////////////////////
	trace_start_wait_all();
	/////////////////////////////

	// Who am I? Query the CoreID from hardware.
	coreid = e_get_coreid();
	e_coords_from_coreid(coreid, &my_row, &my_col);

	if ( E_OK != e_shm_attach(&emem, ShmName) ) {
		return EXIT_FAILURE;
	}

	// Attach to the shm segment
	snprintf(buf, sizeof(buf), Msg, coreid);

	if ( emem.size >= strlen(buf) + 1 ) {
		// Write the message (including the null terminating
		// character) to shared memory
		e_write((void*)&emem, buf, my_row, my_col, NULL, strlen(buf) + 1);
	} else {
		// Shared memory region is too small for the message
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int trace_start_wait_all()
{
	unsigned irqState;
	e_irq_global_mask(E_FALSE);
	e_irq_attach(E_WAND_INT, wand_trace_isr);
	e_irq_mask(E_WAND_INT, E_FALSE);
	__asm__ __volatile__("wand"); // Set our WAND bit
	__asm__ __volatile__("idle"); // Idle and wait for an interrupt
	//irqState = e_reg_read(E_REG_STATUS);
	//irqState = irqState & (~0x8);  // This is the WAND interrupt flag
	//e_reg_write(E_REG_FSTATUS, irqState);
	//e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK); // Start counting
	return 0;
}


/**
 *  * This routine is installed with the interrupt Attach function
 *   * There will be no signal number attached when invoked
 *    * @param signum: not used
 *     */
void __attribute__((interrupt)) wand_trace_isr(int signum)
{
	(void)signum;
	//Intentionally left empty
	//
}
