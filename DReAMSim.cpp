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
/* DReAMSim.cpp: 
 * This file defines the required routines for the simulation of task scheduling in DReAMSim.
 *
 *  Author: Arash Ostadzadeh
 *  Lastly revised on 23-11-2010
*/
//==============================================================================

#include "DReAMSim.h"

using namespace std;

DReAMSim::DReAMSim(unsigned int TN,unsigned int TC, unsigned long int TT, unsigned int NextTaskMaxInterval,
			   unsigned int NlowA, unsigned int NhighA,
			   unsigned int TlowA, unsigned int ThighA,
		       unsigned int TRTlow, unsigned int TRThigh)
{
	TotalNodes=TN;
	TotalConfigs=TC;
	TotalTasks=TT;

	NodelowA=NlowA;
	NodehighA=NhighA;

	TasklowA=TlowA;
	TaskhighA=ThighA;

	TaskReqTimelow=TRTlow;
	TaskReqTimehigh=TRThigh;

	DReAMSim::NextTaskMaxInterval=NextTaskMaxInterval;
	
	TotalCompletedTasks=0;
	TotalCurGenTasks=0;
	TotalCurSusTasks=0;
	TotalDiscardedTasks=0;
	
	suspendedlist=NULL; // no task initially in the suspension list

	// for report
	Total_Wasted_Area=0;
	Total_Search_Length_Scheduler=0;
	Total_Task_Wait_Time=0;	
	Total_Tasks_Running_Time=0;
	
	
	TimeTick=0;
	
	InitNodes();
	InitConfigs();
	
}

void DReAMSim::InitNodes()
{
	int i;
	Node *t;
	
	blanklist=new Node*[TotalNodes];
	if(!blanklist) { cerr<<"\nError in memory allocation.\n"; exit(1);}
	
	CurBlankNodeIndex=0;
	//nodesList=NULL; // no node initially in the already configured linked list
	
	for(unsigned int i=0;i<TotalNodes;i++)
    {

		t=new Node;
		if(!t) { cerr<<"\nError in memory allocation.\n"; exit(1);}
		
		blanklist[i]=t;
		t->NodeNo=i+1;  // The Node numbers are starting from 1.
		t->Area=(x.rand_int31()%(NodehighA-NodelowA+1))+NodelowA;
		t->ConfigNo=0;  // The node initial is blank
		t->Bnext=NULL;
		t->Inext=NULL;
		t->CurTask=NULL;
	}
}

void DReAMSim::InitConfigs()
{
	int i;
	
	configs=new Config[TotalConfigs+1];
	if(!configs) { cerr<<"\nError in memory allocation.\n"; exit(1);}

	for(unsigned int i=0;i<TotalConfigs+1;i++)
	{
		configs[i].ConfigNo=i;  // ConfigNo are beginning from 1.
    	configs[i].idle=configs[i].busy=NULL;
    }
}

void DReAMSim::AddNodeToBusyList(Node *n)
{
	// update the busy list for the current node/config
	// add the node to the current busy list at the starting point

	n->Bnext=configs[n->ConfigNo].busy;
	configs[n->ConfigNo].busy=n;
}	

void DReAMSim::AddNodeToIdleList(Node *n)
{
	// update the idle list for the current node/config
	// add the node to the current idle list at the starting point

	n->Inext=configs[n->ConfigNo].idle;
	configs[n->ConfigNo].idle=n;
}	

void DReAMSim::RemoveNodeFromIdleList(Node *n)
{
	Node * temp=configs[n->ConfigNo].idle;
	if (temp==n) configs[n->ConfigNo].idle=temp->Inext;
	else
	{
		while(temp->Inext && temp->Inext!=n)	temp=temp->Inext;
		temp->Inext=temp->Inext->Inext;
	}
}	

void DReAMSim::RemoveNodeFromBusyList(Node *n)
{
	Node * temp=configs[n->ConfigNo].busy;
	if (temp==n) configs[n->ConfigNo].busy=temp->Bnext;
	else
	{
		while(temp->Bnext && temp->Bnext!=n)	temp=temp->Bnext;
		temp->Bnext=temp->Bnext->Bnext;
	}
}	


