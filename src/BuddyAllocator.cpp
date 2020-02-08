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
  } else if(b == this->head){
    if(this->head->next == NULL){
      this->head = NULL;
      temp->next = NULL;
    } else {
      this->head = this->head->next;
      temp->next = NULL;
    }
  } else {
    BlockHeader* temp = this->head;
    while(temp->next != b ){
      temp = temp->next;
    }
    temp->next = b->next;
    b->next = NULL;
  }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//BUDDYALLOCATOR IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////

//BuddyAllocator Constructor and Destuctor
BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size, total_memory_size = _total_memory_length;

  //Setting up basic_block_size to be a power of 2
  if(log2(basic_block_size) > (int)log2(basic_block_size)){
    basic_block_size = (int)pow(2,(int)(log2(basic_block_size) + 1));
  }

  //Setting up total memory size to be a power of 2
  if(log2(total_memory_size) > (int)log2(total_memory_size)){
    total_memory_size = (int)pow(2,(int)(log2(total_memory_size) + 1));
  }

  //Sets up the offeset to be used for caluclating Freelist Indexes
  basic_block_base2 = log2(basic_block_size);

  //setup freelist with the amount of levels needed
  max_level_index = log2(total_memory_size/basic_block_size);
  setupFreeList(max_level_index + 1);
 
  start_loc = (char*)malloc(total_memory_size);
  BlockHeader startBlock;
  startBlock.block_size = total_memory_size;
  startBlock.buddy_Loc = '0';
  memcpy( start_loc, &startBlock, sizeof(startBlock) );
  BlockHeader * startBlock_ptr = (BlockHeader*)start_loc;
  FreeList.at(FreeList.size()-1).head = startBlock_ptr;
};

BuddyAllocator::~BuddyAllocator (){
	std::free(start_loc);
}

//BuddyAllocator private functions
BlockHeader* BuddyAllocator::getbuddy(BlockHeader * addr){

  //Proper getbuddy algorithm FIXME: Look at this for possible cause of issues later on
  BlockHeader* buddy = (BlockHeader*)( ( (char*)addr - start_loc ) ^ ((unsigned long)(addr->block_size + start_loc)) );
  return buddy;
};

bool BuddyAllocator::arebuddies(BlockHeader* block1, BlockHeader* block2){

  return block1->block_size == block2->block_size;
      
};

//Performs the merger of two blocks. will return the starting pointer to the block header of the left most block after merge.
//This function assumes that both block are indeed buddies, both are free, and that block1 is the left block
BlockHeader* BuddyAllocator::merge(BlockHeader* block1, BlockHeader* block2){

  short indexToRemoveBlock = (short)(log2(block1->block_size) - basic_block_base2);

  //Remove both blocks from their coresponding FreeList
  FreeList.at(indexToRemoveBlock).remove(block1);
  FreeList.at(indexToRemoveBlock).remove(block2);

  //Adjust block1 header
  block1->block_size = block1->block_size*2;

  short indexToInsertBlock = (short)(log2(block1->block_size) - basic_block_base2);

  //insert the merged block into the correct list
  FreeList.at(indexToInsertBlock).insert(block1);

  return block1;
};

BlockHeader* BuddyAllocator::split(BlockHeader* block){

  BlockHeader buddyBlock;

  //Placing new BlockHeader* in correct location
  char* dest = (char*)block + block->block_size/2;
  memcpy( dest,  block, sizeof(buddyBlock));
  BlockHeader* buddyBlock_ptr = (BlockHeader*)dest;

  //Setting new block_size
  block->block_size = block->block_size/2;
  buddyBlock_ptr->block_size = block->block_size;

  //Setting new buddy_Loc
  block->buddy_Loc = '0';
  buddyBlock_ptr->buddy_Loc = '1';

  //Setting in_use for buddy block
  buddyBlock_ptr->in_use = '0';


  // //Adding blocks to appropriate LinkedList in FreeList
  short indexToPlaceBlock = (short)(log2(block->block_size) - basic_block_base2);
  
  FreeList.at(indexToPlaceBlock).insert(buddyBlock_ptr);
  FreeList.at(indexToPlaceBlock).insert(block);

  return NULL;
};

void BuddyAllocator::setupFreeList(int levelsNeeded){

  for(int i =0; i < levelsNeeded; i++){
    FreeList.push_back(LinkedList());
  }

};

//BuddyAllocator public functions
void* BuddyAllocator::alloc(int length) {

  //Checking function input
  if(length < 1){
    cout << "Cannot allocate less than 1 byte! returned a Null pointer" << endl;
    return NULL;
  } else if ( length > total_memory_size ){
    cout << "Not enough Memory Null Pointer returned instead" << endl;
    return NULL;
  }

  int bytesRequested = length + sizeof(BlockHeader);
  int indexOfBlockSizeRequested = ceil( log2(bytesRequested) ) - basic_block_base2;

  if( indexOfBlockSizeRequested < 0 ){
    indexOfBlockSizeRequested = 0;
  }

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

  while( indexOfNearestFreeBlock != indexOfBlockSizeRequested ){
    BlockHeader* blockToSplit = FreeList.at(indexOfNearestFreeBlock).head;

    //Need to remove the Block from it's FreeList first
    FreeList.at(indexOfNearestFreeBlock).remove(blockToSplit);

    split(blockToSplit);
    indexOfNearestFreeBlock -= 1;
  }



  //Now that you have a FreeBlock of the right size you need to remove it from the FreeList 
  //and pass the pointer to the User. We add 16 to the pointer in order to account for overhead.
  BlockHeader* freeBlock = FreeList.at(indexOfNearestFreeBlock).head;

  FreeList.at(indexOfNearestFreeBlock).remove(freeBlock);

  //Set in_use flag
  freeBlock->in_use = '1';

  return ( (void*)( (char*)freeBlock + 16 ) ) ;

};

void BuddyAllocator::free(void* a) {


  //Reset block for insertion
  BlockHeader* blockToFree = (BlockHeader*)( (char*)a - 16);
  blockToFree->in_use = '0';

  //insert back into FreeList
  short indexToInsertBlock = (short)(log2( blockToFree->block_size) - basic_block_base2);

  FreeList.at(indexToInsertBlock).insert(blockToFree);

  for(short freeListIndex = 0; freeListIndex < max_level_index; freeListIndex++){

    if(FreeList.at(freeListIndex).head != NULL && FreeList.at(freeListIndex).head->next){

      BlockHeader* blockToCheck = FreeList.at(freeListIndex).head;
      BlockHeader* blockToCheckBuddy = getbuddy(blockToCheck);

      if(FreeList.at(freeListIndex).head->next == blockToCheckBuddy){
        merge( blockToCheck, blockToCheckBuddy );
      }
    }
  }
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

