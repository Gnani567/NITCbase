#include "BlockAccess.h"
#include <cstring>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE],
                                union Attribute attrVal, int op) {
    RecId prevRecId;

    int ret = RelCacheTable::getSearchIndex(relId, &prevRecId);

    if (ret != SUCCESS)
        return {-1, -1};

    int block, slot;

    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        RelCatEntry relCatEntry;

        ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        if (ret != SUCCESS)
            return {-1, -1};

        block = relCatEntry.firstBlk;
        slot = 0;
    }
    else {
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    while (block != -1) {
        RecBuffer recBuffer(block);

        HeadInfo head;
        ret = recBuffer.getHeader(&head);

        if (ret != SUCCESS)
            return {-1, -1};

        unsigned char slotMap[head.numSlots];
        recBuffer.getSlotMap(slotMap);

        if (slot >= head.numSlots) {
            block = head.rblock;
            slot = 0;
            continue;
        }

        if (slotMap[slot] == SLOT_UNOCCUPIED) {
            slot++;
            continue;
        }

        Attribute record[head.numAttrs];

        ret = recBuffer.getRecord(record, slot);
        if (ret != SUCCESS)
            return {-1, -1};

        AttrCatEntry attrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(
            relId, attrName, &attrCatEntry
        );

        if (ret != SUCCESS)
            return {-1, -1};

        int offset = attrCatEntry.offset;

        Attribute recordAttr = record[offset];

        int cmpVal = compareAttrs(
            recordAttr,
            attrVal,
            attrCatEntry.attrType
        );

        if ((op == NE && cmpVal != 0) ||
            (op == LT && cmpVal < 0) ||
            (op == LE && cmpVal <= 0) ||
            (op == EQ && cmpVal == 0) ||
            (op == GT && cmpVal > 0) ||
            (op == GE && cmpVal >= 0)) {

            RecId foundRecId{block, slot};

            RelCacheTable::setSearchIndex(relId, &foundRecId);

            return foundRecId;
        }

        slot++;
    }

    return {-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE],
                                char newName[ATTR_SIZE]) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName;
    strcpy(newRelationName.sVal, newName);

    RecId recId = linearSearch(
        RELCAT_RELID,
        (char *)RELCAT_ATTR_RELNAME,
        newRelationName,
        EQ
    );

    if (recId.block != -1 && recId.slot != -1)
        return E_RELEXIST;

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;
    strcpy(oldRelationName.sVal, oldName);

    RecId relRecId = linearSearch(
        RELCAT_RELID,
        (char *)RELCAT_ATTR_RELNAME,
        oldRelationName,
        EQ
    );

    if (relRecId.block == -1 && relRecId.slot == -1)
        return E_RELNOTEXIST;

    RecBuffer relCatBlock(relRecId.block);

    Attribute record[RELCAT_NO_ATTRS];

    int retVal = relCatBlock.getRecord(
        record,
        relRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    int numAttrs = (int)record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    strcpy(
        record[RELCAT_REL_NAME_INDEX].sVal,
        newName
    );

    retVal = relCatBlock.setRecord(
        record,
        relRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    for (int i = 0; i < numAttrs; i++) {

        RecId attrRecId = linearSearch(
            ATTRCAT_RELID,
            (char *)ATTRCAT_ATTR_RELNAME,
            oldRelationName,
            EQ
        );

        if (attrRecId.block == -1 && attrRecId.slot == -1)
            break;

        RecBuffer attrBlock(attrRecId.block);

        Attribute attrRecord[ATTRCAT_NO_ATTRS];

        retVal = attrBlock.getRecord(
            attrRecord,
            attrRecId.slot
        );

        if (retVal != SUCCESS)
            return retVal;

        strcpy(
            attrRecord[ATTRCAT_REL_NAME_INDEX].sVal,
            newName
        );

        retVal = attrBlock.setRecord(
            attrRecord,
            attrRecId.slot
        );

        if (retVal != SUCCESS)
            return retVal;
    }

    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE],
                                  char oldName[ATTR_SIZE],
                                  char newName[ATTR_SIZE]) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;
    strcpy(relNameAttr.sVal, relName);

    RecId recId = linearSearch(
        RELCAT_RELID,
        (char *)RELCAT_ATTR_RELNAME,
        relNameAttr,
        EQ
    );

    if (recId.block == -1 && recId.slot == -1)
        return E_RELNOTEXIST;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    RecId attrToRenameRecId{-1, -1};

    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    while (true) {

        RecId attrRecId = linearSearch(
            ATTRCAT_RELID,
            (char *)ATTRCAT_ATTR_RELNAME,
            relNameAttr,
            EQ
        );

        if (attrRecId.block == -1 && attrRecId.slot == -1)
            break;

        RecBuffer attrBlock(attrRecId.block);

        int retVal = attrBlock.getRecord(
            attrCatEntryRecord,
            attrRecId.slot
        );

        if (retVal != SUCCESS)
            return retVal;

        if (strcmp(
                attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,
                oldName
            ) == 0) {

            attrToRenameRecId.block = attrRecId.block;
            attrToRenameRecId.slot = attrRecId.slot;
        }

        if (strcmp(
                attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,
                newName
            ) == 0) {

            return E_ATTREXIST;
        }
    }

    if (attrToRenameRecId.block == -1 ||
        attrToRenameRecId.slot == -1) {

        return E_ATTRNOTEXIST;
    }

    RecBuffer attrBlock(attrToRenameRecId.block);

    int retVal = attrBlock.getRecord(
        attrCatEntryRecord,
        attrToRenameRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    strcpy(
        attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,
        newName
    );

    retVal = attrBlock.setRecord(
        attrCatEntryRecord,
        attrToRenameRecId.slot
    );

    if (retVal != SUCCESS)
        return retVal;

    return SUCCESS;
}

