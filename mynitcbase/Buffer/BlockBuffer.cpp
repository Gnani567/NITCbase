#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>
// the declarations for these functions can be found in "BlockBuffer.h"
BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum=blockNum; // every function should know on which block we are currently working on
}


BlockBuffer::BlockBuffer(char blockType){
     // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
    int type=UNUSED_BLK;
    if(blockType=='R'){
        type=0;
    }
    else if(blockType=='I'){
        type=1;
    }
    else if(blockType=='L'){
        type=2;
    }
    int freeBlockNum=getFreeBlock(type);
      // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
    if(freeBlockNum<0){
        this->blockNum=freeBlockNum;
    }
    else{
        this->blockNum=freeBlockNum;
    }
}
RecBuffer::RecBuffer(): BlockBuffer('R') {}
// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {}

// call the corresponding parent constructor
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType){}

// call the corresponding parent constructor
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum){}

IndInternal::IndInternal() : IndBuffer('I'){}
// call the corresponding parent constructor
// 'I' used to denote IndInternal.

IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum){}
// call the corresponding parent constructor

IndLeaf::IndLeaf() : IndBuffer('L'){} // this is the way to call parent non-default constructor.
                      // 'L' used to denote IndLeaf.
//this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum){}

int IndInternal::getEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
    if(indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
        return E_OUTOFBOUND;  // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    }
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
     // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if(ret!=SUCCESS){
        return ret;
    }
   
    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    /*
    - copy the entries from the indexNum`th entry to *internalEntry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
    memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
    memcpy(&(internalEntry->rChild), entryPtr + 20, 4);

    return SUCCESS;
}
int IndInternal::setEntry(void *ptr, int indexNum) {

    if(indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
        return E_OUTOFBOUND;  // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if(ret!=SUCCESS){
        return ret;
    }
    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(entryPtr, &(internalEntry->lChild), 4);
    memcpy(entryPtr + 4, &(internalEntry->attrVal), ATTR_SIZE);
    memcpy(entryPtr + 20, &(internalEntry->rChild), 4);


    StaticBuffer::setDirtyBit(this->blockNum);
    // update dirty bit using setDirtyBit()
    // if setDirtyBit failed, return the value returned by the call

    return SUCCESS;
}
int IndLeaf::getEntry(void *ptr, int indexNum) {

    if(indexNum < 0 || indexNum >= MAX_KEYS_LEAF) {
        return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS){
        return ret;
    }
    // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);

    return SUCCESS;
}
int IndLeaf::setEntry(void *ptr, int indexNum) {

    if(indexNum < 0 || indexNum >= MAX_KEYS_LEAF) {
        return E_OUTOFBOUND;  
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if(ret!=SUCCESS){
        return ret;
    }
    // copy the Index at ptr to indexNum'th entry in the buffer using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy(entryPtr, (struct Index *)ptr, LEAF_ENTRY_SIZE);

    StaticBuffer::setDirtyBit(this->blockNum);

    return SUCCESS;
}

int BlockBuffer::getFreeBlock(int blockType){
     // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int freeBlockNum=-1;
    for(int i=4;i<DISK_BLOCKS;i++){
        if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK){
            freeBlockNum=i;
            break;
            // set the object's blockNum to the block number of the free block.
        }
    }
    // if no block is free, return E_DISKFULL.
    if(freeBlockNum==-1){
        return E_DISKFULL;
    }
    // set the object's blockNum to the block number of the free block.
    this->blockNum=freeBlockNum;
    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int bufferNum=StaticBuffer::getFreeBuffer(this->blockNum);
    if(bufferNum==E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }
    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    struct HeadInfo head;
    head.pblock=-1;
    head.lblock=-1;
    head.rblock=-1;
    head.numEntries=0;
    head.numAttrs=0;
    head.numSlots=0;
    int ret=setHeader(&head);   // to the setHeader() function.
    if(ret!=SUCCESS){
        return ret;
    }
    ret=setBlockType(blockType);
    if(ret!=SUCCESS){
        return ret;
    }
    
    // update the block type of the block to the input block type using setBlockType().
    return freeBlockNum;
    // return block number of the free block.
}

int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }
//   // read the block at this.blockNum into the buffer

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->blockType, bufferPtr, 4);
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries,bufferPtr+16, 4);
  memcpy(&head->numAttrs, bufferPtr+20, 4);
  memcpy(&head->rblock, bufferPtr+12, 4);
  memcpy(&head->lblock, bufferPtr+8, 4);
  memcpy(&head->pblock, bufferPtr+4, 4);

  return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  
  unsigned char *bufferPtr; // points to actual block data
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);  // address of buffer pointer
  if (ret != SUCCESS) {
    return ret;
  }
   struct HeadInfo head;

  ret=this->getHeader(&head);
  if(ret!=SUCCESS){
    return ret;
  }
  // get the header using this.getHeader() function

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  // read the block at this.blockNum into a buffer

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr+32+slotCount+recordSize*slotNum;

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  getHeader(&head);
  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  memcpy(slotMap, slotMapInBuffer, slotCount);
  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)

  return SUCCESS;
}

int BlockBuffer::getBlockNum(){

    return this->blockNum;
}
void BlockBuffer::releaseBlock(){
    // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing
    if(this->blockNum==INVALID_BLOCKNUM){
        return;
    }
    int ret=StaticBuffer::getBufferNum(this->blockNum);

    if(ret!=E_BLOCKNOTINBUFFER){
        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.
        StaticBuffer::metainfo[ret].free=true;
    }
    // free the block in disk by setting the data type of the entry
        // corresponding to the block number in StaticBuffer::blockAllocMap
        // to UNUSED_BLK.
    StaticBuffer::blockAllocMap[this->blockNum]=UNUSED_BLK;
    // set the object's blockNum to INVALID_BLOCK (-1)
    this->blockNum=INVALID_BLOCKNUM;        
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
    if(bufferNum != E_BLOCKNOTINBUFFER){
        // if present (!=E_BLOCKNOTINBUFFER),
        // set the timestamp of the corresponding buffer to 0 and increment the
        // timestamps of all other occupied buffers in BufferMetaInfo.
        for(int i=0;i<BUFFER_CAPACITY;i++){
            if(StaticBuffer::metainfo[i].free==false){
                StaticBuffer::metainfo[i].timeStamp++;
            }
        }
        StaticBuffer::metainfo[bufferNum].timeStamp=0;
    }
    else{
        // else
        // get a free buffer using StaticBuffer.getFreeBuffer()
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum == E_OUTOFBOUND){
            return E_OUTOFBOUND;  // return E_OUTOFBOUND here as the blockNum is invalid
        }
        // Read the block into the free buffer using readBlock()
        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }
    *buffPtr = StaticBuffer::blocks[bufferNum];
    return SUCCESS;
    // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr

    // return SUCCESS;
}
/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    if(attrType==STRING){
        diff=strcmp(attr1.sVal,attr2.sVal);
    }
    else{
        diff=attr1.nVal-attr2.nVal;
    }
    if(diff>0){
        return 1;
    }
    else if(diff<0){
        return -1;
    }
    else{
        return 0;
    }
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
     // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret != SUCCESS) {
        return ret;
    }
    struct HeadInfo head;
    // get the header of the block using the getHeader() function
    getHeader(&head);
    if(ret!=SUCCESS){
        return ret;
    }
    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
    int numSlots = head.numSlots;
    unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
    memcpy(slotMapInBuffer, slotMap, numSlots);
    ret=StaticBuffer::setDirtyBit(this->blockNum);
    if(ret!=SUCCESS){
        return ret;
    }

    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call

    return SUCCESS;
}
int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS){
        return ret;
    }
     /* get the header of the block using the getHeader() function */

    // get number of attributes in the block.

    // get the number of slots in the block.

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    struct HeadInfo head;
    ret=this->getHeader(&head);
    if(ret!=SUCCESS){
        return ret;
    }
    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
    if(slotNum<0 || slotNum>=slotCount){
        return E_OUTOFBOUND;
    }
    unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (attrCount * ATTR_SIZE * slotNum);
    memcpy(slotPointer, rec, attrCount * ATTR_SIZE);
    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    StaticBuffer::setDirtyBit(this->blockNum); // set the dirty bit for this block in the buffer
    // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */
    return SUCCESS;
    // return SUCCESS
}
int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
     // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS){  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
        return ret;
    }
   
    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;
    bufferHeader->blockType = head->blockType;
    bufferHeader->pblock = head->pblock;
    bufferHeader->lblock = head->lblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numSlots = head->numSlots;
    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )

    ret=StaticBuffer::setDirtyBit(this->blockNum);   // set the dirty bit for this block in the buffer
    if(ret!=SUCCESS){
        // if setDirtyBit() failed, return the error code
        return ret;
    }
   
    // update dirty bit by calling StaticBuffer::setDirtyBit()

    return SUCCESS;
}
int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    if(ret!=SUCCESS){  
        return ret;
    }
    
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    *((int32_t *)bufferPtr) = blockType;
    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;

    StaticBuffer::blockAllocMap[this->blockNum] = blockType;
    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    ret=StaticBuffer::setDirtyBit(this->blockNum);

    if(ret!=SUCCESS){
        return ret;
    }
        // return the returned value from the call

    return SUCCESS;
}
