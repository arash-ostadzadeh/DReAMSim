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
 * This is the header file for the simulation of task scheduling in DReAMSim.
 *
 *  Author: Arash Ostadzadeh
 *  Lastly revised on 23-11-2010
*/
//==============================================================================


#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#include "rng.h"

#ifndef _DReAMSim_
#define _DReAMSim_

typedef struct
{
	unsigned int TaskNo;
	unsigned long int NeededArea;
	unsigned int PrefConfig;
	unsigned int AssignedConfig;
	unsigned long long int CreateTime;
	unsigned long long int StartTime;
	unsigned long long int CompletionTime;
	unsigned long long int RequiredTime;
	unsigned long long int SusRetry;
} Task;

typedef struct SL
{
	Task* item;
	struct SL* next;
} SusList;   // the list is only used to keep track of suspended tasks

typedef struct N 
{
	unsigned int NodeNo;
	unsigned int ConfigNo;
	unsigned long int Area;
	Task * CurTask; // can also be used to detect whether or not the node is idle
	struct N * Inext;
	struct N * Bnext;
} Node;

typedef struct
{
	unsigned int ConfigNo; // starting at 1...TotalConfigs
	Node * idle;
	Node * busy;
} Config;


class DReAMSim
{
	public:
			DReAMSim(unsigned int TN=100,unsigned int TC=10, unsigned long int TT=10000, 
					unsigned int NextTaskMaxInterval=50, unsigned int NlowA=1000, unsigned int NhighA=5000,
					unsigned int TlowA=100, unsigned int ThighA=2500,
					unsigned int TRTlow=100, unsigned int TRThigh=10000);
					
			void Start();

	private:
			char fileName[100];
			void InitNodes();
			void InitConfigs();
			void IncreaseTimeTick(unsigned int lapse=1) { TimeTick+=lapse; }
			void DecreaseTimeTick(unsigned int lapse=1) { TimeTick-=lapse; }
			unsigned int FindClosestConfig(unsigned int); // for now the simulator only picks a random number for the closest configuration match
			void TaskCompletionProc(Node *);
			Task * CheckSuspensionQueue(Node *);  // check suspended tasks for a suitable match of the already released node
			void SendTaskToNode(Task *,Node *);
			Task * CreateTask();
			void AddNodeToBusyList(Node *);
			void RemoveNodeFromBusyList(Node *);
			void AddNodeToIdleList(Node *);
			void RemoveNodeFromIdleList(Node *);
			Config* findPreferredConfig(Task *);
			Config* findClosestConfig(Task *);
			Node* findAnyIdleNode(Task* ,unsigned long int& );
			Node* findBestBlankNodeMatch(Task* ,unsigned long int& );
			Node* findBestNodeMatch(Task* ,Node *,unsigned long int&);
			void makeNodeBlank(Node *);
			void sendBitstream(Node *);
			void DiscardTask(Task *);
			void PutInSuspensionQueue(Task * );
			bool queryBusyListforPotentialCandidate(Task *, unsigned long int& );
			void MakeReport();
			
			// Task Scheduler.... different strategies should be implemented as the body of this function
			void RunTaskScheduler(Task *);

			unsigned int TotalTasks;  // total number of synthatic tasks to be generated
			Node ** blanklist;		// initially all the created nodes are blank and they will be configured as the sim progresses
									// the blanklist contains non-blank nodes up to CurBlankNodeIndex, the rest of the list is blank

			unsigned int CurBlankNodeIndex;
			unsigned int TotalConfigs; // total number of configurations
			RNG x;		// random number 
			unsigned int TotalNodes;
			
			unsigned long int TotalCompletedTasks; // total number of completed tasks to determine the simulation termination time
			unsigned long int TotalCurGenTasks;
			unsigned long int TotalCurSusTasks;
			unsigned long int TotalDiscardedTasks;
			
			
			// Node * nodesList;
			Config * configs;
			SusList * suspendedlist;
			
			// needed for random number generation
			unsigned int NextTaskMaxInterval;
			unsigned int NodelowA,NodehighA;
			unsigned int TasklowA,TaskhighA;
			unsigned int TaskReqTimelow,TaskReqTimehigh;
			
			unsigned long int Total_Wasted_Area;
			unsigned long int Total_Search_Length_Scheduler;
			unsigned long int Total_Task_Wait_Time;
			unsigned long int Total_Tasks_Running_Time;
			
			unsigned long long int TimeTick;
};

#endif
