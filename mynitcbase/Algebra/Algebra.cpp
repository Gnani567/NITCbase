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
        printf(" %lf |", record[i].nVal);
      }
      else if (attrCatEntry.attrType == STRING) {
        printf(" %s |", record[i].sVal);
      }
    }

    printf("\n");
  }

  return SUCCESS;
}