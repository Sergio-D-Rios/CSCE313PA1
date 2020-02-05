#include "BuddyAllocator.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace std;


////////////////////////////////////////////////////////////////////////////////////////////////////////
//LINKEDLIST IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////

//LinkedList public functions
void LinkedList::insert(BlockHeader* b){
  cout << "Current Head is " << this->head << endl;
  if(this->head == NULL){
    this->head = b;
    b->next = NULL;
  } else {
    b->next = this->head;
    this->head = b;
  }
};

void LinkedList::remove(BlockHeader* b){
  BlockHeader* temp = this->head;
  if(b == NULL){
    cout << "Removal not possible" << endl;
  } else if(b == this->head){
    if(this->head->next == NULL){
      this->head = NULL;
      temp->next = NULL;
      cout << "Removing the head, list is now empty." << endl;
    } else {
      this->head = this->head->next;
      temp->next = NULL;
      cout << "Removing the head, new head designated for non-empty list." << endl;
    }
  } else {
    BlockHeader* temp = this->head;
    while(temp->next != b ){
      temp = temp->next;
    }
    temp->next = b->next;
    b->next = NULL;
    cout << "Removing an interior element for non-empty list." << endl;
  }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//BUDDYALLOCATOR IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////

//BuddyAllocator Constructor and Destuctor
BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size, total_memory_size = _total_memory_length;
  start_loc = (char*)malloc(total_memory_size);
  BlockHeader startBlock;
  startBlock.block_size = total_memory_size;
  startBlock.buddy_Loc = 0;
  BlockHeader * startBlock_ptr = &startBlock;
  memcpy( start_loc, startBlock_ptr, sizeof(startBlock) );
  startBlock_ptr = (BlockHeader*)start_loc;
  
  //FIXME: Need to create Freelist then add the starting block to the correct index in freelist


};

BuddyAllocator::~BuddyAllocator (){
  cout << "Called Destructor" << endl;
	free(start_loc);
}

//BuddyAllocator private functions
BlockHeader* BuddyAllocator::getbuddy(BlockHeader * addr){

  return NULL;
};

bool BuddyAllocator::arebuddies(BlockHeader* block1, BlockHeader* block2){

  return true;
};

BlockHeader* BuddyAllocator::merge(BlockHeader* block1, BlockHeader* block2){

  return NULL;
};

BlockHeader* BuddyAllocator::split(BlockHeader* block){

  return NULL;
};

//BuddyAllocator public functions
void* BuddyAllocator::alloc(int length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  return malloc (length);
};

void BuddyAllocator::free(void* a) {
  /* Same here! */
  ::free (a);
};

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  int64_t total_free_memory = 0;
  for (int i=0; i<FreeList.size(); i++){
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
      if (b->block_size != blocksize){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      b = b->next;
    }
    cout << count << endl;
    cout << "Amount of available free memory: " << total_free_memory << " byes" << endl;  
  }
};