void DReAMSim::TaskCompletionProc(Node* cur)
{
	//print task report summary
	cout<<"Node # "<< cur->NodeNo << " finished executing Task # "<< cur->CurTask->TaskNo <<endl;
	cout<<"Starting time: "<< cur->CurTask->StartTime <<"     CompletionTime= "<< cur->CurTask->CompletionTime <<endl;
	cout<<"elapsed time ticks: " << cur->CurTask->RequiredTime <<endl;
	if (cur->CurTask->SusRetry) cout<<"The task had been initially put in the suspension queue. "
								<< cur->CurTask->SusRetry << " retries have been carried out before accommodation\n";
	
	
	//update report statistics
	TotalCompletedTasks++;
	Total_Tasks_Running_Time+=cur->CurTask->RequiredTime;
	
	// remove the node from current busy list
	RemoveNodeFromBusyList(cur);
	// add the node to the current idle list
	AddNodeToIdleList(cur);
	
	delete cur->CurTask;
	cur->CurTask=NULL;// current node becomes available
}

void DReAMSim::SendTaskToNode(Task *t, Node *n)
{
	cout<<"Node # "<< n->NodeNo << " has started executing task # "<< t->TaskNo << endl;
	cout<<"Task creation time: "<< t->CreateTime <<endl;
	t->StartTime=TimeTick;
	t->CompletionTime=TimeTick + t->RequiredTime;
	
	cout<<"start time: "<< t->StartTime <<"     completion time: "<< t->CompletionTime << endl;
	
	n->CurTask=t;
	
	// remove the node from current idle list
	RemoveNodeFromIdleList(n);
	// add the node to the current busy list
	AddNodeToBusyList(n);
	
	// update some report statistics
	Total_Wasted_Area+= n->Area - t->NeededArea;
	Total_Task_Wait_Time+= t->StartTime - t->CreateTime;
}

Task * DReAMSim::CreateTask()
{
	Task *t;
	
	TotalCurGenTasks++; // new task is going to be created
	t=new Task;
	if (!t) { cerr<<"\nError in memory allocation for Task # " << TotalCurGenTasks <<"\n"; exit(1);}
	
	t->TaskNo=TotalCurGenTasks;
	t->NeededArea=(x.rand_int31()%(TaskhighA-TasklowA+1))+TasklowA;
	
	// we will assume about 10% of the created tasks preferring a configuration which is not available in the system
	// the criterion can be changed later
	t->PrefConfig=1 + ( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) ); 
	
	t->CreateTime=TimeTick;
	t->RequiredTime=(x.rand_int31()%(TaskReqTimehigh-TaskReqTimelow+1))+TaskReqTimelow;
	t->SusRetry=0;
	
	cout<<"Task # " << TotalCurGenTasks << " submitted to the system at time # " << TimeTick << endl;
	// can print other information about the task here
	
	return t;
}

Task * DReAMSim::CheckSuspensionQueue(Node *cur)
{
	SusList* temp=suspendedlist;
	// the search for a match on the suspension queue will terminate as soon as a matched node with the same config and available area found
	// could be a point for further improvement (a more intelligent match making to optimize waste area ,...)
	
	SusList* prev_temp=NULL; // this always precedes temp
	
	while(temp) 
	{
		if (cur->ConfigNo==temp->item->AssignedConfig && cur->Area >= temp->item->NeededArea) 
		 // a match is found
		{
			Task * t=temp->item;
			
			//update the suspended tasks list
			if (!prev_temp) // first node to be removed from suspension queue
							suspendedlist=temp->next;
			else prev_temp->next=temp->next;
			
			delete temp;
			return t;
		}
		
		temp->item->SusRetry++; // this task was agained tested to be assigned to a node without success
								// the info looks stupid for now but can be useful later. for now the list is checked on every time tick (crazy)!!!!
		prev_temp=temp;
		temp=temp->next;
	}
	
	return NULL; // no suitable task found the recently released node
}

Config* DReAMSim::findPreferredConfig(Task *t)
{
	// for now just a stupid straightforward check!
	if (t->PrefConfig <= TotalConfigs) return &configs[t->PrefConfig];
	return NULL; // no exact match
}


