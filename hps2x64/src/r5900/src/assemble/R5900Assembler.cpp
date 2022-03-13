/*
	Copyright (C) 2012-2016

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifdef INLINE_DEBUG
#include "WinApiHandler.h"
#endif


#include "R5900_assembler.h"


#include "StringUtils.h"
using namespace Utilities::Strings;
using namespace R5900::Utilities;

#include <string>
#include <iostream>


using namespace std;

// constant for the file extension of assembler output file
static const char* c_sFileExt = ".ps2";
static const char* c_sTargetName = "R5900";

void pause() __attribute__((destructor)); /* Function Declaration, You Cannot Declare Attributes Without This */

void pause()
{
   //cin.sync(); // Flush The Input Buffer Just In Case
   //cin.ignore(); // There's No Need To Actually Store The Users Input
   cin.get();
}

int main( int argc, char* argv [] )
{
	string InputFile, OutputFile;
	
	cout << "Assembler program for " << c_sTargetName << "\n";
	
	if ( argc < 2 )
	{
		cout << "One source file must be dropped on program for PS1 Assembler to run.\n";
		return 0;
	}
	
	cout << "\nStarting Assembler object...\n";
	
	Assembler* a = new Assembler ();
	
	cout << "\nGetting input file..\n";
	
	InputFile = argv [ 1 ];
	
	cout << "\nCreating output file name...\n";
	
	// create output file name
	OutputFile = Left ( InputFile, InStrRev ( InputFile, "." ) ) + c_sFileExt;
	
	cout << "\nSetting the input file for Assembler..\n";
	
	// set the files to use for assembler
	a->SetInputFile ( InputFile );
	
	cout << "\nSetting the output file for Assembler..\n";
	
	a->SetOutputFile ( OutputFile );
	
	cout << "\nPerforming PASS1\n";

	// assemble
	a->Pass1 ();
	
	cout << "\nPerforming PASS2\n";
	
	a->Pass2 ();
	
	cout << "\nDeleting assembler object...\n";
	
	delete a;
	
	cout << "\nDone.\n";
	
	return 0;
}





