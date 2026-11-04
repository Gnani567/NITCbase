#include "Algebra.h"
#include<stdlib.h>
#include <cstring>
#include<stdio.h>
#include "../Buffer/counter.h"
/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/
bool isNumber(char *str);
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
  int srcRelId = OpenRelTable::getRelId(srcRel);      // we'll implement this later
  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
  if (ret == E_ATTRNOTEXIST) {
    return E_ATTRNOTEXIST;
  }

  /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {       // the isNumber() function is implemented below
      attrVal.nVal = atof(strVal);
    } else {
      return E_ATTRTYPEMISMATCH;
    }
  } else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }

  /*** Selecting records from the source relation ***/

  // Before calling the search function, reset the search to start from the first hit
  // using RelCacheTable::resetSearchIndex()

  RelCatEntry relCatEntry;
  ret=RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
  if(ret!=SUCCESS){
    return ret;
  }
  int src_nattrs=relCatEntry.numAttrs;
  // get relCatEntry using RelCacheTable::getRelCatEntry()
  char attr_names[src_nattrs][ATTR_SIZE];
  // let attr_types[src_nattrs] be an array of type int
  int attr_types[src_nattrs];
  /*iterate through 0 to src_nattrs-1 :
        get the i'th attribute's AttrCatEntry using AttrCacheTable::getAttrCatEntry()
        fill the attr_names, attr_types arrays that we declared with the entries
        of corresponding attributes
    */
  for(int i=0;i<src_nattrs;i++){
    AttrCatEntry attrCatEntry;
    ret=AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
    strcpy(attr_names[i], attrCatEntry.attrName);
    attr_types[i]=attrCatEntry.attrType;
  }
    /* Create the relation for target relation by calling Schema::createRel()
       by providing appropriate arguments */
    ret=Schema::createRel(targetRel, src_nattrs, attr_names, attr_types);
    // if the createRel returns an error code, then return that value.
    if(ret!=SUCCESS){
      return ret;
    }
    /* Open the newly created target relation by calling OpenRelTable::openRel()
       method and store the target relid */
    int relID=OpenRelTable::openRel(targetRel);
    if(relID<0 || relID>=MAX_OPEN){
      Schema::deleteRel(targetRel); 
      return relID;
    }
    /* If opening fails, delete the target relation by calling Schema::deleteRel()
       and return the error value returned from openRel() */
    /*** Selecting and inserting records into the target relation ***/
    /* Before calling the search function, reset the search to start from the
       first using RelCacheTable::resetSearchIndex() */

    Attribute record[src_nattrs];
    RelCacheTable::resetSearchIndex(srcRelId);
    AttrCacheTable::resetSearchIndex(srcRelId, attr);
    // read every record that satisfies the condition by repeatedly calling
    // BlockAccess::search() until there are no more records to be read
    count=0;
    while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS) {
      
      ret = BlockAccess::insert(relID, record);
         // if (insert fails) {
        //     close the targetrel(by calling Schema::closeRel(targetrel))
        //     delete targetrel (by calling Schema::deleteRel(targetrel))
        //     return ret;
        // }
      if (ret != SUCCESS) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
      }
       
    }
    Schema::closeRel(targetRel);
    printf("Number of comparisions : %d\n",count);
    // Close the targetRel by calling closeRel() method of schema layer

    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);
     // if srcRel is not open in open relation table, return E_RELNOTOPEN
    if(srcRelId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }
    RelCatEntry relCatEntry;
    // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()
    int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    if(ret != SUCCESS) {
        return ret;
    }
    // get the no. of attributes present in relation from the fetched RelCatEntry.
    int numAttrs = relCatEntry.numAttrs;
    // attrNames and attrTypes will be used to store the attribute names
    // and types of the source relation respectively
    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];
    /*iterate through every attribute of the source relation :
        - get the AttributeCat entry of the attribute with offset.
          (using AttrCacheTable::getAttrCatEntry())
        - fill the arrays `attrNames` and `attrTypes` that we declared earlier
          with the data about each attribute
    */
    for(int i=0;i<numAttrs;i++) {
        AttrCatEntry attrCatEntry;
        // get the AttrCatEntry of the i'th attribute of srcRel using
        // AttrCacheTable::getAttrCatEntry()
        ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        if(ret != SUCCESS) {
            return ret;
        }
        // fill the arrays attrNames and attrTypes with the data about each attribute
        strcpy(attrNames[i], attrCatEntry.attrName);
        attrTypes[i] = attrCatEntry.attrType;
    }

    /*** Creating and opening the target relation ***/
    // Create a relation for target relation by calling Schema::createRel()
    ret=Schema::createRel(targetRel, numAttrs, attrNames, attrTypes);
    // if the createRel returns an error code, then return that value.
    if(ret != SUCCESS) {
        return ret;
    }
     // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid
    int targetRelId = OpenRelTable::openRel(targetRel);
    // If opening fails, delete the target relation by calling Schema::deleteRel() of
    // return the error value returned from openRel().
    if(targetRelId < 0 || targetRelId >= MAX_OPEN) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }
    /*** Inserting projected records into the target relation ***/

    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];


    while (BlockAccess::project(srcRelId, record) == SUCCESS)
    {
        // record will contain the next record

        ret = BlockAccess::insert(targetRelId, record);

        if (ret != SUCCESS) {
            // close the targetrel by calling Schema::closeRel()
            Schema::closeRel(targetRel);
            // delete targetrel by calling Schema::deleteRel()
            Schema::deleteRel(targetRel);
            // return ret;
            return ret;
        }
    }
    Schema::closeRel(targetRel);
    // Close the targetRel by calling Schema::closeRel()

    return SUCCESS;
}
int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);
    // if srcRel is not open in open relation table, return E_RELNOTOPEN

  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }
  RelCatEntry relCatEntry;
  int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
      // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()

  if (ret != SUCCESS) {
    return ret;
  }
      // get the no. of attributes present in relation from the fetched RelCatEntry.

  int src_nAttrs = relCatEntry.numAttrs;
  // declare attr_offset[tar_nAttrs] an array of type int.
    // where i-th entry will store the offset in a record of srcRel for the
    // i-th attribute in the target relation.
  int attr_offset[tar_nAttrs];
  // let attr_types[tar_nAttrs] be an array of type int.
    // where i-th entry will store the type of the i-th attribute in the
    // target relation.
  int attr_types[tar_nAttrs];
    /*** Checking if attributes of target are present in the source relation
         and storing its offsets and types ***/
    /*iterate through 0 to tar_nAttrs-1 :
        - get the attribute catalog entry of the attribute with name tar_attrs[i].
        - if the attribute is not found return E_ATTRNOTEXIST
        - fill the attr_offset, attr_types arrays of target relation from the
          corresponding attribute catalog entries of source relation
    */
    for (int i = 0; i < tar_nAttrs; i++) {
        AttrCatEntry attrCatEntry;
        ret = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &attrCatEntry);
        // get the AttributeCat entry of the attribute with name tar_attrs[i].
        // (using AttrCacheTable::getAttrCatEntry())
        if (ret == E_ATTRNOTEXIST) {
            return E_ATTRNOTEXIST;
        }
        // fill the attr_offset, attr_types arrays of target relation from the
        // corresponding attribute catalog entries of source relation
        attr_offset[i] = attrCatEntry.offset;
        attr_types[i] = attrCatEntry.attrType;
    }
    
    /*** Creating and opening the target relation ***/
    // Create a relation for target relation by calling Schema::createRel()

    ret=Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
    // if the createRel returns an error code, then return that value.

    if (ret != SUCCESS) {
        return ret;
    }
    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid
    int targetRelId = OpenRelTable::openRel(targetRel);
    if(targetRelId < 0 || targetRelId >= MAX_OPEN) {
       // If opening fails, delete the target relation by calling Schema::deleteRel()
    // and return the error value from openRel()
        Schema::deleteRel(targetRel);
        return targetRelId;
    }
    /*** Inserting projected records into the target relation ***/
    RelCacheTable::resetSearchIndex(srcRelId);
    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()

    Attribute record[src_nAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS) {
        // the variable `record` will contain the next record

        Attribute proj_record[tar_nAttrs];
        //iterate through 0 to tar_attrs-1:
        //    proj_record[attr_iter] = record[attr_offset[attr_iter]]
        for(int i = 0; i < tar_nAttrs; i++) {
            proj_record[i] = record[attr_offset[i]];
        }
        ret = BlockAccess::insert(targetRelId, proj_record);
        // ret = BlockAccess::insert(targetRelId, proj_record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
            // close the targetrel by calling Schema::closeRel()
            // delete targetrel by calling Schema::deleteRel()
            // return ret;
        }
    }

    Schema::closeRel(targetRel);
    // Close the targetRel by calling Schema::closeRel()

    return SUCCESS;
}
// will return if a string can be parsed as a floating point number
bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}
int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
  if(strcmp(relName, "RELATIONCAT") == 0 || strcmp(relName, "ATTRIBUTECAT") == 0) {
    // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
    return E_NOTPERMITTED;
  }
    // get the relation's rel-id using OpenRelTable::getRelId() method
    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    int relId = OpenRelTable::getRelId(relName);
    if(relId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }
    RelCatEntry relCatEntry;
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)
    int ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);
    if(ret != SUCCESS) {  
        return ret;
    }
    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */
    if(relCatEntry.numAttrs != nAttrs) {
        return E_NATTRMISMATCH;
    }
    Attribute recordValues[nAttrs]; // let recordValues be an array of type union Attribute
    // let recordValues[numberOfAttributes] be an array of type union Attribute

    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    // iterate through 0 to nAttrs-1: (let i be the iterator)
    for(int i = 0; i < nAttrs; i++) {
      // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())
        AttrCatEntry attrCatEntry;
        ret = AttrCacheTable::getAttrCatEntry(relId, i, &attrCatEntry);
        if(ret != SUCCESS) {
            return ret;
        }
        // let type = attrCatEntry.attrType;
        int type = attrCatEntry.attrType;
        if(type == NUMBER) {
           // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if(isNumber(record[i])) {
              /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */
                recordValues[i].nVal = atof(record[i]);
            } else {
                return E_ATTRTYPEMISMATCH;
            }
        } else if(type == STRING) {
            // copy record[i] to recordValues[i].sVal
            strcpy(recordValues[i].sVal, record[i]);
        }
    }
    int retVal=BlockAccess::insert(relId, recordValues);

    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call

    return retVal;
}
int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE]) {
  // get the srcRelation1's rel-id using OpenRelTable::getRelId() method

    // get the srcRelation2's rel-id using OpenRelTable::getRelId() method

    // if either of the two source relations is not open
    //     return E_RELNOTOPEN
    int srcRelId_1=OpenRelTable::getRelId(srcRelation1);
    int srcRelId_2=OpenRelTable::getRelId(srcRelation2);
    if(srcRelId_1==E_RELNOTOPEN || srcRelId_2==E_RELNOTOPEN){
      return E_RELNOTOPEN;
    }
    
    AttrCatEntry attrCatEntry1, attrCatEntry2;
    // get the attribute catalog entries for the following from the attribute cache
    // (using AttrCacheTable::getAttrCatEntry())
    int retVal1=AttrCacheTable::getAttrCatEntry(srcRelId_1,attribute1,&attrCatEntry1);
    int retVal2=AttrCacheTable::getAttrCatEntry(srcRelId_2,attribute2,&attrCatEntry2);
    // - attrCatEntry1 = attribute1 of srcRelation1
    // - attrCatEntry2 = attribute2 of srcRelation2
    // if attribute1 is not present in srcRelation1 or attribute2 is not
    // present in srcRelation2 (getAttrCatEntry() returned E_ATTRNOTEXIST)
    //     return E_ATTRNOTEXIST.
    if(retVal1==E_ATTRNOTEXIST || retVal2==E_ATTRNOTEXIST){
      return E_ATTRNOTEXIST;
    }
    // if attribute1 and attribute2 are of different types return E_ATTRTYPEMISMATCH
    if(attrCatEntry1.attrType!=attrCatEntry2.attrType){
      return E_ATTRTYPEMISMATCH;
    }
    RelCatEntry relCatEntry1, relCatEntry2;
    retVal1=RelCacheTable::getRelCatEntry(srcRelId_1,&relCatEntry1);
    retVal2=RelCacheTable::getRelCatEntry(srcRelId_2,&relCatEntry2);
    
    // iterate through all the attributes in both the source relations and check if
    // there are any other pair of attributes other than join attributes
    // (i.e. attribute1 and attribute2) with duplicate names in srcRelation1 and
    // srcRelation2 (use AttrCacheTable::getAttrCatEntry())
    // If yes, return E_DUPLICATEATTR
    for(int i=0;i<relCatEntry1.numAttrs;i++){
      AttrCatEntry e1;
      AttrCacheTable::getAttrCatEntry(srcRelId_1,i,&e1);
      for(int j=0;j<relCatEntry2.numAttrs;j++){
        AttrCatEntry e2;
        AttrCacheTable::getAttrCatEntry(srcRelId_2,j,&e2);
        if(strcmp(e1.attrName,e2.attrName)==0 && strcmp(e1.attrName,attribute1)!=0 && strcmp(e2.attrName,attribute2)!=0){
          E_DUPLICATEATTR;
        }
      }
    }
    
    // get the relation catalog entries for the relations from the relation cache
    // (use RelCacheTable::getRelCatEntry() function)

    int numOfAttributes1 = relCatEntry1.numAttrs;
    int numOfAttributes2 = relCatEntry2.numAttrs;

    // if rel2 does not have an index on attr2
    //     create it using BPlusTree:bPlusCreate()
    //     if call fails, return the appropriate error code
    //     (if your implementation is correct, the only error code that will
    //      be returned here is E_DISKFULL)
    if(attrCatEntry2.rootBlock==-1){
      int ret=BPlusTree::bPlusCreate(srcRelId_2,attribute2);
      if(ret!=SUCCESS){
        return ret;
      }
    }
    int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;
    // Note: The target relation has number of attributes one less than
    // nAttrs1+nAttrs2 (Why?)

    // declare the following arrays to store the details of the target relation
    char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
    int targetRelAttrTypes[numOfAttributesInTarget];
    int k=0;
    for(int i=0;i<relCatEntry1.numAttrs;i++){
        AttrCatEntry e3;
        AttrCacheTable::getAttrCatEntry(srcRelId_1,i,&e3);
        strcpy(targetRelAttrNames[k],e3.attrName);
        targetRelAttrTypes[k]=e3.attrType;
        k++;
    }
    // iterate through all the attributes in both the source relations and
    // update targetRelAttrNames[],targetRelAttrTypes[] arrays excluding attribute2
    // in srcRelation2 (use AttrCacheTable::getAttrCatEntry())
    for(int j=0;j<relCatEntry2.numAttrs;j++){
      AttrCatEntry e4;
      AttrCacheTable::getAttrCatEntry(srcRelId_2,j,&e4);
      if(strcmp(e4.attrName,attribute2)==0){
        continue;
      }
      strcpy(targetRelAttrNames[k],e4.attrName);
      targetRelAttrTypes[k]=e4.attrType;
      k++;
    }
    retVal1=Schema::createRel(targetRelation,numOfAttributesInTarget,targetRelAttrNames,targetRelAttrTypes);
    // create the target relation using the Schema::createRel() function
    // if createRel() returns an error, return that error
    if(retVal1!=SUCCESS){
      return retVal1;
    }
    
    retVal2=OpenRelTable::openRel(targetRelation);
    // Open the targetRelation using OpenRelTable::openRel()
    if(retVal2<0 || retVal2>=MAX_OPEN)
    // if openRel() fails (No free entries left in the Open Relation Table)
    {   
        Schema::deleteRel(targetRelation);
        return retVal2;
        // delete target relation by calling Schema::deleteRel()
        // return the error code
    }

    Attribute record1[numOfAttributes1];
    Attribute record2[numOfAttributes2];
    Attribute targetRecord[numOfAttributesInTarget];

    // this loop is to get every record of the srcRelation1 one by one
    while (BlockAccess::project(srcRelId_1, record1) == SUCCESS) {

        RelCacheTable::resetSearchIndex(srcRelId_2);
        // reset the search index of `srcRelation2` in the relation cache
        // using RelCacheTable::resetSearchIndex()
        AttrCacheTable::resetSearchIndex(srcRelId_2,attribute2);
        // reset the search index of `attribute2` in the attribute cache
        // using AttrCacheTable::resetSearchIndex()

        // this loop is to get every record of the srcRelation2 which satisfies
        //the following condition:
        // record1.attribute1 = record2.attribute2 (i.e. Equi-Join condition)
        while (BlockAccess::search(
            srcRelId_2, record2, attribute2, record1[attrCatEntry1.offset], EQ
        ) == SUCCESS ) {
            int idx=0;
            for(int i=0;i<relCatEntry1.numAttrs;i++){
              targetRecord[idx++]=record1[i];
            }
            // copy srcRelation1's and srcRelation2's attribute values(except
            // for attribute2 in rel2) from record1 and record2 to targetRecord
            for(int i=0;i<relCatEntry2.numAttrs;i++){
              if(i==attrCatEntry2.offset){
                continue;
              }
              targetRecord[idx++]=record2[i];
            }
            retVal1=BlockAccess::insert(retVal2,targetRecord);
            // insert the current record into the target relation by calling
            // BlockAccess::insert()

            if(retVal1!=SUCCESS) {

                OpenRelTable::closeRel(retVal2);
                Schema::deleteRel(targetRelation);
                // close the target relation by calling OpenRelTable::closeRel()
                // delete targetRelation (by calling Schema::deleteRel())
                return E_DISKFULL;
            }
        }
    }
    OpenRelTable::closeRel(retVal2);
    // close the target relation by calling OpenRelTable::closeRel()
    return SUCCESS;
}