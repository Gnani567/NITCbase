#include "Schema.h"
#include <cmath>
#include <cstring>
int Schema::openRel(char relName[ATTR_SIZE]) {
  int ret = OpenRelTable::openRel(relName);
    
  // the OpenRelTable::openRel() function returns the rel-id if successful
  // a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
  // error codes will be negative
  if(ret >= 0){
    return SUCCESS;
  }

  //otherwise it returns an error message
  return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE]) {
  if (strcmp(relName, "RELATIONCAT") == 0 || strcmp(relName, "ATTRIBUTECAT") == 0) {
    return E_NOTPERMITTED;
  }

  // this function returns the rel-id of a relation if it is open or
  // E_RELNOTOPEN if it is not. we will implement this later.
  int relId = OpenRelTable::getRelId(relName);

 if (relId<0 || relId>=MAX_OPEN) {
    return E_RELNOTOPEN;
  }

  return OpenRelTable::closeRel(relId);
}
int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]) {
     // if the oldRelName or newRelName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_RELNAME and ATTRCAT_RELNAME)
    if(strcmp(oldRelName, RELCAT_RELNAME) == 0 || strcmp(oldRelName, ATTRCAT_RELNAME) == 0 ||
       strcmp(newRelName, RELCAT_RELNAME) == 0 || strcmp(newRelName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }
    int ret=OpenRelTable::getRelId(oldRelName);
     // if the relation is open
    //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
    //    return E_RELOPEN

    // retVal = BlockAccess::renameRelation(oldRelName, newRelName);
    // return retVal
    if(ret!=E_RELNOTOPEN){ 
        return E_RELOPEN;
    }
    ret=BlockAccess::renameRelation(oldRelName,newRelName);
    return ret;  
}
int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName) {
    // if the relName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_RELNAME and ATTRCAT_RELNAME)
    if(strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }
    // if the relation is open
        //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
        //    return E_RELOPEN

    // Call BlockAccess::renameAttribute with appropriate arguments.

    // return the value returned by the above renameAttribute() call
    int ret=OpenRelTable::getRelId(relName);
    if(ret!=E_RELNOTOPEN){
        return E_RELOPEN;
    }
    ret=BlockAccess::renameAttribute(relName, oldAttrName, newAttrName);
    return ret;
    
}
int Schema::createRel(char relName[],int nAttrs, char attrs[][ATTR_SIZE],int attrtype[]){

    // declare variable relNameAsAttribute of type Attribute
    // copy the relName into relNameAsAttribute.sVal
    Attribute relNameAsAttribute;
    strcpy(relNameAsAttribute.sVal, relName);
    
    // declare a variable targetRelId of type RecId
    RecId targetRelId{-1, -1};
    // Reset the searchIndex using RelCacheTable::resetSearhIndex()
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    
    // Search the relation catalog (relId given by the constant RELCAT_RELID)
    // for attribute value attribute "RelName" = relNameAsAttribute using
    // BlockAccess::linearSearch() with OP = EQ
    targetRelId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", relNameAsAttribute, EQ);
    
    if(targetRelId.block != -1 && targetRelId.slot != -1) {
        // if a relation with name `relName` already exists  ( linearSearch() does
        //                                                     not return {-1,-1} )
        return E_RELEXIST;
    }

    // compare every pair of attributes of attrNames[] array
    // if any attribute names have same string value,
    //     return E_DUPLICATEATTR (i.e 2 attributes have same value)
    for(int i=0;i<nAttrs;i++){
        for(int j=i+1;j<nAttrs;j++){
            if(strcmp(attrs[i],attrs[j])==0){
                return E_DUPLICATEATTR;
            }
        }
    }
    

    /* declare relCatRecord of type Attribute which will be used to store the
       record corresponding to the new relation which will be inserted
       into relation catalog */
    Attribute relCatRecord[RELCAT_NO_ATTRS];

    // fill relCatRecord fields as given below
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, relNameAsAttribute.sVal);
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal = nAttrs; // Initialize to number of attributes
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal = 0; // Initialize to 0
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal = -1; // Initialize to -1
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal = -1; // Initialize to -1
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = floor((2016 / (16 * nAttrs + 1))); // Calculate and initialize
    // (number of slots is calculated as specified in the physical layer docs)

    int retVal = BlockAccess::insert(RELCAT_RELID, relCatRecord);
    // retVal = BlockAccess::insert(RELCAT_RELID(=0), relCatRecord);
    if(retVal != SUCCESS) {
        return retVal;  // if BlockAccess::insert fails return retVal
    }
    // if BlockAccess::insert fails return retVal
    // (this call could fail if there is no more space in the relation catalog)
    for(int i=0;i<nAttrs;i++){
        /* declare attrCatRecord of type Attribute which will be used to store the
           record corresponding to the new attribute which will be inserted
           into attribute catalog */
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

        // fill attrCatRecord fields as given below
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relNameAsAttribute.sVal);
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrs[i]);
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrtype[i];
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = -1; // Initialize to -1
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal = -1; // Initialize to -1
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal = i; // Initialize to i

        retVal = BlockAccess::insert(ATTRCAT_RELID, attrCatRecord);
        // retVal = BlockAccess::insert(ATTRCAT_RELID(=1), attrCatRecord);
        if(retVal != SUCCESS) {
            BlockAccess::deleteRelation(relName);  // delete the relation by calling deleteRel(targetrel)
            return E_DISKFULL;  // return E_DISKFULL
            // (this is necessary because we had already created the
            //  relation catalog entry which needs to be removed)
        }
    }

    return SUCCESS;
}
int Schema::deleteRel(char *relName) {
     // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
    // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_RELNAME and ATTRCAT_RELNAME)
    if(strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }
    // get the rel-id using appropriate method of OpenRelTable class by
    // passing relation name as argument

    // if relation is opened in open relation table, return E_RELOPEN
    int ret=OpenRelTable::getRelId(relName);
    if(ret!=E_RELNOTOPEN){
        return E_RELOPEN;
    }
    
    ret=BlockAccess::deleteRelation(relName);
    // Call BlockAccess::deleteRelation() with appropriate argument.
    return ret;
    // return the value returned by the above deleteRelation() call

    /* the only that should be returned from deleteRelation() is E_RELNOTEXIST.
       The deleteRelation call may return E_OUTOFBOUND from the call to
       loadBlockAndGetBufferPtr, but if your implementation so far has been
       correct, it should not reach that point. That error could only occur
       if the BlockBuffer was initialized with an invalid block number.
    */
}
int Schema::createIndex(char relName[ATTR_SIZE],char attrName[ATTR_SIZE])
{
    if(strcmp(relName,RELCAT_RELNAME)==0 || strcmp(relName,ATTRCAT_RELNAME)==0){
        return E_NOTPERMITTED;
    }
    int relId=OpenRelTable::getRelId(relName);
    if(relId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }

    return BPlusTree::bPlusCreate(relId, attrName);
}
int Schema::dropIndex(char *relName, char *attrName) {

    if(strcmp(relName,RELCAT_RELNAME)==0 || strcmp(relName,ATTRCAT_RELNAME)==0){
        return E_NOTPERMITTED;
    }

    int relId=OpenRelTable::getRelId(relName);
    if(relId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);
    // get the attribute catalog entry corresponding to the attribute
    // using AttrCacheTable::getAttrCatEntry()
    if(ret==E_ATTRNOTEXIST){
        return E_ATTRNOTEXIST;
    }

    int rootBlock = attrCatEntry.rootBlock;

    if (rootBlock==-1) {
        return E_NOINDEX;
    }

    BPlusTree::bPlusDestroy(rootBlock);

    attrCatEntry.rootBlock=-1;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);
    // set rootBlock = -1 in the attribute cache entry of the attribute using
    // AttrCacheTable::setAttrCatEntry()

    return SUCCESS;
}

