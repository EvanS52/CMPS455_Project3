
//begin code changes made by Evan Soileau
#include "syscall.h"



int main(){
	SpaceId newProg;
	Write("Join test \n", 2, ConsoleOutput);
	newProg = Exec("../test/matmult");

	Join(newProg);
}
