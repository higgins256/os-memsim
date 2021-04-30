#include <iostream>
#include <string>
#include <cstring>
#include <math.h>
#include "mmu.h"
#include "pagetable.h"

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table, int page_size);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, int page_size);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, int page_size);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table, int page_size);
int getTypeByteSize(DataType type);
void checkAndFreePage(uint32_t pid, Mmu *mmu, PageTable *page_table, int page_size);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint32_t mem_size = 67108864;
    void *memory = malloc(mem_size); // 64 MB (64 * 1024 * 1024)

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline (std::cin, command);
    while (command != "exit") {
        // Handle command

        std::vector<std::string> cmdcontainer;
        for(int i = 0; i < command.length(); i++){
            std::string hold;
            if(command[i] !=  ' '){
                hold += command[i];
			} else {
                cmdcontainer.push_back(hold);
                hold.clear();
			}
		}

        if(cmdcontainer[0] == "create"){
            //Initialize new process, print PID
            createProcess(stoi(cmdcontainer[1]), stoi(cmdcontainer[2]), mmu, page_table, page_size);
		}

        //FreeSpace, Char, Short, Int, Float, Long, Double

        else if(cmdcontainer[0] == "allocate"){
            DataType type;

            if(cmdcontainer[3] == "freespace"){
                type = FreeSpace;
			} else if(cmdcontainer[3] == "char") {
                type = Char;
			} else if(cmdcontainer[3] == "short") {
                type = Short;
            } else if(cmdcontainer[3] == "int") {
                type = Int;
            } else if(cmdcontainer[3] == "float") {
                type = Float;
            } else if(cmdcontainer[3] == "long") {
                type = Long;
            } else if(cmdcontainer[3] == "double") {
                type = Double;
            } else {
                std::cout << "error: illegal type" << '\n';     
			}

            allocateVariable(stoi(cmdcontainer[1]), cmdcontainer[2], type, stoi(cmdcontainer[4]), mmu, page_table, page_size);
		}

        //"  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)"
        //void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
        else if(cmdcontainer[0] == "set"){
            uint32_t num_to_set = cmdcontainer.size() - 4; //how many variables do we need to set?
            uint32_t offset_inc; //size of type offset for each parameter
            uint32_t offset = stoi(cmdcontainer[3]); //starting offset

            Variable *inq = mmu->getVariableAt(stoi(cmdcontainer[1]), cmdcontainer[3]); //get var to check variable type.
            if(inq == NULL){
                break; //getVariableAt returns null if it threw an error. 
			}			

            void *val;

            if(inq->type == FreeSpace){ //set offset incrementer based on datatype.
                offset_inc = 1;
			} else if(inq->type == Char) {
                offset_inc = 1;
            } else if(inq->type == Short) {
                offset_inc = 2;
            } else if(inq->type == Int) {
                offset_inc = 4;
            } else if(inq->type == Float) {
                offset_inc = 4;
            } else if(inq->type == Long) {
                offset_inc = 8;
            } else if(inq->type == Double) {
                offset_inc = 8;
            } else {
                std::cout << "Something went really wrong - this variable has no type." << '\n';     
			}

            for(int i = 0; i < num_to_set; i++){
                std::string input = cmdcontainer[i + 3];
                setVariable(stoi(cmdcontainer[1]), cmdcontainer[2], offset, &input, mmu, page_table, memory);
                offset += offset_inc; //run setVariable each time offsetting by datatype's size.
			}
		}

        else if(cmdcontainer[0] == "print"){
            if(cmdcontainer[1] == "mmu"){
                mmu->print();
			} else if (cmdcontainer[1] == "page"){
                page_table->print();    
			} else if (cmdcontainer[1] == "process"){
                mmu->printProcesses();
			} else {
                //split argument by colon     
                size_t found = cmdcontainer[1].find(":");
                int pid = stoi(cmdcontainer[1].substr(0, found - 1));
                std::string varname = cmdcontainer[1].substr(found + 1, cmdcontainer[1].length() - found + 1);
                //print the value of the variable indicated by the request
                Variable *inq = mmu->getVariableAt(pid, varname);
                int offset_inc = getTypeByteSize(inq->type);

                int numvars = inq->size / offset_inc; //how many values are in this variable?

                for(int i = 0; i < 4; i++){
                    int phys_addr = page_table->getPhysicalAddress(pid, inq->virtual_address + i); //i acts as virtual offset for subvars

                    if(inq->type == Char) {
                        std::cout << ((char*)memory)[phys_addr];
                    } else if(inq->type == Short) {
                        std::cout << ((short*)memory)[phys_addr];
                    } else if(inq->type == Int) {
                        std::cout << ((int*)memory)[phys_addr];
                    } else if(inq->type == Float) {
                        std::cout << ((float*)memory)[phys_addr];
                    } else if(inq->type == Long) {
                        std::cout << ((long*)memory)[phys_addr];
                    } else if(inq->type == Double) {
                        std::cout << ((double*)memory)[phys_addr];
                    }

                    if(i < 3 || i < numvars - 1){
                        std::cout << ", ";
                    }

				} //POTENTIAL ISSUE - incrementer doesn't do type-oriened bit counting - it just shifts virt addr by 1 each run.

                if(numvars > 4){
                    std::cout  << "...[" << numvars << " items]" << '\n';      
				} else {
                    std::cout << '\n';        
				}
			}
		}

        else if(cmdcontainer[0] == "free"){
            freeVariable(stoi(cmdcontainer[1]), cmdcontainer[2], mmu, page_table, page_size);
		}

        else if(cmdcontainer[0] == "terminate"){
            terminateProcess(stoi(cmdcontainer[1]), mmu, page_table, page_size);
		}

        else if(cmdcontainer[0] == "exit"){
            break;  
		}

        else {
            std::cout << "error: command not recognized";  
		}

        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }

    // Clean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table, int page_size)
{
    // TODO: implement this!
    //   - create new process in the MMU
    uint32_t PID = mmu->createProcess();

    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    allocateVariable(PID, "<TEXT>", DataType::Char, text_size, mmu, page_table, page_size);
    allocateVariable(PID, "<GLOBALS>", DataType::Char, data_size, mmu, page_table, page_size);
    allocateVariable(PID, "<STACK>", DataType::Char, 65536, mmu, page_table, page_size);

    //   - print pid
    std::cout << PID << '\n';
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, int page_size)
{
    // TODO: implement this!
    if(mmu->isProcessInMMU(pid) == 0){
        std::cout << "error: process not found" << '\n';
        return;
	}

    uint32_t req_size = getTypeByteSize(type) * num_elements;
    Process *p = mmu->getProcessAt(pid);
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    Variable *target;
    for(int i = 0; i < p->variables.size(); i++){
        if(p->variables[i]->type == FreeSpace && p->variables[i]->size > req_size){
            target = p->variables[i]; 
            break;
		} //get first hole large enough for allocation
	}

    uint32_t prev_addr; //this is the address we will be allocating the variable to.

    //   - if no hole is large enough, allocate new page(s)
    if(target == NULL){
        //int pagecomp = trunc(req_size / page_size); //is our variable too big for a single page?
        int pagenum = trunc((p->variables[p->variables.size() - 1]->virtual_address / page_size)+1); //get page number of last element in variables and then add 1.
        prev_addr = pagenum * page_size;
        mmu->addVariableToProcess(pid, "<FREE_SPACE>", FreeSpace, page_size, prev_addr); //BREAKPOINT - virtual address for new 'page' is the page # * pagesize.
        page_table->addEntry(pid, pagenum);
        target = p->variables[p->variables.size() - 1]; //new page starts at the latest index
	} else { //   - insert variable into MMU
        prev_addr = target->virtual_address;
	}

    target->virtual_address += req_size;
    target->size -= req_size;
    mmu->addVariableToProcess(pid, var_name, type, req_size, prev_addr);

    //   - print virtual memory address
    std::cout << prev_addr << '\n';

}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    if(mmu->isProcessInMMU(pid) == 0){
        std::cout << "error: process not found" << '\n';
        return;
	}
    //   - look up physical address for variable based on its virtual address / offset
    Variable *target = mmu->getVariableAt(pid, var_name);
    uint32_t phys_addr = page_table->getPhysicalAddress(pid, target->virtual_address + offset); //BREAKPOINT: might be misusing offset
    DataType dt = target->type;

    //   - insert `value` into `memory` at physical address
    if(dt == Char) {
        value = (char*)value;
        memcpy((char*)memory + phys_addr, value, getTypeByteSize(dt)); //copy one instance of (type-size) bytes
	} else if(dt == Short) {
        value = (short*)value;
        memcpy((short*)memory + phys_addr, value, getTypeByteSize(dt));
    } else if(dt == Int) {
        value = (int*)value;
        memcpy((int*)memory + phys_addr, value, getTypeByteSize(dt));
    } else if(dt == Float) {
        value = (float*)value;
        memcpy((float*)memory + phys_addr, value, getTypeByteSize(dt));
    } else if(dt == Long) {
        value = (long*)value;
        memcpy((long*)memory + phys_addr, value, getTypeByteSize(dt));
    } else if(dt == Double) {
        value = (double*)value;
        memcpy((double*)memory + phys_addr, value, getTypeByteSize(dt));
    } else {
        std::cout << "error: invalid type" << '\n';
	}

    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, int page_size)
{
    // TODO: implement this!
    if(mmu->isProcessInMMU(pid) == 0){
        std::cout << "error: process not found" << '\n';
        return;
	}
    
    //   - remove entry from MMU
    Variable *toRemove = mmu->getVariableAt(pid, var_name);
    toRemove->name = "<FREE_SPACE>";
    toRemove->type = FreeSpace;

    //if newFree has free space neighbors, merge them.
    mmu->checkAndMerge(pid, toRemove, page_size);

    //   - free page if this variable was the only one on a given page
    checkAndFreePage(pid, mmu, page_table, page_size); 
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table, int page_size)
{
    if(mmu->isProcessInMMU(pid) == 0){
        std::cout << "error: process not found" << '\n';
        return;
	}

    Process *toKill = mmu->getProcessAt(pid);
    for(int i = 0; i < toKill->variables.size(); i++){
        freeVariable(pid, toKill->variables[i]->name, mmu, page_table, page_size); //when this finishes all pages will be freed as a consequence.
	}

    mmu->killProcess(pid);
}

