#include "OpenRelTable.h"
#include<stdlib.h>
#include <cstring>
OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    OpenRelTable::tableMetaInfo[i].free=true;
  }
  

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK); 

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, 0);  // gives the relName 

  struct RelCacheEntry relCacheEntry; // making entry in relcache i.e RelCacheEntry consists of RelCatEntry, recId, searchindex and dirty
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry); // converting a record into a valid relcache entry
  //  record id
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot =0;
  relCacheEntry.searchIndex = {-1, -1}; 
relCacheEntry.dirty = false;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry)); // allocate memory to hold one RelCacheEntry 
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry; //points to that  memory address of RelCache entry // it says permament until freed

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
  // relCache[1]
  Attribute attrCatRecord[RELCAT_NO_ATTRS];
   relCatBlock.getRecord(attrCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
  struct RelCacheEntry attrCacheEntry;
  RelCacheTable::recordToRelCatEntry(attrCatRecord, &attrCacheEntry.relCatEntry);
  attrCacheEntry.recId.block = RELCAT_BLOCK;
  attrCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
  attrCacheEntry.searchIndex = {-1, -1};
  attrCacheEntry.dirty = false;
  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCacheEntry;

  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  struct AttrCacheEntry *head=nullptr;
  for(int i=0;i<6;i++){    // traversing through relation catalog attributes
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  attrCatBlock.getRecord(attrCatRecord,i);
  struct AttrCacheEntry *attr=(struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  attr->next=nullptr;
  attr->dirty=false;
  attr->searchIndex={-1,-1};
  attr->recId.block=ATTRCAT_BLOCK;
  attr->recId.slot=i;
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attr->attrCatEntry); // fills the fields of six attributes
  struct AttrCacheEntry *x=head;
  while(x!=nullptr && x->next!=nullptr){
    x=x->next;
  }
  if(x!=nullptr){
    x->next=attr;
  }
  else{
    head=attr;
  }
  
}
 AttrCacheTable::attrCache[RELCAT_RELID] = head; //attrCache[0]-> attributes of Relation catalog
  // set the next field in the last entry to nullptr

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/
  head=nullptr;
  for(int i=6;i<12;i++){
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  attrCatBlock.getRecord(attrCatRecord,i);
  struct AttrCacheEntry *attr=(struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  attr->next=nullptr;
  attr->dirty=false;
  attr->searchIndex={-1,-1};
  attr->recId.block=ATTRCAT_BLOCK;
  attr->recId.slot=i;
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&(attr->attrCatEntry));
  struct AttrCacheEntry *x=head;
  while(x!=nullptr && x->next!=nullptr){
    x=x->next;
  }
  if(x!=nullptr){
    x->next=attr;
  }
  else{
    head=attr;
  }
  
  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
}
 AttrCacheTable::attrCache[ATTRCAT_RELID] = head;


  tableMetaInfo[RELCAT_RELID].free=false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, "RELATIONCAT");
  tableMetaInfo[ATTRCAT_RELID].free=false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, "ATTRIBUTECAT");
 head=nullptr;
  }

  int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

    int ret=OpenRelTable::getRelId(relName);
    if(ret!=E_RELNOTOPEN){
      return ret;
    }
    ret=OpenRelTable::getFreeOpenRelTableEntry();
  if(ret==E_CACHEFULL){
    return E_CACHEFULL;
  }
  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */

  // let relId be used to store the free slot.
  int relId;
  relId=ret;

  /****** Setting up Relation Cache entry for the relation ******/
  // set the search index to {-1, -1}
  
  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId;
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute AttrName;
  strcpy(AttrName.sVal, relName);
  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", AttrName,EQ);

  if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  RecBuffer relCatBlock(relcatRecId.block);
  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, relcatRecId.slot);
  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block=relcatRecId.block;
  relCacheEntry.recId.slot=relcatRecId.slot;
  relCacheEntry.searchIndex={-1,-1};
  relCacheEntry.dirty=false;
  RelCacheTable::relCache[relId] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry)); // allocate memory to hold one RelCacheEntry 
  *(RelCacheTable::relCache[relId])=relCacheEntry;
  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  struct AttrCacheEntry* listHead=nullptr;
  while(true)

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)"RelName", AttrName, EQ);
      if(attrcatRecId.block == -1 && attrcatRecId.slot == -1) {
          break;  // no more attributes found for the relation relName
      }
       /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      RecBuffer attrBuf(attrcatRecId.block);
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrBuf.getRecord(attrCatRecord, attrcatRecId.slot);
      struct AttrCacheEntry *entry= (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);
      entry->recId.block = attrcatRecId.block;
      entry->recId.slot = attrcatRecId.slot;
      entry->searchIndex = {-1, -1};
      entry->next=nullptr;
      entry->dirty=false;
      if(listHead == nullptr) {
          listHead = entry;
      } else {
          struct AttrCacheEntry *x = listHead;
          while (x->next != nullptr) {
              x = x->next;
          }
          x->next = entry;
      }
  }
  AttrCacheTable::attrCache[relId] = listHead;
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);
  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}
  
  int OpenRelTable::getFreeOpenRelTableEntry() {
    /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  // if found return the relation id, else return E_CACHEFULL.
    for (int i = 2; i < MAX_OPEN; ++i) {
      if (tableMetaInfo[i].free) {
        return i;
      }
    }
    return E_CACHEFULL;
}
 int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  for(int i=0;i<MAX_OPEN;i++){
    if(!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName,relName)==0){
      return i;
    }
  }
  return E_RELNOTOPEN;
}


