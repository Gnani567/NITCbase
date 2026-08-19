#include "BlockAccess.h"

#include <cstring>


RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE],union Attribute attrVal, int op) {

  // Get the previous search position
  RecId prevRecId;
  int ret = RelCacheTable::getSearchIndex(relId, &prevRecId);

  if (ret != SUCCESS) {
    return RecId{-1, -1};
  }

  int block;
  int slot;

  // Start from the first record if there was no previous hit
  if (prevRecId.block == -1 && prevRecId.slot == -1) {

    RelCatEntry relCatEntry;

    ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    if (ret != SUCCESS) {
      return RecId{-1, -1};
    }

    block = relCatEntry.firstBlk;
    slot = 0;
  }
  else {

    // Start from the record immediately after the previous hit
    block = prevRecId.block;
    slot = prevRecId.slot + 1;
  }


  // Search through the relation
  while (block != -1) {

    // Create RecBuffer for the current record block
    RecBuffer recBuffer(block);

    // Get block header
    HeadInfo head;

    ret = recBuffer.getHeader(&head);

    if (ret != SUCCESS) {
      return RecId{-1, -1};
    }

    // If we have reached the end of this block,
    // move to the right block.
    if (slot >= head.numSlots) {
      block = head.rblock;
      slot = 0;
      continue;
    }

    // Allocate slot map for this block
    unsigned char *slotMap = new unsigned char[head.numSlots];

    ret = recBuffer.getSlotMap(slotMap);

    if (ret != SUCCESS) {
      delete[] slotMap;
      return RecId{-1, -1};
    }

    // If this slot is free, move to the next slot
    if (slotMap[slot] == SLOT_UNOCCUPIED) {
      delete[] slotMap;
      slot++;
      continue;
    }

    // Get the current record
    Attribute record[head.numAttrs];

    ret = recBuffer.getRecord(record, slot);

    if (ret != SUCCESS) {
      delete[] slotMap;
      return RecId{-1, -1};
    }

    delete[] slotMap;


    // Get information about the attribute being searched
    AttrCatEntry attrCatEntry;

    ret = AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    if (ret != SUCCESS) {
      return RecId{-1, -1};
    }


    // Get the value of this attribute from the current record
    Attribute recordAttr = record[attrCatEntry.offset];


    // Compare record value with the search value
    int cmpVal = compareAttrs(recordAttr,attrVal,attrCatEntry.attrType);


    // Check whether the condition is satisfied
    if (
        (op == NE && cmpVal != 0) ||
        (op == LT && cmpVal < 0) ||
        (op == LE && cmpVal <= 0) ||
        (op == EQ && cmpVal == 0) ||
        (op == GT && cmpVal > 0) ||
        (op == GE && cmpVal >= 0)
    ) {

      // Update search index to this matching record
      RecId searchIndex = {block, slot};

      ret = RelCacheTable::setSearchIndex(relId,&searchIndex);

      if (ret != SUCCESS) {
        return RecId{-1, -1};
      }

      return searchIndex;
    }

    // Current record didn't match
    slot++;
  }


  // No more records to search.
  // Reset search index so a future search can start again.
  RelCacheTable::resetSearchIndex(relId);

  return RecId{-1, -1};
}