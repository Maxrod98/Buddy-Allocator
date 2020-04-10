#include "BuddyAllocator.h"
#include <iostream>
#include <math.h>

using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size, total_memory_size = _total_memory_length;
	int size_of_block_vector = log2(total_memory_size) - log2(basic_block_size) + 1;
	
	memory = new char[_total_memory_length];

	//step 1
	for (int i = 0; i < size_of_block_vector; i++)
	{
		LinkedList temp;
		temp.head = nullptr;
		FreeList.push_back(temp);
	}
	
  BlockHeader* first = (BlockHeader*) memory;
  start = (char *)first;
  first->next = nullptr;
  first->block_size_power = log2(_total_memory_length);
  first->is_free = true;
  FreeList.at(size_of_block_vector - 1).insert(first);
}

BuddyAllocator::~BuddyAllocator() {
    delete[] memory;
}

//helper function
BlockHeader* BuddyAllocator::popHeader(int index)
{
  if (index >= FreeList.size() or index < 0) return nullptr;

  BlockHeader* temp = FreeList.at(index).head;
  FreeList.at(index).remove(temp);
  temp->is_free = false;
  return temp;
}

void* BuddyAllocator::alloc(int length) {
  //memory to be allocated is too small or beyond the limits
  if (length <= 0 or length > total_memory_size)
    return nullptr;

  int index = (1 + log2(length + sizeof(BlockHeader)) - log2(basic_block_size));
  if (index <= 0) index = 0;

  if (index >= FreeList.size()) return nullptr;
  
  //a block of the required size is available
  if (FreeList.at(index).head != nullptr)
  {
    void * ans = (void *) popHeader(index) + sizeof(BlockHeader);
    return ans;
  }

  //we need to split a block until we find the desired size
  else
  {
    //cursor i will move up the vector of LinkedLists until it finds a Block that it can split up
    int i = 0;
    
    for (i = (index + 1) ; i < FreeList.size(); i++)
    {
      if ( i > FreeList.size() ) break;
      if (FreeList.at(i).head != nullptr) break;
    }
    
    if (i >= FreeList.size()) i = FreeList.size() - 1;
    //when FreeList.at(i) == nullptr it means that no block was available

    if (FreeList.at(i).head != nullptr and i < FreeList.size())
    {
      //split until needed
      for (int o = i; o > index; o--)
        split(FreeList.at(o).head);

      void * ans = (void *) popHeader(index) + sizeof(BlockHeader);
      return ans;
    }
    //if no memory was present, then it will return nullptr
    cout << "NOT ENOUGH MEMORY: " << length << endl;
    //printlist();
      return nullptr;
  }
}

void BuddyAllocator::free(void* a) {
  char * ptr = (char *) a - sizeof(BlockHeader);
  BlockHeader* freed_block =(BlockHeader* ) ptr;

  //check if block is out of bounds
  if ((char*) freed_block > start + total_memory_size) return;

  //set block free and attach to LL
  int index = freed_block->block_size_power - log2(basic_block_size);
  FreeList.at(index).insert(freed_block);
  freed_block->is_free = true;
  
  mergeIteratively(index);
}

void BuddyAllocator::mergeIteratively(int index)
{
//loop to check when to merge
  for (int i = index; i < FreeList.size() - 1; i++)
  {
    BlockHeader * cur = FreeList.at(i).head;
    
    while (cur != nullptr)
    {
      //check if both buddies are available, if they are merge them
      if (arebuddies(getbuddy(cur), cur))
      {
        BlockHeader* leftBuddy = min(getbuddy(cur), cur);
        BlockHeader* rightBuddy = max(getbuddy(cur), cur);

        FreeList.at(i).remove(rightBuddy);
        FreeList.at(i).remove(leftBuddy);
        FreeList.at(i + 1).insert(leftBuddy);
        leftBuddy->block_size_power++;
        break;
      }
      cur = cur->next;
    }
  }
}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  int64_t total_free_memory = 0;
  for (int i=0; i < FreeList.size(); i++){
    int blocksize = ((1<<i) * basic_block_size); // all blocks at this level are this size
    cout << "[" << i <<"] (" << blocksize << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList [i].head;
    // go through the list from head to tail and count
    while (b){
      total_free_memory += blocksize;
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      
      if (b->block_size_power != log2(blocksize)){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      
      b = b->next;
    }
    cout << count << endl;
    cout << "Amount of available free memory: " << total_free_memory << " byes" << endl;  
  }
}
	/* private function you are required to implement
	 this will allow you and us to do unit test */

	BlockHeader* BuddyAllocator::getbuddy (BlockHeader * addr)
  {
    char *ptr = (char*) addr;
    ptr = ptr - (uintptr_t)start;
    ptr = (char*)((uintptr_t)ptr ^ (uintptr_t)exp2(addr->block_size_power));
    ptr += (uintptr_t) start;

    return (BlockHeader*) ptr;
  }
	// given a block address, this function returns the address of its buddy 
	
	bool BuddyAllocator::arebuddies (BlockHeader* block1, BlockHeader* block2)
  {
    BlockHeader* leftBlock = min(block1, block2);
    BlockHeader* rightBlock = max(block1, block2);

    //get the next block and check if it matches
    BlockHeader* buddy = getbuddy(leftBlock);

    return ((buddy == rightBlock) and block1->is_free and block2->is_free and block1->block_size_power == block2->block_size_power);
  }
	// checks whether the two blocks are buddies are not
	// note that two adjacent blocks are not buddies when they are different sizes

	BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2)
  {
    //need to add condition to check which is to the left or right
    BlockHeader* leftBlock = min(block1, block2);
    BlockHeader* rightBlock = max(block1, block2);

    //Update FreeList
    int index = leftBlock->block_size_power - log2(basic_block_size);
    if (index >= FreeList.size()) index = FreeList.size() - 2;
    FreeList.at(index).remove(leftBlock);
    FreeList.at(index).remove(rightBlock);
    FreeList.at(index + 1).insert(leftBlock);
    
    //Updating the leftBlock
    leftBlock->block_size_power += 1;
    leftBlock->next = rightBlock->next;

    return leftBlock;
  }
	// this function merges the two blocks returns the beginning address of the merged block
	// note that either block1 can be to the left of block2, or the other way around

	BlockHeader* BuddyAllocator::split (BlockHeader* block)
  {
    //this method will split only the first block of the FreeList
    //Updating the array of LinkedList
    int index = block->block_size_power - log2(basic_block_size);
    if (index >= FreeList.size()) index = FreeList.size() - 2;
    FreeList.at(index).remove(block);
    FreeList.at(index - 1).insert(block);
    block->is_free = true;

    //bytewise manipulation
    char* ptr = (char*) block;
    ptr += (uintptr_t)exp2(block->block_size_power - 1);

    //using the previously allocated data to write on it the new header
    BlockHeader* newblock = (BlockHeader*) ptr;
    newblock->block_size_power = block->block_size_power - 1;
    newblock->next = block->next;
    newblock->is_free = true;
    
    //modifying the older's blocks values
    block->next = newblock;
    block->block_size_power--;
  }
	// splits the given block by putting a new header halfway through the block
	// also, the original header needs to be corrected