int OpenRelTable::closeRel(int relId) {
  if (relId==0 || relId==1) {  // RELCAT_RELID or ATTRCAT_RELID
    return E_NOTPERMITTED;
  }

  if (relId<0 || relId>=MAX_OPEN ) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {  // relation is not open
    return E_RELNOTOPEN;
  }
  if(RelCacheTable::relCache[relId]->dirty) {
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);
    RelCacheTable::relCatEntryToRecord(&relCatEntry, relCatRecord);
    RecBuffer relCatBlock(RelCacheTable::relCache[relId]->recId.block);
    relCatBlock.setRecord(relCatRecord, RelCacheTable::relCache[relId]->recId.slot);
  }
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId] = nullptr;
  
 AttrCacheEntry *entry = AttrCacheTable::attrCache[relId];
while(entry != nullptr){
    if(entry->dirty){
        Attribute rec[ATTRCAT_NO_ATTRS];
        AttrCacheTable::attrCatEntryToRecord(&entry->attrCatEntry, rec);
        RecBuffer attrCatBlock(entry->recId.block);
        attrCatBlock.setRecord(rec, entry->recId.slot);
    }
    AttrCacheEntry *next = entry->next;
    free(entry);
    entry = next;
}
AttrCacheTable::attrCache[relId] = nullptr;
  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  tableMetaInfo[relId].free = true;
  tableMetaInfo[relId].relName[0] = '\0';
  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr

  return SUCCESS;
}

OpenRelTable::~OpenRelTable() {
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }
   /**** Closing the catalog relations in the relation cache ****/

    //releasing the relation cache entry of the attribute catalog
    RelCacheEntry *attrCatEntry = RelCacheTable::relCache[ATTRCAT_RELID];
    if(attrCatEntry!=nullptr &&attrCatEntry->dirty ) {
        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        Attribute attrCatRecord[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&attrCatEntry->relCatEntry, attrCatRecord);
        RecBuffer relCatBlock(attrCatEntry->recId.block);
        relCatBlock.setRecord(attrCatRecord,attrCatEntry->recId.slot);
    }
    if(attrCatEntry!=nullptr){
        free(attrCatEntry);
        RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;
    }
    // free the memory dynamically allocated to this RelCacheEntry
    //releasing the relation cache entry of the relation catalog
    RelCacheEntry *relcatEntry = RelCacheTable::relCache[RELCAT_RELID];
    if(relcatEntry!=nullptr && relcatEntry->dirty) {
      /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */

      Attribute relCatRecord[RELCAT_NO_ATTRS];
      RelCacheTable::relCatEntryToRecord(&relcatEntry->relCatEntry, relCatRecord);
      RecBuffer relCatBlock(relcatEntry->recId.block);
        // declaring an object of RecBuffer class to write back to the buffer

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
      relCatBlock.setRecord(relCatRecord, relcatEntry->recId.slot);
      // free the memory dynamically allocated for this RelCacheEntry
    }
      if(relcatEntry!=nullptr){
        free(relcatEntry);
        RelCacheTable::relCache[RELCAT_RELID] = nullptr;
      }
      
    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
  


    // Free RELCAT attribute cache linked list
    AttrCacheEntry* current =AttrCacheTable::attrCache[RELCAT_RELID];

    while (current != nullptr) {

        AttrCacheEntry* next = current->next;

        free(current);

        current = next;
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = nullptr;


    // Free ATTRCAT attribute cache linked list
    AttrCacheEntry *curr =AttrCacheTable::attrCache[ATTRCAT_RELID];

    while (curr != nullptr) {

        AttrCacheEntry* next = curr->next;

        free(curr);

        curr = next;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = nullptr;

}