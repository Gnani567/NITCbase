#include "BlockAccess.h"
#include "../Buffer/counter.h"
#include <cstring>
int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal, newName);
    // search the relation catalog for an entry with "RelName" = newRelationName

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    RecId newRelcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", newRelationName, EQ);
    if(newRelcatRecId.block != -1 && newRelcatRecId.slot != -1) {
        return E_RELEXIST;  // relation with name newName already exists
    }
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal, oldName);
    RecId oldRelcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", oldRelationName, EQ);

    // search the relation catalog for an entry with "RelName" = oldRelationName
    if(oldRelcatRecId.block == -1 && oldRelcatRecId.slot == -1) {
        return E_RELNOTEXIST;  // relation with name oldName does not exist
    }
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    RecBuffer relCatBlock(oldRelcatRecId.block);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, oldRelcatRecId.slot);
    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, newName);
   relCatBlock.setRecord(relCatRecord, oldRelcatRecId.slot);
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    for(int i=0; i<relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; i++) {
        //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
        Attribute attrName;
        RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)"RelName", oldRelationName, EQ);
        if(attrcatRecId.block == -1 && attrcatRecId.slot == -1) {
            break;  // no more attributes found for the relation oldName
        }
        RecBuffer attrBuf(attrcatRecId.block);
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrBuf.getRecord(attrCatRecord, attrcatRecId.slot);
        /* read the record entry corresponding to attrcatRecId and create an
           Attribute Cache entry on it using RecBuffer::getRecord() and
           AttrCacheTable::recordToAttrCatEntry().
           update the recId field of this Attribute Cache entry to attrcatRecId.
           add the Attribute Cache entry to the linked list of listHead .*/
        // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        /* update the relation name attribute in the record with newName.
           (use ATTRCAT_REL_NAME_INDEX) */
        // set back the record value using RecBuffer.setRecord
        attrBuf.setRecord(attrCatRecord, attrcatRecId.slot);
    }
    

    return SUCCESS;
}
int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
       RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal, relName);
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", relNameAttr, EQ);
     // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    if(relcatRecId.block == -1 && relcatRecId.slot == -1) {
        return E_RELNOTEXIST;  // relation with name relName does not exist
    }
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
         // linear search on the attribute catalog for RelName = relNameAttr
        
        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)"RelName", relNameAttr, EQ);
        if(attrcatRecId.block == -1 && attrcatRecId.slot == -1) {
            break;  // no more attributes found for the relation relName
        }
       RecBuffer attrBuf(attrcatRecId.block);
        attrBuf.getRecord(attrCatEntryRecord, attrcatRecId.slot);
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0) {
            attrToRenameRecId.block = attrcatRecId.block;
            attrToRenameRecId.slot = attrcatRecId.slot;
        }

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0) {
            return E_ATTREXIST;  // attribute with name newName already exists
        }
        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
    }
    if(attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1) {
        return E_ATTRNOTEXIST;  // attribute with name oldName does not exist
    }
    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;

    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer attrToRenameBuf(attrToRenameRecId.block);
    attrToRenameBuf.getRecord(attrCatEntryRecord, attrToRenameRecId.slot);
    strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
    attrToRenameBuf.setRecord(attrCatEntryRecord, attrToRenameRecId.slot);
    

    return SUCCESS;
}
RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);
    int block, slot;
    
    // let block and slot denote the record id of the record being currently checked

    // if the current search index record is invalid(i.e. both block and slot = -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);
        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        block=relCatEntry.firstBlk;
        slot=0;
        // block = first record block of the relation
        // slot = 0
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)
        block=prevRecId.block;
        slot=prevRecId.slot + 1;
        // block = search index's block
        // slot = search index's slot + 1
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    while (block != -1)
    {
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        RecBuffer recBuffer(block);
        HeadInfo header;
        recBuffer.getHeader(&header);
        int attrs=header.numAttrs;
        // get the record with id (block, slot) using RecBuffer::getRecord()
        // get header of the block using RecBuffer::getHeader() function
        unsigned char slotMap[header.numSlots];
        // get slot map of the block using RecBuffer::getSlotMap() function
        recBuffer.getSlotMap(slotMap);

        if(slot >= header.numSlots)
        // If slot >= the number of slots per block(i.e. no more slots in this block)
        {
            block = header.rblock;  // update block = right block of block
            slot = 0;  // update slot = 0
            // update block = right block of block
            // update slot = 0
            continue;  // continue to the beginning of this while loop
        }

        // if slot is free skip the loop
        if(slotMap[slot] == SLOT_UNOCCUPIED)
        // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        {   
            slot++;
            continue;  // continue to the beginning of this while loop
            // increment slot and continue to the next record slot
        }

        Attribute record[attrs];
        recBuffer.getRecord(record, slot);
        // compare record's attribute value to the the given attrVal as below:
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
        // get the attribute catalog entry for the relation relId and attribute attrName
        /*
            firstly get the attribute offset for the attrName attribute
            from the attribute cache entry of the relation using
            AttrCacheTable::getAttrCatEntry()
        */
       union Attribute recordAttrVal = record[attrCatEntry.offset];
        /* use the attribute offset to get the value of the attribute from
           current record */

        int cmpVal;  // will store the difference between the attributes
        // set cmpVal using compareAttrs()
        count++;
        cmpVal = compareAttrs(recordAttrVal, attrVal, attrCatEntry.attrType);
        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            RecId searchIndex={block, slot};
            RelCacheTable::setSearchIndex(relId, &searchIndex);
            return RecId{block, slot};
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
        }

        slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};
}
/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId{-1,-1};
    int ret=RelCacheTable::getSearchIndex(relId, &prevRecId);
    if(ret!=SUCCESS){
        return ret;
    }
    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {   
        // (new project operation. start from beginning)
        RelCatEntry relCatEntry;
        ret=RelCacheTable::getRelCatEntry(relId, &relCatEntry);
        if(ret!=SUCCESS){
            return ret;
        }
        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        block=relCatEntry.firstBlk;
        slot=0;
        // block = first record block of the relation
        // slot = 0
    }
    else
    {
        // (a project/search operation is already in progress)
        block=prevRecId.block;
        slot=prevRecId.slot+1;
        // block = previous search index's block
        // slot = previous search index's slot + 1
    }
    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {   
        RecBuffer recBuffer(block);
        // create a RecBuffer object for block (using appropriate constructor!)
        HeadInfo header;
        ret=recBuffer.getHeader(&header);
        if(ret!=SUCCESS){
            return ret;
        }
        unsigned char slotMap[header.numSlots];
        ret=recBuffer.getSlotMap(slotMap);
        if(ret!=SUCCESS){
            return ret;
        }
        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function

        if(slot >= header.numSlots)
        {   
             // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            block=header.rblock;
            slot=0;
           
        }
        else if (slotMap[slot] == SLOT_UNOCCUPIED)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)

            slot++;
            // increment slot
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};
    ret=RelCacheTable::setSearchIndex(relId, &nextRecId);
    if(ret!=SUCCESS){
        return ret;
    }
    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RecBuffer rec(nextRecId.block);
     /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    ret=rec.getRecord(record, nextRecId.slot);
    if(ret!=SUCCESS){
        return ret;
    }
    return SUCCESS;
}
int BlockAccess::insert(int relId, Attribute *record) {
    RelCatEntry relCatEntry;
     // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    int ret=RelCacheTable::getRelCatEntry(relId, &relCatEntry);
    if(ret!=SUCCESS){
        return ret;
    }
    int blockNum = relCatEntry.firstBlk;  // get the last record block of the relation

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk;  // get the number of slots per block
    int numOfAttributes = relCatEntry.numAttrs;  // get the number of attributes in the relation

    int prevBlockNum = -1;  // will store the previous block number in the linked list of record blocks

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        RecBuffer recBuffer(blockNum);
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        struct HeadInfo header;
        recBuffer.getHeader(&header);
        // get header of block(blockNum) using RecBuffer::getHeader() function

        unsigned char slotMap[header.numSlots];
        recBuffer.getSlotMap(slotMap);
        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        for (int slot = 0; slot < header.numSlots; slot++) {
            /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */
            if (slotMap[slot] == SLOT_UNOCCUPIED) {
               
                rec_id.block = blockNum;
                rec_id.slot = slot;
                break;  // free slot found, exit the loop
            }
        }
         /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */
        if(rec_id.block != -1 && rec_id.slot != -1) {
            break;  // free slot found, exit the loop
        }
        prevBlockNum = blockNum;  // update prevBlockNum to blockNum
        blockNum = header.rblock;  // update blockNum to the right block of block
        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id.block == -1 && rec_id.slot == -1)
    {
        // if relation is RELCAT, do not allocate any more blocks
            if(relId == RELCAT_RELID) {
                return E_MAXRELATIONS;
            }

        // Otherwise,
        RecBuffer newRecBlock;
        int ret=newRecBlock.getBlockNum();
         // (use BlockBuffer::getBlockNum() function)
        // get a new record block (using the appropriate RecBuffer constructor!)
        // get the block number of the newly allocated block
        // let ret be the return value of getBlockNum() function call
        if (ret == E_DISKFULL) {
            return E_DISKFULL;
        }

         // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
        rec_id.block = ret;  // set rec_id.block = new block number(i.e. ret)
        rec_id.slot = 0;  // set rec_id.slot = 0
       
        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */
        struct HeadInfo newBlockHeader;
        newBlockHeader.blockType = REC;
        newBlockHeader.pblock = -1;
        newBlockHeader.lblock = (prevBlockNum == -1) ? -1 : prevBlockNum;
        newBlockHeader.rblock = -1;
        newBlockHeader.numEntries = 0;
        newBlockHeader.numSlots = numOfSlots;
        newBlockHeader.numAttrs = numOfAttributes;
        newRecBlock.setHeader(&newBlockHeader);
        
        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
        unsigned char slotMap[numOfSlots];
        for (int i = 0; i < numOfSlots; i++) {
            slotMap[i] = SLOT_UNOCCUPIED;
        }
        newRecBlock.setSlotMap(slotMap);
        
        if(prevBlockNum != -1)
        {
            RecBuffer prevRecBlock(prevBlockNum);
            struct HeadInfo prevBlockHeader;
            prevRecBlock.getHeader(&prevBlockHeader);
            prevBlockHeader.rblock = rec_id.block;  
            // create a RecBuffer object for prevBlockNum
            // get the header of the block prevBlockNum and
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            prevRecBlock.setHeader(&prevBlockHeader);
            // (use BlockBuffer::setHeader() function)
        }
        else
        {
            relCatEntry.firstBlk = rec_id.block;
            RelCacheTable::setRelCatEntry(relId, &relCatEntry);
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)
        }

        relCatEntry.lastBlk = rec_id.block;
        RelCacheTable::setRelCatEntry(relId, &relCatEntry);
        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
    }

     // create a RecBuffer object for rec_id.block
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer insBuffer(rec_id.block);
    insBuffer.setRecord(record, rec_id.slot);
    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
    unsigned char slotMap[numOfSlots];
    insBuffer.getSlotMap(slotMap);
    slotMap[rec_id.slot] = SLOT_OCCUPIED;
    insBuffer.setSlotMap(slotMap);
    
     // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo head;
    insBuffer.getHeader(&head);
    head.numEntries++;
    insBuffer.setHeader(&head);
   
    relCatEntry.numRecs++;
    RelCacheTable::setRelCatEntry(relId, &relCatEntry);
    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)

    int flag = SUCCESS;
    // Iterate over all the attributes of the relation
    // (let attrOffset be iterator ranging from 0 to numOfAttributes-1)
    for(int attrOffset=0;attrOffset<numOfAttributes;attrOffset++)
    {
        // get the attribute catalog entry for the attribute from the attribute cache
        // (use AttrCacheTable::getAttrCatEntry() with args relId and attrOffset)
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId,attrOffset,&attrCatEntry);

        int root=attrCatEntry.rootBlock;
        // get the root block field from the attribute catalog entry
        if(root!=-1)
        // if index exists for the attribute(i.e. rootBlock != -1)
        {
            /* insert the new record into the attribute's bplus tree using
             BPlusTree::bPlusInsert()*/
            int retVal = BPlusTree::bPlusInsert(relId, attrCatEntry.attrName,
                                                record[attrOffset], rec_id);

            if (retVal == E_DISKFULL) {
                flag = E_INDEX_BLOCKS_RELEASED;
            }
        }
    }

    return flag;
}
/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/
int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    RecId recId;

    /* get the attribute catalog entry from the attribute cache corresponding
    to the relation with Id=relid and with attribute_name=attrName  */
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
    
    // if this call returns an error, return the appropriate error code
    if(ret!=SUCCESS){
        return ret;
    }

    // get rootBlock from the attribute catalog entry
    int rootBlock = attrCatEntry.rootBlock;  
    
    if(rootBlock == -1)
    /* if Index does not exist for the attribute (check rootBlock == -1) */ {

        /* search for the record id (recid) corresponding to the attribute with
           attribute name attrName, with value attrval and satisfying the
           condition op using linearSearch()
        */
       recId = BlockAccess::linearSearch(relId, attrName, attrVal, op);
    }

    else  {
        // (index exists for the attribute)

        /* search for the record id (recid) correspoding to the attribute with
        attribute name attrName and with value attrval and satisfying the
        condition op using BPlusTree::bPlusSearch() */
        recId=BPlusTree::bPlusSearch(relId, attrName, attrVal, op);
    }

    // if there's no record satisfying the given condition (recId = {-1, -1})
    //     return E_NOTFOUND;
    if(recId.block == -1 && recId.slot == -1) {
        return E_NOTFOUND;  // if there's no record satisfying the given condition (recId = {-1, -1})
    }

    /* Copy the record with record id (recId) to the record buffer (record).
       For this, instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(recId.block);
    ret=recBuffer.getRecord(record, recId.slot);
    if(ret!=SUCCESS){
        return ret;
    }
    

    return SUCCESS;
}
int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
    if(strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;  // if the relation to delete is either Relation Catalog or Attribute Catalog
    }
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    Attribute relNameAttr; // (stores relName as type union Attribute)
    strcpy(relNameAttr.sVal, relName);
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", relNameAttr, EQ);
    //  linearSearch on the relation catalog for RelName = relNameAttr
    if(relcatRecId.block == -1 && relcatRecId.slot == -1) {
        return E_RELNOTEXIST;  // relation with name relName does not exist
    }
    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    RecBuffer relCatBlock(relcatRecId.block);
     /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */
    relCatBlock.getRecord(relCatEntryRecord, relcatRecId.slot);
    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    int firstBlock = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;  // get the first record block of the relation
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    int nattrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    /*
     Delete all the record blocks of the relation
    */
   // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1
    while(firstBlock != -1) {
        RecBuffer recBuffer(firstBlock);
        // create a RecBuffer for firstBlock
        struct HeadInfo header;
        recBuffer.getHeader(&header);
        // get the header of the block using RecBuffer.getHeader()
        int nextBlock = header.rblock;  // get the next block from the header
        // release the block using BlockBuffer.releaseBlock()
        recBuffer.releaseBlock();
        firstBlock = nextBlock;  // update firstBlock to nextBlock
    }
    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/
    // reset the searchIndex of the attribute catalog
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numberOfAttributesDeleted = 0;

    while(true) {
        RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        attrCatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)"RelName", relNameAttr, EQ);
         // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if(attrCatRecId.block == -1 && attrCatRecId.slot == -1) {
            break;  // no more attributes found for the relation relName
        }
        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
        RecBuffer attrCatBlock(attrCatRecId.block);
        struct HeadInfo header;
        attrCatBlock.getHeader(&header);
        Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatEntryRecord, attrCatRecId.slot);
        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrCatEntryRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal;  // /* get root block from the record */;
        // (This will be used later to delete any indexes if it exists)
        unsigned char slotMap[header.numSlots];
        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap
        attrCatBlock.getSlotMap(slotMap);
        slotMap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        attrCatBlock.setSlotMap(slotMap);
        
        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
        header.numEntries--;
        attrCatBlock.setHeader(&header);
        

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (header.numEntries == 0) {
            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */
           RecBuffer lblock(header.lblock);
           HeadInfo leftHeader;
           lblock.getHeader(&leftHeader);
           leftHeader.rblock=header.rblock;
           lblock.setHeader(&leftHeader);
            // create a RecBuffer for lblock and call appropriate methods

            if (header.rblock != -1) {
                RecBuffer rblock(header.rblock);
                HeadInfo rightHeader;
                rblock.getHeader(&rightHeader);
                rightHeader.lblock=header.lblock;
                rblock.setHeader(&rightHeader);
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods

            } else {

                relCatBlock.getRecord(relCatEntryRecord, relcatRecId.slot);
                relCatEntryRecord[RELCAT_LAST_BLOCK_INDEX].nVal = header.lblock;
                relCatBlock.setRecord(relCatEntryRecord, relcatRecId.slot);
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            attrCatBlock.releaseBlock();
            // call releaseBlock()
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            BPlusTree::bPlusDestroy(rootBlock);
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
        }
    }


    /*** Delete the entry corresponding to the relation from relation catalog ***/
    RecBuffer relCatBlockToDelete(relcatRecId.block);
    // Fetch the header of Relcat block

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
    HeadInfo relCatHeader;
    relCatBlockToDelete.getHeader(&relCatHeader);
    relCatHeader.numEntries--;
    relCatBlockToDelete.setHeader(&relCatHeader);
    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    unsigned char relCatSlotMap[relCatHeader.numSlots];
    relCatBlockToDelete.getSlotMap(relCatSlotMap);
    relCatSlotMap[relcatRecId.slot] = SLOT_UNOCCUPIED;
    relCatBlockToDelete.setSlotMap(relCatSlotMap);
    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntry);
    relCatEntry.numRecs--;
    RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatEntry);
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    RelCatEntry attrCatEntry;
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &attrCatEntry);
    attrCatEntry.numRecs -= numberOfAttributesDeleted;
    RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &attrCatEntry);
    // i.e., #Records = #Records - numberOfAttributesDeleted

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    return SUCCESS;
}