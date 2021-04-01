// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
//**begin code changes by Patrick Courts***//
#include <iostream>
using namespace std;
//**begin code changes by Patrick Courts***//
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
// taslk
static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

//**begin code changes by Patrick Courts***//

// Define global int variable to offset phyiscal pages taken by a process.
int startPage = 0;

//**end code changes by Patrick Courts***//

AddrSpace::AddrSpace(OpenFile *executable, int Thread_id)
{

	
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

   // ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
//**begin code changes by Patrick Courts***//
	//if(numPages > NumPhysPages){
		
	//	cout << "Sufficient memory does not exist for your process." << endl;
		//currentThread->Finish();
		// exit a single process, will handle in Task 4 by changing addrspace constructor		
	//}
//**end code changes by Patrick Courts***//

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];



    for (i = 0; i < numPages; i++) {
	
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #

//**begin code changes by Patrick Courts***//
	
		//pageTable[i].physicalPage =

	pageTable[i].valid = FALSE; // For Task 3 (Deman Paging) set bit to false
//**end code changes by Patrick Courts***//

	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }





// then, copy in the code and data segments into memory


  
   //Write code for swap file 
	//Create a bufffer of size 
	char* buffer = new char[noffH.code.size + noffH.initData.size + noffH.uninitData.size];
        swapFilename = new char[20];
	sprintf(swapFilename, "%d.swap",Thread_id);
	printf("swapFilename:%s\n", swapFilename);
	//**begin code changes by Brody Fontenot**//
	if (replacementAlg == 1) {
		printf("FIFO Page Replacement\n");
	}else if (replacementAlg == 2) {
		printf("Random Page Replacement\n");
	}else if (replacementAlg == 0) {
		printf("Disable Virtual Memory (Demand Paging Only)\n");
	}
	printf("Number of physical pages: %d\n",numPages);	 //task 6 output
	printf("Page size: %d\n", PageSize);			 //task 6 output
	//**end code changes by Brody Fontenot**//
	printf("Thread ID: " , Thread_id,"\n");
	executable->ReadAt(buffer, noffH.code.size + noffH.initData.size + noffH.uninitData.size, sizeof(noffH));
	
	fileSystem->Create(swapFilename, noffH.code.size + noffH.initData.size + noffH.uninitData.size);
	OpenFile *swapFilePointer = fileSystem->Open(swapFilename);
	printf("swapFilename opened\n");
	swapFilePointer->WriteAt(buffer, noffH.code.size + noffH.initData.size + noffH.uninitData.size, 0);
	

   	delete swapFilePointer;
	delete buffer;

//	delete all pointers 

}
//**end code changes by Patrick Courts***//

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