int getTypeByteSize(DataType type){
    if(type == FreeSpace){ //set offset incrementer based on datatype.
            return 1;
	} else if(type == Char) {
            return 1;
    } else if(type == Short) {
            return 2;
    } else if(type == Int) {
            return 4;
    } else if(type == Float) {
            return 4;
    } else if(type == Long) {
            return 8;
    } else if(type == Double) {
            return 8;
    } else {
            std::cout << "error: incorrect type passed to getTypeByteSize" << '\n';
            return -1;
    }
}

void checkAndFreePage(uint32_t pid, Mmu *mmu, PageTable *page_table, int page_size){
    int currPage = 0;
    std::vector<bool> toBeFreed;
    Process *toFree = mmu->getProcessAt(pid);

    int counter = 0;
    int flag = 0;
    int i = 0;
    while(i < toFree->variables.size()){
        while(counter < page_size){
            if(toFree->variables[i]->name != "<FREE_SPACE>"){
               flag = 1;  
		    }
            counter += toFree->variables[i]->size;
            i++;
        }

        if(flag == 0){ 
            toBeFreed.push_back(true);
		} else {
            toBeFreed.push_back(false);  
		}

        flag = 0;
        counter = 0;
    }

    for(int i = 0; i < toBeFreed.size(); i++){
        if(toBeFreed[i]){
            page_table->removePageEntry(pid, i);  //BREAKPOINT: For this to work, i MUST correspond to page_number. 
		}
	}
}