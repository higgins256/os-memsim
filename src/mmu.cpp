#include <iomanip>
#include <math.h>
#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        proc->variables.push_back(var);
    }
}

int Mmu::isProcessInMMU(uint32_t pid){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            return 1;  
		}
	}

    return 0; //if no match is found
}

Process* Mmu::getProcessAt(uint32_t pid){
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            return _processes[i];
        }
    }

    return NULL;
}

Variable* Mmu::getVariableAt(uint32_t pid, std::string desiredVar){
    Process *p = getProcessAt(pid);

    if(p == NULL){
        std::cout << "error: process does not exist";
        return NULL; //self explanatory
	}

    //std::cout << "looking for " << desiredVar << '\n';

    for(int i = 0; i < p->variables.size(); i++){
        //std::cout << "Is " << p->variables[i]->name << "equal to" << desiredVar << " \n";
        if(p->variables[i]->name == desiredVar){
            return p->variables[i];
	    }
	}
    return NULL;
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            if(1){ //_processes[i]->variables[j]->name != "<FREE_SPACE>"
                uint32_t pid = _processes[i]->pid;
                std::string varname = _processes[i]->variables[j]->name;
                uint32_t addr = _processes[i]->variables[j]->virtual_address;
                uint32_t size = _processes[i]->variables[j]->size;

                std::cout << std::setw(5) << pid << " | " << std::setw(13) << varname << " | " << std::setw(12) << addr << " | " << size << '\n';
            }
        }
    }
}

void Mmu:: killProcess(uint32_t pid){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            _processes.erase(_processes.begin() + i);  
		}
	}
}

void Mmu::checkAndMerge(uint32_t pid, Variable *var, uint32_t page_size){ //for a process, check for ANY adjacent FreeSpaces within the variable's page and merge.
    int currPage = 0;
    int byteCounter = 0;
    
    Process *inq;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            inq = _processes[i];
        }
    }

    int toCheck = -1;

    for(int i = 0; i < inq->variables.size(); i++){
        if(inq->variables[i] == var){
              toCheck = i;
		}
	}

    if(toCheck == -1){
        std::cout << "error: variable not found. did you pass an incorrect argument to checkAndMerge?" << '\n';
	} else {
        int pgstart = trunc(inq->variables[toCheck]->virtual_address / page_size) * page_size; //start of page in VM is virtual addr / page size truncated and then remultiplied by pgsize
        int pgend = pgstart + page_size; //end of this page is just page_size bytes away from pgstart

        if(inq->variables[toCheck - 1]->name == "<FREE_SPACE>" && inq->variables[toCheck + 1]->name == "<FREE_SPACE>"){ //Merge L&R

            //Check page matches RIGHT to LEFT to avoid unusual FS distributions
            if(inq->variables[toCheck + 1]->virtual_address + inq->variables[toCheck + 1]->size <= pgend){ //is the right freespace on the same page?
                inq->variables[toCheck]->size += inq->variables[toCheck + 1]->size;
                inq->variables.erase(inq->variables.begin() + (toCheck + 1));
            }
            
            if(inq->variables[toCheck - 1]->virtual_address >= pgstart){ //is the left freespace on the same page?
                inq->variables[toCheck - 1]->size += inq->variables[toCheck]->size;
                inq->variables.erase(inq->variables.begin() + (toCheck));
			}
            
		} else if (inq->variables[toCheck - 1]->name == "<FREE_SPACE>" && inq->variables[toCheck + 1]->name != "<FREE_SPACE>" && inq->variables[toCheck - 1]->virtual_address >= pgstart){ //Merge L
            inq->variables[toCheck - 1]->size += inq->variables[toCheck]->size;
            inq->variables.erase(inq->variables.begin() + (toCheck));
        } else if(inq->variables[toCheck - 1]->name != "<FREE_SPACE>" && inq->variables[toCheck + 1]->name == "<FREE_SPACE>" && inq->variables[toCheck + 1]->virtual_address + inq->variables[toCheck + 1]->size <= pgend){ //Merge R
            inq->variables[toCheck]->size += inq->variables[toCheck + 1]->size;
            inq->variables.erase(inq->variables.begin() + (toCheck + 1));
		}
	}

}

void Mmu::printProcesses(){
    for(int i = 0; i < _processes.size(); i++){
        std::cout << _processes[i]->pid << '\n';        
	}
}