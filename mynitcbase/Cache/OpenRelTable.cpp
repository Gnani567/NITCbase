#include "OpenRelTable.h"
#include <cstdlib>
#include <cstring>

OpenRelTable::OpenRelTable() {

    // Initialize relation cache and attribute cache
    for(int i = 0; i < MAX_OPEN; i++) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /**** Setting up Relation Cache entries ****/
    /** Relation Catalog **/
    RecBuffer relCatBlock(RELCAT_BLOCK); 
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord,RELCAT_SLOTNUM_FOR_RELCAT);

    //converts the raw Attribute[] representation into the more convenient:RelCatEntry
    RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord,&relCacheEntry.relCatEntry);
    relCacheEntry.dirty = false;
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
    relCacheEntry.searchIndex.block = -1;
    relCacheEntry.searchIndex.slot = -1;

    RelCacheTable::relCache[RELCAT_RELID]=(RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;


    /** Attribute Catalog **/
    //Read the catalog record of ATTRCAT from the Relation Catalog (RELCAT) 
    //and store that record in relCatRecord.
    relCatBlock.getRecord(relCatRecord,RELCAT_SLOTNUM_FOR_ATTRCAT);
    RelCacheEntry attrRelCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord,&attrRelCacheEntry.relCatEntry);
    attrRelCacheEntry.dirty = false;
    attrRelCacheEntry.recId.block = RELCAT_BLOCK;
    attrRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
    attrRelCacheEntry.searchIndex.block = -1;
    attrRelCacheEntry.searchIndex.slot = -1;

    RelCacheTable::relCache[ATTRCAT_RELID]=(RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) =attrRelCacheEntry;

    /**** Setting up Attribute Cache entries ****/
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    /** Attributes of Relation Catalog **/
    AttrCacheEntry *head = nullptr;
    AttrCacheEntry *prev = nullptr;
    for(int i = 0; i < RELCAT_NO_ATTRS; i++) {
        attrCatBlock.getRecord(attrCatRecord, i);
        AttrCacheEntry *entry =(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);
        entry->dirty = false;
        entry->recId.block = ATTRCAT_BLOCK;
        entry->recId.slot = i;
        entry->searchIndex.block = -1;
        entry->searchIndex.index = -1;
        entry->next = nullptr;

        if(head == nullptr) {
            head = entry;
        }
        else {
            prev->next = entry;
        }
        prev = entry;
    }
    
    AttrCacheTable::attrCache[RELCAT_RELID] = head;

    /** Attributes of Attribute Catalog **/

    head = nullptr;
    prev = nullptr;
    for(int i = RELCAT_NO_ATTRS;i < ATTRCAT_NO_ATTRS + RELCAT_NO_ATTRS;i++) {
        attrCatBlock.getRecord(attrCatRecord, i);
        AttrCacheEntry *entry =(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);
        entry->dirty = false;
        entry->recId.block = ATTRCAT_BLOCK;
        entry->recId.slot = i;
        entry->searchIndex.block = -1;
        entry->searchIndex.index = -1;
        entry->next = nullptr;

        if(head == nullptr) {
            head = entry;
        }
        else {
            prev->next = entry;
        }
        prev = entry;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;
    
    
    
        //*****students relation*****/
    //RecBuffer relCatBlock(RELCAT_BLOCK);
    //Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, 2);
    //RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = 2;
    relCacheEntry.dirty = false;
    relCacheEntry.searchIndex.block = -1;
    relCacheEntry.searchIndex.slot = -1;

    RelCacheTable::relCache[2] =(RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *RelCacheTable::relCache[2] = relCacheEntry;

    //RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    //AttrCacheEntry *head = nullptr;
    //AttrCacheEntry *tail = nullptr;
    head = nullptr;
    prev = nullptr;
    for(int i = 0; i < relCacheEntry.relCatEntry.numAttrs; i++) {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, 12 + i);
        AttrCacheEntry *entry =(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);
        entry->dirty = false;
        entry->recId.block = ATTRCAT_BLOCK;
        entry->recId.slot = 12 + i;
        entry->searchIndex.block = -1;
        entry->searchIndex.index = -1;
        entry->next = nullptr;
        if(head == nullptr) {
                head = entry;
            }
            else {
                prev->next = entry;
            }
            prev = entry;
    }
    AttrCacheTable::attrCache[2] = head;
    }


    OpenRelTable::~OpenRelTable() {
        /**** Free Relation Cache entries ****/
        for(int i = 0; i < MAX_OPEN; i++) {
            if(RelCacheTable::relCache[i] != nullptr) {
                free(RelCacheTable::relCache[i]);
                RelCacheTable::relCache[i] = nullptr;
            }
        }    
        /**** Free Attribute Cache linked lists ****/
        for(int i = 0; i < MAX_OPEN; i++) {
            AttrCacheEntry *entry =AttrCacheTable::attrCache[i];
            while(entry != nullptr) {
                AttrCacheEntry *next = entry->next;
                free(entry);
                entry = next;
            }
            AttrCacheTable::attrCache[i] = nullptr;
        }
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  if (strcmp(relName, RELCAT_RELNAME) == 0) {
    return RELCAT_RELID;
  }

  if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return ATTRCAT_RELID;
  }

  if (strcmp(relName, "Students") == 0) {
    return 2;
  }

  return E_RELNOTOPEN;
}
