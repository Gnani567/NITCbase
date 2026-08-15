#include "StaticBuffer.h"

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];

struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all blocks as free
  for (int bufferIndex = 0;bufferIndex < BUFFER_CAPACITY;bufferIndex++) {

    metainfo[bufferIndex].free = true;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {}


int StaticBuffer::getFreeBuffer(int blockNum) {

  // check whether blockNum is valid
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  int allocatedBuffer = E_OUTOFBOUND;

  // iterate through all the blocks in the StaticBuffer
  // find the first free block in the buffer
  for (int bufferIndex = 0;bufferIndex < BUFFER_CAPACITY;bufferIndex++) {

    if (metainfo[bufferIndex].free) {
      allocatedBuffer = bufferIndex;
      break;
    }
  }

  // no free buffer was found
  if (allocatedBuffer == E_OUTOFBOUND) {
    return E_OUTOFBOUND;
  }

  // mark the buffer as occupied
  metainfo[allocatedBuffer].free = false;

  // store the disk block number in the metadata
  metainfo[allocatedBuffer].blockNum = blockNum;

  return allocatedBuffer;
}

/*
Get the buffer index where a particular block is stored
or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {

  // check whether blockNum is valid
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // search for the block in the buffer
  for (int bufferIndex = 0;bufferIndex < BUFFER_CAPACITY;bufferIndex++) {

    if (!metainfo[bufferIndex].free &&metainfo[bufferIndex].blockNum == blockNum) {

      return bufferIndex;
    }
  }

  // block is not currently present in the buffer
  return E_BLOCKNOTINBUFFER;
}