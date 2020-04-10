#include <getopt.h>
#include <iostream>
#include "Ackerman.h"
#include "BuddyAllocator.h"


void easytest(BuddyAllocator* ba){
  
  //DEBUGGING CODE
  cout << "alloc 100000" << endl;
  int* m3 = (int*) ba->alloc(100000);
  ba->printlist();
  
  cout << "alloc 150" << endl;
  int* m1 = (int*) ba->alloc(150);
  ba->printlist();

  cout << "alloc 4000" << endl;
  int* m2 = (int*) ba->alloc(4000);
  ba->printlist();


  cout << "FREE 4000" << endl;
  ba->free(m2);
  ba->printlist();

  cout << "FREE 100000" << endl;
  ba->free(m3);
  ba->printlist();

  cout << "free 150" << endl;
  ba->free(m1);
  ba->printlist();

   // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte
  char * mem = (char *) ba->alloc (1);
  // now print again, how should the list look now
  ba->printlist ();

  ba->free (mem); // give back the memory you just allocated
  ba->printlist(); // shouldn't the list now look like as in the beginning
  
}

int main(int argc, char ** argv) {
  int basic_block_size = 128, memory_length = 512 * 1024;
  

  int option;
  while ((option = (getopt(argc, argv, "b:s:"))) != -1)
  {
    //cout << option << endl;
      switch (option)
      {
      case 'b':
        basic_block_size = exp2(static_cast<int>(log2(atoi(optarg) - 1) + 1) );
        break;
      case 's':
        memory_length = exp2(static_cast<int>(log2(atoi(optarg) - 1) + 1) );
        break;
      default:
        cout <<"optarg:" << optarg << endl;
        cout << "NOT FOUND " << option << endl;
      }
  }
  // create memory manager
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);


  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest (allocator);
  
  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator;

  delete am;
}