#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;

  int ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;
  }

  memcpy(&head->lblock,     bufferPtr + 8,  4);
  memcpy(&head->rblock,     bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs,   bufferPtr + 20, 4);
  memcpy(&head->numSlots,   bufferPtr + 24, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {

  struct HeadInfo head;

  // get the header
  int ret = this->getHeader(&head);

  if (ret != SUCCESS) {
    return ret;
  }

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  unsigned char *bufferPtr;

  // load the block using the buffer manager
  ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;
  }

  /*
   * Record at slotNum will be at:
   * HEADER_SIZE + slotMapSize + (recordSize * slotNum)
   */
  int recordSize = attrCount * ATTR_SIZE;
  int slotMapSize = slotCount;

  unsigned char *slotPointer = bufferPtr + HEADER_SIZE +slotMapSize +(recordSize * slotNum);

  // load the record
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

// set the record at slotNum
int RecBuffer::setRecord(union Attribute *rec, int slotNum) {

  struct HeadInfo head;

  // get the header
  int ret = this->getHeader(&head);

  if (ret != SUCCESS) {
    return ret;
  }

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  unsigned char *bufferPtr;

  // load the block using the buffer manager
  ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;
  }

  /*
   * Record at slotNum will be at:
   * HEADER_SIZE + slotMapSize + (recordSize * slotNum)
   */
  int recordSize = attrCount * ATTR_SIZE;
  int slotMapSize = slotCount;

  unsigned char *slotPointer = bufferPtr + HEADER_SIZE +slotMapSize +(recordSize * slotNum);

  // store the record
  memcpy(slotPointer, rec, recordSize);

  // write the modified block back to disk
  Disk::writeBlock(bufferPtr, this->blockNum);

  return SUCCESS;
}

// load a block to the buffer and get a pointer to it
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {

  // check whether the block is already present in the buffer
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum == E_BLOCKNOTINBUFFER) {

    // get a free buffer
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }

    // read the block from disk into the buffer
    Disk::readBlock(
        StaticBuffer::blocks[bufferNum],
        this->blockNum
    );
  }

  // store the pointer to this buffer in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}