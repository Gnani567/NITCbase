#include "StaticBuffer.h"
// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];
StaticBuffer::StaticBuffer() {

    // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
  for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];
		Disk::readBlock(buffer, blockIndex);

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++)
			StaticBuffer::blockAllocMap[blockAllocMapSlot] = buffer[slot];
	}
  // initialise all blocks as free
  for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].blockNum = -1;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = 0;
  }

}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {
    //copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
   for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++) 
			buffer[slot] = blockAllocMap[blockAllocMapSlot];

		Disk::writeBlock(buffer, blockIndex);
	}
  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    if(metainfo[bufferIndex].dirty && metainfo[bufferIndex].free==false) {
      Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
    }
  }
}

int StaticBuffer::getFreeBuffer(int blockNum){
    if(blockNum<0 || blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(!metainfo[i].free){
            metainfo[i].timeStamp++;
        }
    }
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.

    // increase the timeStamp in metaInfo of all occupied buffers.

    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum=-1;
     // iterate through metainfo and check if there is any buffer free

    // if a free buffer is available, set bufferNum = index of that free buffer.
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(metainfo[i].free){
            bufferNum=i;
            break;
        }
    }
    if(bufferNum==-1){
        // if a free buffer is not available,
        //     find the buffer with the largest timestamp
        int maxTimeStamp=-1;
        for(int i=0;i<BUFFER_CAPACITY;i++){
            if(metainfo[i].timeStamp>maxTimeStamp){
                maxTimeStamp=metainfo[i].timeStamp;
                bufferNum=i;
            }
        }
        //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
        if(metainfo[bufferNum].dirty){
            Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
        }
    }
    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer
    metainfo[bufferNum].free=false;
    metainfo[bufferNum].dirty=false;
    metainfo[bufferNum].blockNum=blockNum;
    metainfo[bufferNum].timeStamp=0;
    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    return bufferNum;
    // return the bufferNum.
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::setDirtyBit(int blockNum){
  // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum=StaticBuffer::getBufferNum(blockNum);
    if(bufferNum==E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }
    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo

    // return SUCCESS
    if(bufferNum==E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }
    else{
        metainfo[bufferNum].dirty=true;
    }
    return SUCCESS;
}
int StaticBuffer::getBufferNum(int blockNum) {
    // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
    if(blockNum<0 || blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(!metainfo[i].free && metainfo[i].blockNum==blockNum){
            return i;
        }
    }
  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}
int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum<0 || blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    return (int)StaticBuffer::blockAllocMap[blockNum];

    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
}