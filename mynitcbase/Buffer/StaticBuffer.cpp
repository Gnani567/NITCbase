#include "StaticBuffer.h"

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];

struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        metainfo[bufferIndex].free = true;
        metainfo[bufferIndex].dirty = false;
        metainfo[bufferIndex].timeStamp = -1;
        metainfo[bufferIndex].blockNum = -1;
    }
}

// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {

    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if (metainfo[bufferIndex].free == false &&
            metainfo[bufferIndex].dirty == true) {

            Disk::writeBlock(
                blocks[bufferIndex],
                metainfo[bufferIndex].blockNum
            );
        }
    }
}

int StaticBuffer::getFreeBuffer(int blockNum) {

    if (blockNum <= 0 || blockNum >= DISK_BLOCKS)
        return E_OUTOFBOUND;

    // Increase timeStamp of occupied buffers
    for (int i = 0; i < BUFFER_CAPACITY; i++) {
        if (metainfo[i].free == false)
            metainfo[i].timeStamp++;
    }

    int bufferNum = -1;

    // Find a free buffer
    for (int i = 0; i < BUFFER_CAPACITY; i++) {
        if (metainfo[i].free == true) {
            bufferNum = i;
            break;
        }
    }

    // No free buffer: find the oldest buffer
    if (bufferNum == -1) {
        int maxTimeStamp = -1;

        for (int i = 0; i < BUFFER_CAPACITY; i++) {
            if (metainfo[i].timeStamp > maxTimeStamp) {
                maxTimeStamp = metainfo[i].timeStamp;
                bufferNum = i;
            }
        }

        if (metainfo[bufferNum].dirty == true) {
            Disk::writeBlock(
                blocks[bufferNum],
                metainfo[bufferNum].blockNum
            );
        }
    }

    metainfo[bufferNum].free = false;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].timeStamp = 0;

    return bufferNum;
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

int StaticBuffer::setDirtyBit(int blockNum) {

    int bufferNum = getBufferNum(blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER)
        return E_BLOCKNOTINBUFFER;

    if (bufferNum == E_OUTOFBOUND)
        return E_OUTOFBOUND;

    metainfo[bufferNum].dirty = true;

    return SUCCESS;
}