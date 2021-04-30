#include <iomanip>
#include "pagetable.h"

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int frame = -1; 
    int num_frames = 67108864 / _page_size; //not sure how to find this val without hardcoding 

    // Find free frame
    //first, iterate thru pagetable and mark all used frames.
    std::vector<std::string> keys = sortedKeys();
    bool marked[num_frames];
    for(int i = 0; i < num_frames; i++){
        marked[i] = false;
	}

    int min = num_frames + 1;
    int cur = num_frames + 1;


    for(int i = 0; i < keys.size(); i++){
        marked[_table[keys[i]]] = true; //mark all frames currently associated with a page
	}

    int i = 0;
    while(frame == -1){
        if(marked[i] = false){
            frame = i; //grab earliest possible unmarked frame  
		}
	}

    _table[entry] = frame;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    // TODO: implement this!
    int page_number = 0;
    int page_offset = 0;

    page_number = virtual_address / _page_size;
    page_offset  = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        address = _table[entry] * _page_size + page_offset;
    }

    return address;
}

void PageTable::removePageEntry(uint32_t pid, int page_num){
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_num);
    if (_table.count(entry) > 0)
    {
        _table.erase(entry);
    }
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        // TODO: print all pages
        size_t found = keys[i].find("|");
        if(found == std::string::npos){
             std::cout << "error in key formatting. " << '\n';
             break;
		}

        std::string pid = keys[i].substr(0, found - 1);
        std::string pagenum = keys[i].substr(found + 1, keys[i].size() - (found + 1));
        int framenum = _table[keys[i]];

        std::cout << std::setw(5) << pid << " | " << std::setw(11) << pagenum << " | " << std::setw(12) << framenum << '\n';
    }
}
