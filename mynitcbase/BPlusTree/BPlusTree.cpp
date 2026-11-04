#include "BPlusTree.h"
#include "../Buffer/counter.h"
#include <cstring>
RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // declare searchIndex which will be used to store search index for attrName.
    IndexId searchIndex;

     /* get the search index corresponding to attribute with name attrName
       using AttrCacheTable::getSearchIndex(). */
    int ret=AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);
    if(ret!=SUCCESS){
        return RecId{-1,-1};
    }

    /* load the attribute cache entry into attrCatEntry using
     AttrCacheTable::getAttrCatEntry(). */
    AttrCatEntry attrCatEntry;
    ret=AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    if(ret!=SUCCESS){
        return RecId{-1,-1};
    }

    // declare variables block and index which will be used during search
    int block, index;

    if (searchIndex.block == -1 && searchIndex.index == -1) {
        // (search is done for the first time)

        // start the search from the first entry of root.
        block = attrCatEntry.rootBlock;
        index = 0;

        if (block == -1) {
            return RecId{-1, -1};
        }

    } else {
        /*a valid searchIndex points to an entry in the leaf index of the attribute's
        B+ Tree which had previously satisfied the op for the given attrVal.*/

        block = searchIndex.block;
        index = searchIndex.index + 1;  // search is resumed from the next index.

        // load block into leaf using IndLeaf::IndLeaf().
        IndLeaf leaf(block);

        // declare leafHead which will be used to hold the header of leaf.
        HeadInfo leafHead;

        ret=leaf.getHeader(&leafHead);
        // load header into leafHead using BlockBuffer::getHeader().

        if (index >= leafHead.numEntries) {
            /* (all the entries in the block has been searched; search from the
            beginning of the next leaf index block. */
            block=leafHead.rblock;  
            index=0; 
            // update block to rblock of current block and index to 0.

            if (block == -1) {
                // (end of linked list reached - the search is done.)
                return RecId{-1, -1};
            }
        }
    }

    while(StaticBuffer::getStaticBlockType(block)==IND_INTERNAL) {  //use StaticBuffer::getStaticBlockType()

        // load the block into internalBlk using IndInternal::IndInternal().
        IndInternal internalBlk(block);

        HeadInfo intHead;

        internalBlk.getHeader(&intHead);
        // load the header of internalBlk into intHead using BlockBuffer::getHeader()

        // declare intEntry which will be used to store an entry of internalBlk.
        InternalEntry intEntry;

        if (op==NE || op==LT || op==LE) {
            
            ret=internalBlk.getEntry(&intEntry, 0);
            if(ret!=SUCCESS){
                return RecId{-1,-1};
            }
            // load entry in the first slot of the block into intEntry
            // using IndInternal::getEntry().

            block = intEntry.lChild;

        } else {
            
            bool found = false;
            for(int i=0;i<intHead.numEntries;i++){
                ret=internalBlk.getEntry(&intEntry, i);
                if(ret!=SUCCESS){
                    return RecId{-1,-1};
                }
                // load entry in the i'th slot of the block into intEntry
                // using IndInternal::getEntry().
                count++;
                int cmpVal = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType);
                if((op == EQ || op == GE) && cmpVal >= 0){
                    block = intEntry.lChild;
                    found = true;
                    break;
                }
                if(op == GT && cmpVal > 0){
                    block = intEntry.lChild;
                    found = true;
                    break;
                }
            }
            if (!found) {
               ret=internalBlk.getEntry(&intEntry,intHead.numEntries-1);
               if(ret!=SUCCESS){
                return RecId{-1,-1};
               }
               block=intEntry.rChild;
            }
        }
    }

    // NOTE: `block` now has the block number of a leaf index block.

    /******  Identify the first leaf index entry from the current position
                that satisfies our condition (moving right)             ******/

    while (block != -1) {
        // load the block into leafBlk using IndLeaf::IndLeaf().
        IndLeaf leafBlk(block);
        HeadInfo leafHead;

        ret=leafBlk.getHeader(&leafHead);

        if(ret!=SUCCESS){
            return RecId{-1,-1};
        }

        // load the header to leafHead using BlockBuffer::getHeader().

        // declare leafEntry which will be used to store an entry from leafBlk
        Index leafEntry;

        while (index < leafHead.numEntries) {

            ret=leafBlk.getEntry(&leafEntry,index);
            if(ret!=SUCCESS){
                return RecId{-1,-1};
            }
            // load entry corresponding to block and index into leafEntry
            // using IndLeaf::getEntry().
            count++;
            int cmpVal = compareAttrs(leafEntry.attrVal,attrVal,attrCatEntry.attrType);

            if (
                (op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == NE && cmpVal != 0)
            ) {
                IndexId newIdx;
                newIdx.block=block;
                newIdx.index=index;
                // (entry satisfying the condition found)

                // set search index to {block, index}
                AttrCacheTable::setSearchIndex(relId,attrName,&newIdx);

                // return the recId {leafEntry.block, leafEntry.slot}.
                return RecId{leafEntry.block,leafEntry.slot};

            } else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
                /*future entries will not satisfy EQ, LE, LT since the values
                    are arranged in ascending order in the leaves */

                return RecId {-1, -1};
            }

            // search next index.
            ++index;
        }

        /*only for NE operation do we have to check the entire linked list;
        for all the other op it is guaranteed that the block being searched
        will have an entry, if it exists, satisying that op. */
        if (op != NE) {
            break;
        }

        block=leafHead.rblock;
        index=0;
        // block = next block in the linked list, i.e., the rblock in leafHead.
        // update index to 0.
    }

    // no entry satisying the op was found; return the recId {-1,-1}
    return RecId{-1,-1};
}
int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

    // if relId is either RELCAT_RELID or ATTRCAT_RELID:
    //     return E_NOTPERMITTED;
    if(relId==0 || relId==1){
        return E_NOTPERMITTED;
    }
    
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    // get the attribute catalog entry of attribute `attrName`
    // using AttrCacheTable::getAttrCatEntry()
    if(ret!=SUCCESS){
        return ret;
    }
    // if getAttrCatEntry fails
    //     return the error code from getAttrCatEntry

    if (attrCatEntry.rootBlock!=-1) {
        return SUCCESS;
    }

    /******Creating a new B+ Tree ******/

    // get a free leaf block using constructor 1 to allocate a new block
    IndLeaf rootBlockBuf;

    int rootBlock = rootBlockBuf.getBlockNum();

    attrCatEntry.rootBlock=rootBlock;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);
    // if there is no more disk space for creating an index
    if (rootBlock == E_DISKFULL) {
        return E_DISKFULL;
    }

    RelCatEntry relCatEntry;
    ret=RelCacheTable::getRelCatEntry(relId,&relCatEntry);
    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().

    int block = relCatEntry.firstBlk;

    /***** Traverse all the blocks in the relation and insert them one
           by one into the B+ Tree *****/
    while (block != -1) {

        RecBuffer currBlock(block);
        // declare a RecBuffer object for `block` (using appropriate constructor)

        unsigned char slotMap[relCatEntry.numSlotsPerBlk];
        currBlock.getSlotMap(slotMap);
        // load the slot map into slotMap using RecBuffer::getSlotMap().

        for(int i=0;i<relCatEntry.numSlotsPerBlk;i++)
        {
            if(slotMap[i]==SLOT_UNOCCUPIED){
                continue;
            }
            Attribute record[relCatEntry.numAttrs];
            // load the record corresponding to the slot into `record`
            // using RecBuffer::getRecord().
            currBlock.getRecord(record,i);

            // declare recId and store the rec-id of this record in it
            RecId recId{block, i};

            // insert the attribute value corresponding to attrName from the record
            // into the B+ tree using bPlusInsert.
            // (note that bPlusInsert will destroy any existing bplus tree if
            // insert fails i.e when disk is full)
            ret = bPlusInsert(relId, attrName, record[attrCatEntry.offset], recId);

             if (ret== E_DISKFULL) {
             // (unable to get enough blocks to build the B+ Tree.)
             return E_DISKFULL;
            }
        }

        HeadInfo header;
        currBlock.getHeader(&header);
        // get the header of the block using BlockBuffer::getHeader()
        block=header.rblock;
        // set block = rblock of current block (from the header)
    }

    return SUCCESS;
}
int BPlusTree::bPlusDestroy(int rootBlockNum) {
    if (rootBlockNum<0 || rootBlockNum>=DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }

    int type =StaticBuffer::getStaticBlockType(rootBlockNum);

    if (type == IND_LEAF) {
        IndLeaf leafBlk(rootBlockNum);
        // declare an instance of IndLeaf for rootBlockNum using appropriate
        // constructor
        leafBlk.releaseBlock();
        // release the block using BlockBuffer::releaseBlock().

        return SUCCESS;

    } else if (type == IND_INTERNAL) {
        IndInternal intblk(rootBlockNum);
        // declare an instance of IndInternal for rootBlockNum using appropriate
        // constructor
        HeadInfo head;
        intblk.getHeader(&head);
        // load the header of the block using BlockBuffer::getHeader().
        InternalEntry entry;
        intblk.getEntry(&entry,0);
        if(entry.lChild!=-1){
            bPlusDestroy(entry.lChild);
        }
        for(int i=0;i<head.numEntries;i++){
            intblk.getEntry(&entry,i);
            if(entry.rChild!=-1){
            bPlusDestroy(entry.rChild);
            }
        }
        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */
         intblk.releaseBlock();

        return SUCCESS;

    } else {
        // (block is not an index block.)
        return E_INVALIDBLOCK;
    }
}
int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    if(ret!=SUCCESS){
        return ret;
    }
    // if getAttrCatEntry() failed
    //     return the error code

    int blockNum = attrCatEntry.rootBlock;

    if (blockNum==-1) {
        return E_NOINDEX;
    }

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    int leafBlkNum = findLeafToInsert(blockNum, attrVal, attrCatEntry.attrType);

    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.
    // declare a struct Index with attrVal = attrVal, block = recId.block and
    Index index;
    index.attrVal=attrVal;
    index.block=recId.block;
    index.slot=recId.slot;
    // slot = recId.slot to pass as argument to the function.
    ret=insertIntoLeaf(relId, attrName, leafBlkNum,index);
    // NOTE: the insertIntoLeaf() function will propagate the insertion to the
    //       required internal nodes by calling the required helper functions
    //       like insertIntoInternal() or createNewRoot()

    if (ret==E_DISKFULL) {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
        bPlusDestroy(blockNum);
        // update the rootBlock of attribute catalog cache entry to -1 using
        // AttrCacheTable::setAttrCatEntry().
        attrCatEntry.rootBlock=-1;
        AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);

        return E_DISKFULL;
    }

    return SUCCESS;
}
int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {
    int blockNum = rootBlock;

    while (StaticBuffer::getStaticBlockType(blockNum)!=IND_LEAF) {  // use StaticBuffer::getStaticBlockType()

        IndInternal intblk(blockNum);
        // declare an IndInternal object for block using appropriate constructor
        HeadInfo header;
        intblk.getHeader(&header);
        // get header of the block using BlockBuffer::getHeader()

        /* iterate through all the entries, to find the first entry whose
             attribute value >= value to be inserted.
             NOTE: the helper function compareAttrs() declared in BlockBuffer.h
                   can be used to compare two Attribute values. */
        int found=0;
        InternalEntry entry;
        for(int i=0;i<header.numEntries;i++){
            intblk.getEntry(&entry,i);
            int cmpVal=compareAttrs(entry.attrVal,attrVal,attrType);
            if(cmpVal>=0){
                found=1;
                break;
            }
    }
        if (!found) {
            // set blockNum = rChild of (nEntries-1)'th entry of the block
            // (i.e. rightmost child of the block)
            blockNum=entry.rChild;
        } else {
            blockNum=entry.lChild;
            // set blockNum = lChild of the entry that was found
        }
    }
    return blockNum;
}
int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry) {
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().

    // declare an IndLeaf instance for the block using appropriate constructor

    IndLeaf leafBlk(blockNum);
    HeadInfo blockHeader;
    // store the header of the leaf index block into blockHeader
    // using BlockBuffer::getHeader()
    leafBlk.getHeader(&blockHeader);
    // the following variable will be used to store a list of index entries with
    // existing indices + the new index to insert
    Index indices[blockHeader.numEntries + 1];

    for(int i=0;i<blockHeader.numEntries;i++){
        leafBlk.getEntry(&indices[i],i);
    }
    /*
    Iterate through all the entries in the block and copy them to the array indices.
    Also insert `indexEntry` at appropriate position in the indices array maintaining
    the ascending order.
    - use IndLeaf::getEntry() to get the entry
    - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
    */
   int i, retVal;
   for(i=0;i<blockHeader.numEntries;i++){
        retVal=compareAttrs(indices[i].attrVal,indexEntry.attrVal,attrCatEntry.attrType);
        if(retVal>0){
            for(int j=blockHeader.numEntries;j>i;j--){
                indices[j]=indices[j-1];
            }
            indices[i]=indexEntry;
            break;
        }
    }
        if(i==blockHeader.numEntries){
            indices[i]=indexEntry;
        }
        if(blockHeader.numEntries!=MAX_KEYS_LEAF){
        // (leaf block has not reached max limit)
        blockHeader.numEntries++;
        leafBlk.setHeader(&blockHeader);
        // increment blockHeader.numEntries and update the header of block
        // using BlockBuffer::setHeader().
        for(int i=0;i<blockHeader.numEntries;i++){
            leafBlk.setEntry(&indices[i],i);
        }
        // iterate through all the entries of the array `indices` and populate the
        // entries of block with them using IndLeaf::setEntry().

        return SUCCESS;
    }

    int newRightBlk = splitLeaf(blockNum, indices);

     // if splitLeaf() returned E_DISKFULL
    //     return E_DISKFULL
    if(newRightBlk==E_DISKFULL){
        return E_DISKFULL;
    }
    /* if the current leaf block was not the root */
    if (blockHeader.pblock!=-1) {  // check pblock in header
        // insert the middle value from `indices` into the parent block using the
        // insertIntoInternal() function. (i.e the last value of the left block)
        InternalEntry newIntEntry;
        newIntEntry.attrVal=indices[MIDDLE_INDEX_LEAF].attrVal;
        newIntEntry.lChild=blockNum;
        newIntEntry.rChild=newRightBlk;

        return insertIntoInternal(relId,attrName,blockHeader.pblock,newIntEntry);
        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)

    } else {
        
        return createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,blockNum, newRightBlk);
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
}
int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {
    IndLeaf rightBlk;
    // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
    // leaf index block that will be used as the right block in the splitting

    IndLeaf leftBlk(leafBlockNum);
    // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
    // the existing leaf block

    int rightBlkNum = rightBlk.getBlockNum();
    int leftBlkNum = leafBlockNum;

    if (rightBlkNum==E_DISKFULL) {
        //(failed to obtain a new leaf index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    rightBlk.getHeader(&rightBlkHeader);
    leftBlk.getHeader(&leftBlkHeader);
    // set rightBlkHeader with the following values
    rightBlkHeader.numEntries=32;
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
    rightBlkHeader.pblock=leftBlkHeader.pblock;
    // - pblock = pblock of leftBlk
    // - lblock = leftBlkNum
    rightBlkHeader.lblock=leftBlkNum;
    // - rblock = rblock of leftBlk
    rightBlkHeader.rblock=leftBlkHeader.rblock;
    if(rightBlkHeader.rblock!=-1){
        IndLeaf nextBlk(rightBlkHeader.rblock);
        HeadInfo nextHeader;
        nextBlk.getHeader(&nextHeader);
        nextHeader.lblock=rightBlkNum;
        nextBlk.setHeader(&nextHeader);
    }
    // and update the header of rightBlk using BlockBuffer::setHeader()
    rightBlk.setHeader(&rightBlkHeader);
    // set leftBlkHeader with the following values
    leftBlkHeader.numEntries=32;
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
    // - rblock = rightBlkNum
    leftBlkHeader.rblock=rightBlkNum;
    // and update the header of leftBlk using BlockBuffer::setHeader() */
    leftBlk.setHeader(&leftBlkHeader);
    for(int i=0;i<leftBlkHeader.numEntries;i++){
        leftBlk.setEntry(&indices[i],i);
    }
    // set the first 32 entries of leftBlk = the first 32 entries of indices array
    for(int i=0;i<rightBlkHeader.numEntries;i++){
        rightBlk.setEntry(&indices[i+leftBlkHeader.numEntries],i);
    }
    // and set the first 32 entries of newRightBlk = the next 32 entries of
    // indices array using IndLeaf::setEntry().

    return rightBlkNum;
}
int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry intEntry) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    // declare intBlk, an instance of IndInternal using constructor 2 for the block
    // corresponding to intBlockNum
    IndInternal intBlk(intBlockNum);

    HeadInfo blockHeader;
    // load blockHeader with header of intBlk using BlockBuffer::getHeader().
    intBlk.getHeader(&blockHeader);
    // declare internalEntries to store all existing entries + the new entry
    InternalEntry internalEntries[blockHeader.numEntries + 1];

    for(int i=0;i<blockHeader.numEntries;i++){
        intBlk.getEntry(&internalEntries[i],i);
    }

    int i , retVal;
    for(i=0;i<blockHeader.numEntries;i++){
        retVal=compareAttrs(internalEntries[i].attrVal,intEntry.attrVal,attrCatEntry.attrType);
        if(retVal>0){
            for(int j=blockHeader.numEntries;j>i;j--){
                internalEntries[j]=internalEntries[j-1];
            }
            internalEntries[i]=intEntry;
            break;
        }
    }
    if(i==blockHeader.numEntries){
        internalEntries[i]=intEntry;
    }
    if(i<blockHeader.numEntries){
        internalEntries[i+1].lChild=intEntry.rChild;
    }
    if(i>0){
        internalEntries[i-1].rChild=intEntry.lChild;
    }
    if (blockHeader.numEntries != MAX_KEYS_INTERNAL) {
        // (internal index block has not reached max limit)
        blockHeader.numEntries++;
        // increment blockheader.numEntries and update the header of intBlk
        // using BlockBuffer::setHeader().
        intBlk.setHeader(&blockHeader);
        // iterate through all entries in internalEntries array and populate the
        // entries of intBlk with them using IndInternal::setEntry().
        for(int i=0;i<blockHeader.numEntries;i++){
            intBlk.setEntry(&internalEntries[i],i);
        }
        return SUCCESS;
    }

    int newRightBlk = splitInternal(intBlockNum, internalEntries);

    if (newRightBlk==E_DISKFULL) {

        bPlusDestroy(intEntry.rChild);
        // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree

        return E_DISKFULL;
    }

    if (blockHeader.pblock!=-1) {  // (check pblock in header)

        // insert the middle value from `internalEntries` into the parent block
        // using the insertIntoInternal() function (recursively).
        InternalEntry entry;
        entry.lChild=intBlockNum;
        entry.rChild=newRightBlk;
        entry.attrVal=internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;
        // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)

        // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
        // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
        // and pass it as argument to the insertIntoInternalFunction as follows

        return insertIntoInternal(relId, attrName,blockHeader.pblock,entry);

    } else {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        return createNewRoot(relId, attrName,internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,intBlockNum,newRightBlk);
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
}
int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {
    // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
    // internal index block that will be used as the right block in the splitting
    IndInternal rightBlk;
    // declare leftBlk, an instance of IndInternal using constructor 2 to read from
    // the existing internal index block
    IndInternal leftBlk(intBlockNum);
    int rightBlkNum = rightBlk.getBlockNum();
    int leftBlkNum =intBlockNum;

    if (rightBlkNum==E_DISKFULL) {
        //(failed to obtain a new internal index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    leftBlk.getHeader(&leftBlkHeader);
    rightBlk.getHeader(&rightBlkHeader);
    // set rightBlkHeader with the following values
    rightBlkHeader.numEntries=50;
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    rightBlkHeader.pblock=leftBlkHeader.pblock;
    // - pblock = pblock of leftBlk
    rightBlk.setHeader(&rightBlkHeader);
    // and update the header of rightBlk using BlockBuffer::setHeader()

    // set leftBlkHeader with the following values
    leftBlkHeader.numEntries=50;
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    leftBlk.setHeader(&leftBlkHeader);

   for(int i=0;i<leftBlkHeader.numEntries;i++){
        leftBlk.setEntry(&internalEntries[i],i);
   }
   for(int i=0;i<rightBlkHeader.numEntries;i++){
        rightBlk.setEntry(&internalEntries[i+MIDDLE_INDEX_INTERNAL+1],i);
   }

    int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild);
    //            (use StaticBuffer::getStaticBlockType())
    BlockBuffer firstBlk(internalEntries[MIDDLE_INDEX_INTERNAL+1].lChild);
    HeadInfo firstChildHeader;
    firstBlk.getHeader(&firstChildHeader);
    firstChildHeader.pblock=rightBlkNum;
    firstBlk.setHeader(&firstChildHeader);

    for (int i=0;i<MIDDLE_INDEX_INTERNAL;i++) {
        // declare an instance of BlockBuffer to access the child block using
        // constructor 2
        BlockBuffer childBlk(internalEntries[i+MIDDLE_INDEX_INTERNAL+1].rChild);

        HeadInfo childHeader;
        childBlk.getHeader(&childHeader);
        childHeader.pblock=rightBlkNum;
        childBlk.setHeader(&childHeader);
        // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
        // and BlockBuffer::setHeader().
    }

    return rightBlkNum;
}
int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
    AttrCatEntry attrCatEntry;
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    
    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newRootBlk;
    int newRootBlkNum = newRootBlk.getBlockNum();

    if (newRootBlkNum == E_DISKFULL) {
        // (failed to obtain an empty internal index block because the disk is full)
        bPlusDestroy(rChild);

        return E_DISKFULL;
    }

    HeadInfo newHeader;
    newRootBlk.getHeader(&newHeader);
    newHeader.numEntries=1;
    newRootBlk.setHeader(&newHeader);
    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    InternalEntry entry;
    entry.attrVal=attrVal;
    entry.lChild=lChild;
    entry.rChild=rChild;
    newRootBlk.setEntry(&entry,0);
    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()

    BlockBuffer lBlk(lChild);
    BlockBuffer rBlk(rChild);
    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
    HeadInfo lHeader;
    lBlk.getHeader(&lHeader);
    lHeader.pblock=newRootBlkNum;
    lBlk.setHeader(&lHeader);
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()

    HeadInfo rHeader;
    rBlk.getHeader(&rHeader);
    rHeader.pblock=newRootBlkNum;
    rBlk.setHeader(&rHeader);

    attrCatEntry.rootBlock=newRootBlkNum;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);
    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().

    return SUCCESS;
}