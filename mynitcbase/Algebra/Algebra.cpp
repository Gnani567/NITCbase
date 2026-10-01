#include "Algebra.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

bool isNumber(char *str) {
  int len;
  float ignore;

  int ret = sscanf(str, "%f %n", &ignore, &len);

  return ret == 1 && len == strlen(str);
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]) {

  if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return E_NOTPERMITTED;
  }

  int relId = OpenRelTable::getRelId(relName);

  if (relId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  // get the relation catalog entry from relation cache
  RelCatEntry relCatEntry;

  int ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);

  if (ret != SUCCESS) {
    return ret;
  }

  /* if relCatEntry.numAttrs != numberOfAttributes in relation,
     return E_NATTRMISMATCH */

  if (relCatEntry.numAttrs != nAttrs) {
    return E_NATTRMISMATCH;
  }

  // let recordValues[numberOfAttributes] be an array of type union Attribute
  Attribute recordValues[nAttrs];

  /*
      Converting 2D char array of record values to Attribute array recordValues
  */

  // iterate through 0 to nAttrs-1
  for (int i = 0; i < nAttrs; i++) {

    // get the attr-cat entry for the i'th attribute from the attr-cache
    AttrCatEntry attrCatEntry;

    ret = AttrCacheTable::getAttrCatEntry(relId,i, &attrCatEntry);

    if (ret != SUCCESS) {
      return ret;
    }

    // let type = attrCatEntry.attrType
    int type = attrCatEntry.attrType;

    if (type == NUMBER) {

      // if the char array record[i] can be converted to a number
      if (isNumber(record[i])) {

        /* convert the char array to numeral and store it
           at recordValues[i].nVal using atof() */
        recordValues[i].nVal = atof(record[i]);
      }
      else {

        return E_ATTRTYPEMISMATCH;
      }

    }
    else if (type == STRING) {

      // copy record[i] to recordValues[i].sVal
      strcpy(recordValues[i].sVal, record[i]);
    }
  }

  // insert the record by calling BlockAccess::insert() function
  int retVal = BlockAccess::insert(relId, recordValues);

  return retVal;
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {

  int srcRelId = OpenRelTable::getRelId(srcRel);

  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  AttrCatEntry attrCatEntry;

  // Get the attribute on which the condition is applied
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);

  if (ret != SUCCESS) {
    return E_ATTRNOTEXIST;
  }


  /*** Convert strVal to an Attribute of type NUMBER or STRING ***/

  int type = attrCatEntry.attrType;
  Attribute attrVal;

  if (type == NUMBER) {

    if (isNumber(strVal)) {
      attrVal.nVal = atof(strVal);
    }
    else {
      return E_ATTRTYPEMISMATCH;
    }

  }
  else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }


  /*** Selecting records from the source relation ***/

  // Start searching from the beginning
  ret = RelCacheTable::resetSearchIndex(srcRelId);

  if (ret != SUCCESS) {
    return ret;
  }


  RelCatEntry relCatEntry;

  // Get relation catalog entry
  ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

  if (ret != SUCCESS) {
    return ret;
  }


  /************************
   * Print attribute names
   ************************/

  printf("|");

  for (int i = 0; i < relCatEntry.numAttrs; ++i) {

    AttrCatEntry attrCatEntry;

    ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

    if (ret != SUCCESS) {
      return ret;
    }

    printf(" %s |", attrCatEntry.attrName);
  }

  printf("\n");


  /************************
   * Search and print records
   ************************/

  while (true) {

    RecId searchRes =
        BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

    // No more matching records
    if (searchRes.block == -1 || searchRes.slot == -1) {
      break;
    }

    // Allocate space for one complete record
    Attribute record[relCatEntry.numAttrs];

    // Get the matching record
    RecBuffer recBuffer(searchRes.block);

    ret = recBuffer.getRecord(record, searchRes.slot);

    if (ret != SUCCESS) {
      return ret;
    }


    // Print the record
    printf("|");

    for (int i = 0; i < relCatEntry.numAttrs; ++i) {

      AttrCatEntry attrCatEntry;

      ret = AttrCacheTable::getAttrCatEntry(
          srcRelId, i, &attrCatEntry
      );

      if (ret != SUCCESS) {
        return ret;
      }

      if (attrCatEntry.attrType == NUMBER) {
        printf(" %0.lf |", record[i].nVal);
      }
      else if (attrCatEntry.attrType == STRING) {
        printf(" %s |", record[i].sVal);
      }
    }

    printf("\n");
  }

  return SUCCESS;
}