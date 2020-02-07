#include "BuddyAllocator.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
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

  cout << endl << endl << endl << "/////////////////////Start of Constructor////////////////////" << endl;

  //Setting up basic_block_size to be a power of 2
  if(log2(basic_block_size) > (int)log2(basic_block_size)){
    // cout << "Given basic_block_size was not a power of 2: " << basic_block_size << endl;
    basic_block_size = (int)pow(2,(int)(log2(basic_block_size) + 1));
    // cout << "Will instead allocate nearest power: " << basic_block_size << endl;
  }

  //Setting up total memory size to be a power of 2
  if(log2(total_memory_size) > (int)log2(total_memory_size)){
    // cout << "Given total_memory_size was not a power of 2: " << total_memory_size << endl;
    total_memory_size = (int)pow(2,(int)(log2(total_memory_size) + 1));
    // cout << "Will instead allocate nearest power: " << total_memory_size << endl;
  }

  //Sets up the offeset to be used for caluclating Freelist Indexes
  basic_block_base2 = log2(basic_block_size);

  // cout << "Levels needed " << log2(total_memory_size/basic_block_size) + 1 << endl;

  //setup freelist with the amount of levels needed
  max_level_index = log2(total_memory_size/basic_block_size);
  setupFreeList(max_level_index + 1);
 
  start_loc = (char*)malloc(total_memory_size);
  BlockHeader startBlock;
  startBlock.block_size = total_memory_size;
  startBlock.buddy_Loc = 0;
  memcpy( start_loc, &startBlock, sizeof(startBlock) );
  BlockHeader * startBlock_ptr = (BlockHeader*)start_loc;
  // cout << "Size of Block Header " << sizeof(startBlock) << endl;
  FreeList.at(FreeList.size()-1).head = startBlock_ptr;

  // cout << "Succesfully placed startBlock in FreeList -> total_memory_size: " << FreeList.at(FreeList.size()-1).head->block_size << endl;

  cout << "/////////////////////End of Constructor////////////////////" << endl;
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

  cout << endl << endl << endl << endl;
  cout << "///////////////////////Start of Split///////////////////////////" << endl;
  BlockHeader * buddyBlock = (BlockHeader*)((char*)block + (block->block_size/2));

  //Should be doing this removal inside alloc
  // FreeList.at(log2(block->block_size) - basic_block_base2).remove(block);

  cout << "BuddyBlock Address should be: " << block << " + " << (block->block_size/2) <<endl;
  cout << "BuddyBlock Address is: " << buddyBlock << endl; 

  cout << "Original Block size is: " << block->block_size << endl;

  //Setting new block_size
  block->block_size = block->block_size/2;
  buddyBlock->block_size = block->block_size;

  cout << "New Block size is: " << block->block_size<< endl;

  //Setting new buddy_Loc
  block->buddy_Loc = 0;
  buddyBlock->buddy_Loc = 1;


  // //Adding blocks to appropriate LinkedList in FreeList
  short indexToPlaceBlock = (short)(log2(block->block_size) - basic_block_base2);
  
  FreeList.at(indexToPlaceBlock).insert(block);
  FreeList.at(indexToPlaceBlock).insert(buddyBlock);

  cout << "///////////////////////End of Split///////////////////////////" << endl;

  return NULL;
};

void BuddyAllocator::setupFreeList(int levelsNeeded){

  for(int i =0; i < levelsNeeded; i++){
    FreeList.push_back(LinkedList());
  }

  cout << "Current FreeList size " << FreeList.size() << endl;

};

//BuddyAllocator public functions
void* BuddyAllocator::alloc(int length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */

  cout << "///////////////////////Start of Alloc///////////////////////////" << endl;

  printlist();

  //Checking function input
  if(length < 1){
    cout << "Cannot allocate less than 1 byte! returned a Null pointer" << endl;
    return NULL;
  } else if ( length > total_memory_size ){
    cout << "Not enough Memory Null Pointer returned instead" << endl;
  }

  int bytesRequested = length + sizeof(BlockHeader);
  int indexOfBlockSizeRequested = ceil( log2(bytesRequested) ) - basic_block_base2;

  if( indexOfBlockSizeRequested < 0 ){
    indexOfBlockSizeRequested = 0;
  }

  cout << "indexOfBlockSizeRequested: " << indexOfBlockSizeRequested << endl;

  int indexOfNearestFreeBlock = indexOfBlockSizeRequested;
  
  //Goes through Freelist until it finds the next available block that can fit
  //the requested size. Returns NULL if none available.
  while( FreeList.at(indexOfNearestFreeBlock).head == NULL ){
    if(indexOfNearestFreeBlock == max_level_index){
      cout << "Not enough Memory Null pointer returned instead";
      return NULL;
    } else {
      indexOfNearestFreeBlock += 1;
    }
  };

  cout << "IndexOfNearestFreeBlock: " << indexOfNearestFreeBlock << endl;

  while( indexOfNearestFreeBlock != indexOfBlockSizeRequested ){
    BlockHeader* blockToSplit = FreeList.at(indexOfNearestFreeBlock).head;

    //Need to remove the Block from it's FreeList first
    FreeList.at(indexOfNearestFreeBlock).remove(blockToSplit);

    split(blockToSplit);
    indexOfNearestFreeBlock -= 1;
  }

  printlist();

  cout << "IndexOfNearestFreeBlock: " << indexOfNearestFreeBlock << endl;


  //Now that you have a FreeBlock of the right size you need to remove it from the FreeList 
  //and pass the pointer to the User. We add 16 to the pointer in order to account for overhead.
  //FIXME: Remember you have to also subtract by this number in order to properly free the memory
  BlockHeader* freeBlock = FreeList.at(indexOfNearestFreeBlock).head;

  cout << "freeBlock Address: " << freeBlock << endl;

  FreeList.at(indexOfNearestFreeBlock).remove(freeBlock);

  printlist();

  cout << "///////////////////////End of Alloc///////////////////////////" << endl;

  return ( (void*)( (char*)freeBlock + 16 ) ) ;

  return NULL;
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
    cout << "Amount of available free memory: " << total_free_memory << " bytes" << endl;  
  }
};