int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    if (ret != SUCCESS)
        return ret;

    int blockNum = relCatEntry.firstBlk;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk;
    int numOfAttributes = relCatEntry.numAttrs;

    int prevBlockNum = -1;

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        RecBuffer recBuffer(blockNum);

        HeadInfo head;
        ret = recBuffer.getHeader(&head);

        if (ret != SUCCESS)
            return ret;

        unsigned char slotMap[numOfSlots];

        ret = recBuffer.getSlotMap(slotMap);

        if (ret != SUCCESS)
            return ret;

        for (int i = 0; i < numOfSlots; i++) {
            if (slotMap[i] == SLOT_UNOCCUPIED) {
                rec_id.block = blockNum;
                rec_id.slot = i;
                break;
            }
        }

        if (rec_id.block != -1)
            break;

        prevBlockNum = blockNum;
        blockNum = head.rblock;
    }

    if (rec_id.block == -1) {
        if (relId == RELCAT_RELID)
            return E_MAXRELATIONS;

        RecBuffer newRecBlock;

        ret = newRecBlock.getBlockNum();

        if (ret == E_DISKFULL)
            return E_DISKFULL;

        if (ret < 0)
            return ret;

        rec_id.block = ret;
        rec_id.slot = 0;

        HeadInfo head;
        head.blockType = REC;
        head.pblock = -1;
        head.lblock = prevBlockNum;
        head.rblock = -1;
        head.numEntries = 0;
        head.numAttrs = numOfAttributes;
        head.numSlots = numOfSlots;

        ret = newRecBlock.setHeader(&head);

        if (ret != SUCCESS)
            return ret;

        unsigned char slotMap[numOfSlots];

        for (int i = 0; i < numOfSlots; i++)
            slotMap[i] = SLOT_UNOCCUPIED;

        ret = newRecBlock.setSlotMap(slotMap);

        if (ret != SUCCESS)
            return ret;

        if (prevBlockNum != -1) {
            RecBuffer prevBlock(prevBlockNum);

            HeadInfo prevHead;

            ret = prevBlock.getHeader(&prevHead);

            if (ret != SUCCESS)
                return ret;

            prevHead.rblock = rec_id.block;

            ret = prevBlock.setHeader(&prevHead);

            if (ret != SUCCESS)
                return ret;
        }
        else {
            relCatEntry.firstBlk = rec_id.block;

            ret = RelCacheTable::setRelCatEntry(
                relId,
                &relCatEntry
            );

            if (ret != SUCCESS)
                return ret;
        }

        relCatEntry.lastBlk = rec_id.block;

        ret = RelCacheTable::setRelCatEntry(
            relId,
            &relCatEntry
        );

        if (ret != SUCCESS)
            return ret;
    }

    RecBuffer recBuffer(rec_id.block);

    ret = recBuffer.setRecord(record, rec_id.slot);

    if (ret != SUCCESS)
        return ret;

    unsigned char slotMap[numOfSlots];

    ret = recBuffer.getSlotMap(slotMap);

    if (ret != SUCCESS)
        return ret;

    slotMap[rec_id.slot] = SLOT_OCCUPIED;

    ret = recBuffer.setSlotMap(slotMap);

    if (ret != SUCCESS)
        return ret;

    HeadInfo head;

    ret = recBuffer.getHeader(&head);

    if (ret != SUCCESS)
        return ret;

    head.numEntries++;

    ret = recBuffer.setHeader(&head);

    if (ret != SUCCESS)
        return ret;

    relCatEntry.numRecs++;

    ret = RelCacheTable::setRelCatEntry(
        relId,
        &relCatEntry
    );

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}