Config* DReAMSim::findClosestConfig(Task *t)
{
	// for now just a random config # will be picked up as the closest match
	return &configs[(x.rand_int31()%TotalConfigs)+1];
}
	
	
Node* DReAMSim::findAnyIdleNode(Task* t,unsigned long int& SL)
{
 	unsigned long int c=0;
 	// trying to find any (first) available idle node with any kind of configuration
 	
	while( c < CurBlankNodeIndex)
 	{
 		if (!blanklist[c]->CurTask && blanklist[c]->Area >= t->NeededArea) return blanklist[c];
  		c++;
  		SL++;
 	}
	return NULL;
}

Node* DReAMSim::findBestBlankNodeMatch(Task* t,unsigned long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff;
 	signed long int temp;
 	signed long int counter=CurBlankNodeIndex;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(counter<TotalNodes)			// find the first suitable node
 	{
 		if (blanklist[counter]->Area >= t->NeededArea) 
 		{
 			bestMatchindex=counter;
 			mindiff=blanklist[counter]->Area - t->NeededArea;
 			break;
 		}
  		counter++;
  		SL++;
 	}
	
	if(bestMatchindex==-1) return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		temp=blanklist[counter]->Area - t->NeededArea;
		
 		if ( (temp>=0) && ( mindiff > temp ) ) 
 		{
 			bestMatchindex=counter;
 			mindiff=temp;
 		}
  		SL++;
	}	
	// swap the node found at bestmatch and the node residing on the CurBlankNodeIndex to keep the already assigned nodes and blank nodes in two
	// separate parts

	Node * tempn=blanklist[CurBlankNodeIndex];
	blanklist[CurBlankNodeIndex]=blanklist[bestMatchindex];
	blanklist[bestMatchindex]=tempn;
	
	CurBlankNodeIndex++;
	return blanklist[CurBlankNodeIndex-1];
}

	
Node* DReAMSim::findBestNodeMatch(Task* t,Node *idlelist,unsigned long int& SL)
{
	Node * bestMatch=NULL;
 	unsigned long int mindiff;
 	signed long int temp;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(idlelist)			// find the first suitable node
 	{
 		if (idlelist->Area >= t->NeededArea) 
 		{
 			bestMatch=idlelist;
 			mindiff=idlelist->Area - t->NeededArea;
 			break;
 		}
  		idlelist=idlelist->Inext;
  		SL++;
 	}
	
	if(!bestMatch) return NULL;
	
	while(idlelist)	// check the remaining nodes to find a better match
	{
		temp=idlelist->Area - t->NeededArea;
		
 		if ( (temp>=0) && ( mindiff > temp ) ) 
 		{
 			bestMatch=idlelist;
 			mindiff=temp;
 		}
  		idlelist=idlelist->Inext;
  		SL++;
	}	

	return bestMatch;
}



void DReAMSim::makeNodeBlank(Node *n)
{
	cout<<"making Node # " << n->NodeNo <<" blank"<<endl;
}


void DReAMSim::sendBitstream(Node *n)
{
	cout<<"sending bitstream for configuration # " << n->ConfigNo << " to the Node # " <<n->NodeNo <<endl;
}

bool DReAMSim::queryBusyListforPotentialCandidate(Task *t, unsigned long int& SL )
{
	Node *temp=configs[t->AssignedConfig].busy;
	
	while(temp)
	{
		if( temp->Area >= t->NeededArea ) return true;
		temp=temp->Bnext;
		SL++;
	}
	
	return false;
}

void DReAMSim::PutInSuspensionQueue(Task *t)
{
	SusList* cur;
	
	cur=new SusList;
	if(!cur) { cerr<<"\nError in memory allocation for suspended task list.\n"; exit(1);}
	
	cout<<"puting the task # "<<t->TaskNo<<" on the suspended tasks queue\n";
	cur->item=t;
	cur->next=suspendedlist;
	suspendedlist=cur;
	TotalCurSusTasks++;
}

void DReAMSim::DiscardTask(Task *t)
{
	cout<<"Task # "<<t->TaskNo<<" can not be accommodated by the scheduler. Discarding the task!"<<endl;
	TotalDiscardedTasks++;
}

