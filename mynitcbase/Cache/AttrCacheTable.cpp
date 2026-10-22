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

void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS], AttrCatEntry* attrCatEntry) {
    strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
    strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
    attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
    attrCatEntry->primaryFlag = (bool)record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
    attrCatEntry->rootBlock = (int)record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
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

int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
        if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
            *searchIndex = entry->searchIndex;
            return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}

int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
        if (entry->attrCatEntry.offset == attrOffset) {
            *searchIndex = entry->searchIndex;
            return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}

int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
        if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
            entry->searchIndex = *searchIndex;
            return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}

int AttrCacheTable::setSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
        if (entry->attrCatEntry.offset == attrOffset) {
            entry->searchIndex = *searchIndex;
            return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}

int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {
    IndexId searchIndex = {-1, -1};
    return setSearchIndex(relId, attrName, &searchIndex);
}

int AttrCacheTable::resetSearchIndex(int relId, int attrOffset) {
    IndexId searchIndex = {-1, -1};
    return setSearchIndex(relId, attrOffset, &searchIndex);
}