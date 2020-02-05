#include "Ackerman.h"
#include "BuddyAllocator.h"
#include "unistd.h"

using namespace std;

void easytest(BuddyAllocator* ba){
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
  opterr = 0;


  //getopt usage to set basic_block_size, and memory_length
  while( (option = getopt(argc, argv, "b:s:")) != -1 ){
    switch(option){
      case 'b':
        cout << "Added b as an option" << endl;
        cout << "The argument passed was: " << optarg << endl;
        basic_block_size = atoi(optarg);
        cout << "The basic block size has been set to: " << basic_block_size << " Bytes" << endl; 
        break;
      case 's':
        cout << "added s as an argument" << endl;
        cout << "The argument passed was: " << optarg << endl;
        memory_length = atoi(optarg) * 1024;
        cout << "The memory length has been set to " << memory_length << " KiloBytes" << endl;
        break;
      case '?':
          if( optopt == 'b' ){
            cout << "Error! no basic block size entered after -b option flag!" << endl;
          } else if( optopt == 's' ){
            cout << "Error! no memory size was entered after -s option flag!" << endl;
          } else if( isprint(optopt) ){
            cout << " Unknown option: " << (char)optopt << endl; 
          } else {
            cout << "Unknown option character: " << (char)optopt << endl;
          }
          return 0;
      default:
        return 0;
    }
  }

  // create memory manager
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);
  delete(allocator);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  // easytest (allocator);

  
  // stress-test the memory manager, do this only after you are done with small test cases
  // Ackerman* am = new Ackerman ();
  // am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  // delete allocator;
}


