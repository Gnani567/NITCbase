#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blockType) {
    int type;

    if(blockType == 'R')
        type = REC;
    else if(blockType == 'I')
        type = IND_INTERNAL;
    else
        type = IND_LEAF;

    this->blockNum = getFreeBlock(type);
}

int BlockBuffer::getBlockNum(){
    return this->blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

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

  unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotMapSize +(recordSize * slotNum);

  // load the record
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {

    unsigned char *bufferPtr;

    int retVal = loadBlockAndGetBufferPtr(&bufferPtr);

    if (retVal != SUCCESS)
        return retVal;

    HeadInfo head;

    retVal = getHeader(&head);

    if (retVal != SUCCESS)
        return retVal;

    int numAttrs = head.numAttrs;
    int numSlots = head.numSlots;

    if (slotNum < 0 || slotNum >= numSlots)
        return E_OUTOFBOUND;

    int recordSize = ATTR_SIZE * numAttrs;
    int slotMapSize = numSlots;

    unsigned char *recordPtr =
        bufferPtr + HEADER_SIZE + slotMapSize +
        (slotNum * recordSize);

    memcpy(recordPtr, rec, recordSize);

    retVal = StaticBuffer::setDirtyBit(this->blockNum);

    if (retVal != SUCCESS)
        return retVal;

    return SUCCESS;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum != E_BLOCKNOTINBUFFER) {

        for (int i = 0; i < BUFFER_CAPACITY; i++) {
            if (i == bufferNum)
                StaticBuffer::metainfo[i].timeStamp = 0;
            else if (StaticBuffer::metainfo[i].free == false)
                StaticBuffer::metainfo[i].timeStamp++;
        }

    } else {

        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

        if (bufferNum == E_OUTOFBOUND)
            return E_OUTOFBOUND;

        Disk::readBlock(
            StaticBuffer::blocks[bufferNum],
            this->blockNum
        );
    }

    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // Get the buffer containing this block
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;

  // Get the header
  ret = getHeader(&head);
  if (ret != SUCCESS) {
    return ret;
  }

  // Number of slots in this record block
  int slotCount = head.numSlots;

  // Slot map starts immediately after the header
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // Copy slot map from buffer to caller's array
  memcpy(slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;

    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret != SUCCESS)
        return ret;

    // get the header of the block using the getHeader() function
    struct HeadInfo head;

    ret = getHeader(&head);
    if (ret != SUCCESS)
        return ret;

    int numSlots = head.numSlots;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
    unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

    memcpy(slotMapInBuffer, slotMap, numSlots);

    // update dirty bit using StaticBuffer::setDirtyBit
    ret = StaticBuffer::setDirtyBit(this->blockNum);

    // if setDirtyBit failed, return the value returned by the call
    if (ret != SUCCESS)
        return ret;

    // return SUCCESS
    return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;

    if (attrType == STRING)
        diff = strcmp(attr1.sVal, attr2.sVal);
    else
        diff = attr1.nVal - attr2.nVal;

    if (diff > 0)
        return 1;

    if (diff < 0)
        return -1;

    return 0;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    bufferHeader->pblock = head->pblock;
    bufferHeader->lblock = head->lblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numSlots = head->numSlots;

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    *((int32_t *)bufferPtr) = blockType;

    StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    ret = StaticBuffer::setDirtyBit(this->blockNum);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int BlockBuffer::getFreeBlock(int blockType){

    int freeBlockNum = -1;

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    for (int i = 4; i < DISK_BLOCKS; i++) {
        if (StaticBuffer::blockAllocMap[i] == UNUSED_BLK) {
            freeBlockNum = i;
            break;
        }
    }

    // if no block is free, return E_DISKFULL.
    if (freeBlockNum == -1)
        return E_DISKFULL;

    // set the object's blockNum to the block number of the free block.
    this->blockNum = freeBlockNum;

    // find a free buffer using StaticBuffer::getFreeBuffer().
    int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum < 0)
        return bufferNum;

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    struct HeadInfo head;

    head.pblock = -1;
    head.lblock = -1;
    head.rblock = -1;
    head.numEntries = 0;
    head.numAttrs = 0;
    head.numSlots = 0;

    int ret = setHeader(&head);

    if (ret != SUCCESS)
        return ret;

    // update the block type of the block to the input block type using setBlockType().
    ret = setBlockType(blockType);

    if (ret != SUCCESS)
        return ret;

    // return block number of the free block.
    return this->blockNum;
}