void DReAMSim::RunTaskScheduler(Task *t)
{
	Config * Cmatch,* Closestmatch;
	bool found=false;
	unsigned long int SL=0;
	Node *n;
	
	Cmatch=findPreferredConfig(t);
	
	if (Cmatch) // required configuration was found in the list (exact match)
	{
		t->AssignedConfig=Cmatch->ConfigNo; // set the assigned config for the current task
		
		if(Cmatch->idle) // there are idle nodes available with the preferred config
		{
			n=findBestNodeMatch(t,Cmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
		
			if (n) 
			{	
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(t,n);  // found a suitable node
				found=true;
			}
		}
		if(!found) // no suitable idle node found at this point
		{
			if(CurBlankNodeIndex<TotalNodes) // blank node(s) still available
			{
				n=findBestBlankNodeMatch(t,SL);
				if (n)
				{
					Total_Search_Length_Scheduler+=SL;
					n->ConfigNo=Cmatch->ConfigNo; // change the ConfigNo of the blank node from 0 to the configuration match
					sendBitstream(n);
					AddNodeToIdleList(n); // this is something stupid!!! extra here because the node was initially blank we first add it to the idle
									  // list of corresponding config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(t,n);
					found=true;
				}
			}// end of blank node(s) still available
			
			if(!found) // no blank node available or there is no suitable blank node available!!!
			{			// try reconfiguring one of the idle nodes!
				n=findAnyIdleNode(t,SL);
				if (n) // An idle node is found for reconfiguration
				{
					Total_Search_Length_Scheduler+=SL;
					RemoveNodeFromIdleList(n);
					makeNodeBlank(n);
					n->ConfigNo=Cmatch->ConfigNo; // change the ConfigNo of the idle node to the configuration match
					sendBitstream(n);
					AddNodeToIdleList(n); // this is something stupid!!! extra here because the node is not in the idle
									  // list of new config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(t,n);
					found=true;
				}
			}// end of doing reconfiguration for one of the idle nodes
		}// exact match found / but the was no idle nodes available
		//--------------------------------------------------------------------------------------------------------
		// if you want to wait for a busy node with exact match to become available this is the point to add the rountine!!!
		//---------------------------------------------------------------------------------------------------------
	
	}// end of exact match for the configuration

	if(!found) // no exact match! or we have exact match but can not accommodate in any way the current task at this moment!
	{
		Closestmatch=findClosestConfig(t); // we assume that there is always a closest config match
											// for now because we actually do not look for closest match the SL is not increased,
											// in reality SL is also increased correspondingly to indicate the search effort
		t->AssignedConfig=Closestmatch->ConfigNo; // set the assigned config for the current task
		
		if(Closestmatch->idle) // there are idle nodes available with the closest config
		{
			n=findBestNodeMatch(t,Closestmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
		
			if (n) 
			{	
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(t,n);  // found a suitable node
				found=true;
			}
		}
		if(!found) // no (suitable) idle nodes available
		{
			// in case we are coming from an exact match configuration part, we have already searched the blank list and no need for a re-search
			if( !Cmatch	&& CurBlankNodeIndex<TotalNodes) // blank node(s) still available
			{
				n=findBestBlankNodeMatch(t,SL);
				if (n)
				{
					Total_Search_Length_Scheduler+=SL;
					n->ConfigNo=Closestmatch->ConfigNo; // change the ConfigNo of the blank node from 0 to the closest configuration match
					sendBitstream(n);
					AddNodeToIdleList(n); // this is something stupid!!! extra here because the node was initially blank we first add it to the idle
									  // list of corresponding config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(t,n);
					found=true;
				}
			}// end of blank node(s) still available
		}
		
		if(!found) // no blank node available or there is no suitable blank node available!!!
			     	   // we are going to wait for a busy node to become idle!
						// The last solution!
		{
				if(Cmatch) 
					t->AssignedConfig=Cmatch->ConfigNo; // adjust the assigned config field again if needed, in case there was previously an exact match
				
				found=queryBusyListforPotentialCandidate(t,SL); // determine whether or not the current busy list has any potential candidate for 
																// accommodating the task regarding the Area restriction
				Total_Search_Length_Scheduler+=SL;
																				
				if( found )   // if yes
					PutInSuspensionQueue(t);

				// we can also go for another check with all busy nodes and do a reconfig later if needed, not considered in this scheduler!!!
				else DiscardTask(t); // bad task, throw it away! 
		} // end of waiting!
	}// end of no exact match or we had exact match but were not able to accommodate on exact config list
}

void DReAMSim::MakeReport()
{
	ofstream f;
	
	    sprintf (fileName, "%d", TotalTasks);
	    strcat(fileName, ".txt");
	  	  
		f.open(fileName);
	if (f.fail()) { cout<<"\n failed opening the simulation report file.\n"; exit(1); }
	  
	
	f<<"total_tasks_generated\t"<<TotalTasks<<endl;
	f<<"total_PEs\t"<<TotalNodes<<endl;
	f<<"total_configurations\t"<<TotalConfigs<<endl;
	f<<"total_used_PEs\t"<<CurBlankNodeIndex<<endl;
	f<<"total_simulation_time\t"<<TimeTick<<endl;


	f<<"task_generation_interval\t[ 1 ... "<<NextTaskMaxInterval<<" ]"<<endl;
	f<<"PE_available_area_range\t[ "<<NodelowA<<" ... "<<NodehighA<<" ]"<<endl;
	f<<"task_required_area_range\t[ "<<TasklowA<<" ... "<<TaskhighA<<" ]"<<endl;
	f<<"task_required_timeslice_range\t[ "<<TaskReqTimelow<<" ... "<<TaskReqTimehigh<<" ]"<<endl;
	
	f<<"total_tasks_completed\t"<<TotalCompletedTasks<<endl;
	f<<"total_tasks_discarded\t"<<TotalDiscardedTasks<<endl;
	f<<"total_wasted_area\t"<<Total_Wasted_Area<<endl;
	f<<"average_wasted_area_per_task\t"<<(Total_Wasted_Area)/(double)(TotalCompletedTasks)<<endl;
	
	f<<"total_scheduling_steps\t"<<Total_Search_Length_Scheduler<<endl;
	f<<"average_scheduling_steps_per_task\t"<<(Total_Search_Length_Scheduler)/(double)(TotalTasks)<<endl;
	
	f<<"total_tasks_waiting_time\t"<<Total_Task_Wait_Time<<endl;
	f<<"average_task_waiting_time\t"<<(Total_Task_Wait_Time)/(double)(TotalCompletedTasks)<<endl;
	
	f<<"total_tasks_running_time\t"<<Total_Tasks_Running_Time<<endl;
	f<<"average_task_running_time\t"<<(Total_Tasks_Running_Time)/(double)(TotalCompletedTasks)<<endl;
	
	f.close();
}

void DReAMSim::Start()
{
	unsigned long long int nextIncomTaskTimeTick=0;
	Task* t;
	
	while( ( TotalCompletedTasks + TotalDiscardedTasks ) < TotalTasks )  // still there are tasks which are not finished
	{
		
		nextIncomTaskTimeTick=TimeTick + 1 + (x.rand_int31() % NextTaskMaxInterval);
		IncreaseTimeTick();  // advance one time tick
		
		// can be improved later because there is actually no need to check on every time tick
		while(TimeTick<=nextIncomTaskTimeTick) // check for completed tasks first then check whether or not suspended tasks could be accomodated
		{
			for(unsigned int i=0;i<CurBlankNodeIndex;i++) 
				if(blanklist[i]->CurTask) // there is a task running at this node
				{
					if( blanklist[i]->CurTask->CompletionTime==TimeTick) //task termination housekeeping
					{
						Task *tsq;
						TaskCompletionProc(blanklist[i]);
						tsq=CheckSuspensionQueue(blanklist[i]);
						if (tsq) // found a task in suspension queue to accomodate
						{
							cout<<"removing task # "<< tsq->TaskNo <<" from suspension queue\n";
							// send the task to the recently released node
							SendTaskToNode(tsq,blanklist[i]);
							TotalCurSusTasks--; 
						}
					}
				}
			IncreaseTimeTick();  // advance one time tick
		}// end of while ( TimeTick<=nextIncomTaskTimeTick )
		
		DecreaseTimeTick();  // the time needs to be adjusted just for the last unsuccessful increase in the TimeTick, otherwise we miss
							//  one of the ticks here! Note that the current TimeTick at this point is one unit ahead of the actual value
		
		//create the new scheduled task
		if ( TotalCurGenTasks < TotalTasks ) // still we need to generate more tasks!
		{
			t=CreateTask();
			//send the created task to the scheduler 
			RunTaskScheduler(t);
		}
	}// main loop of the simulation
	MakeReport(); 	// end of the simulation, make the final report
}
