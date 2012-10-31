/*
DReAMSim v0.0
final revision November 23rd, 2010

This file is part of DReAMSim
http://sourceforge.net/projects/dreamsim/

Copyright © 2010 Arash Ostadzadeh (ostadzadeh@gmail.com)
http://www.ce.ewi.tudelft.nl/ostadzadeh/

DReAMSim is free software: you can redistribute it and/or modify 
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

DReAMSim is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU Lesser General Public License for more details. You should have 
received a copy of the GNU Lesser General Public License along with DReAMSim.
If not, see <http://www.gnu.org/licenses/>.

--------------
<LEGAL NOTICE>
--------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution. The names of the contributors 
must be retained to endorse or promote products derived from this software.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ITS CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
*/


//==============================================================================
/* DReAMSim.h: 
 * This is a sample code to demonstrate the utilization of DReAMSim.
 *
 *  Author: Arash Ostadzadeh
 *  Lastly revised on 23-11-2010
*/
//==============================================================================

#include "rng.cpp"
#include "DReAMSim.cpp"

int main(int argc, char **argv)
{
	unsigned long int TT=10000,TN=16,TC=30;  // some default values
	switch (argc)   // parse the command line to override default values
	{
		case 4: TT=atoi(argv[3]);  // total number of tasks
		case 3: TC=atoi(argv[2]);  // total number of configurations for the processing elements (nodes)
		case 2: TN=atoi(argv[1]);  // total number of processing elements (nodes)
		default:
		   	   break;
	}
		
	DReAMSim DS(TN,TC,TT);  // create the simulator object with the defined parameters
         
         DS.Start();  // start the simulation. A summary of the results will be created automatically after the completion of the simulation.
	
	return 0;
}
