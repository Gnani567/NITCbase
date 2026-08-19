#include "AttrCacheTable.h"

#include <cstring>

AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

/* returns the attrOffset-th attribute for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if ( relId<0 || relId >= MAX_OPEN )
  {
    return E_OUTOFBOUND;
  }
  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true

  if ( attrCache[relId] == nullptr)
  {
    return E_RELNOTOPEN;
  }

  // traverse the linked list of attribute cache entries
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    if (entry->attrCatEntry.offset == attrOffset) {

      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf = entry->attrCatEntry;

      return SUCCESS;
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}

/* Converts a attribute catalog record to AttrCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct AttrCatEntry type.
*/
void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],AttrCatEntry* attrCatEntry) {
 // relation name
  strcpy(attrCatEntry->relName,record[ATTRCAT_REL_NAME_INDEX].sVal);

  // attribute name
  strcpy( attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);

  // attribute type
  attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;

  // attribute offset
  attrCatEntry->offset = (int)record[ATTRCAT_OFFSET_INDEX].nVal;
}

/* returns the attribute with name `attrName` for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {

  // Check that relId is valid
  if (relId < 0 || relId >= MAX_OPEN)
  {
    return E_OUTOFBOUND;
  }

  // Check that the relation is open
  if (attrCache[relId] == nullptr)
  {
    return E_RELNOTOPEN;
  }

  // Traverse the linked list of attributes
  for (AttrCacheEntry* entry = attrCache[relId];entry != nullptr;entry = entry->next)
  {

    // Check whether the attribute name matches
    if (strcmp(entry->attrCatEntry.attrName, attrName) == 0)
    {

      // Copy the attribute catalog entry
      *attrCatBuf = entry->attrCatEntry;

      return SUCCESS;
    }
  }

  // Attribute not found
  return E_ATTRNOTEXIST;
}

