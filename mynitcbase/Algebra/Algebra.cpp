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

// int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
//     // Get source relation id
//     int srcRelId = OpenRelTable::getRelId(srcRel);

//     if (srcRelId == E_RELNOTOPEN)
//         return E_RELNOTOPEN;

//     // Get condition attribute
//     AttrCatEntry attrCatEntry;
//     int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);

//     if (ret != SUCCESS)
//         return E_ATTRNOTEXIST;

//     /*** Convert strVal to an Attribute ***/

//     Attribute attrVal;
//     int type = attrCatEntry.attrType;

//     if (type == NUMBER) {
//         if (isNumber(strVal))
//             attrVal.nVal = atof(strVal);
//         else
//             return E_ATTRTYPEMISMATCH;
//     }
//     else if (type == STRING) {
//         strcpy(attrVal.sVal, strVal);
//     }

//     /*** Create target relation ***/

//     RelCatEntry srcRelCatEntry;

//     ret = RelCacheTable::getRelCatEntry(srcRelId, &srcRelCatEntry);
//     if (ret != SUCCESS)
//         return ret;

//     int src_nAttrs = srcRelCatEntry.numAttrs;

//     char attr_names[src_nAttrs][ATTR_SIZE];
//     int attr_types[src_nAttrs];

//     for (int i = 0; i < src_nAttrs; i++) {
//         AttrCatEntry entry;

//         ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &entry);
//         if (ret != SUCCESS)
//             return ret;

//         strcpy(attr_names[i], entry.attrName);
//         attr_types[i] = entry.attrType;
//     }

//     // Create target relation
//     ret = Schema::createRel(targetRel,src_nAttrs,attr_names,attr_types);

//     if (ret != SUCCESS)
//         return ret;

//     // Open target relation
//     int targetRelId = OpenRelTable::openRel(targetRel);

//     if (targetRelId < 0) {
//         ret = Schema::deleteRel(targetRel);
//         return ret;
//     }

//     /*** Select and insert records ***/

//     Attribute record[src_nAttrs];

//     ret = RelCacheTable::resetSearchIndex(srcRelId);
//     if (ret != SUCCESS) {
//         Schema::closeRel(targetRel);
//         Schema::deleteRel(targetRel);
//         return ret;
//     }

//     while (true) {
//         ret = BlockAccess::search( srcRelId, record, attr, attrVal, op);

//         if (ret != SUCCESS)
//             break;

//         ret = BlockAccess::insert(targetRelId, record);

//         if (ret != SUCCESS) {
//             Schema::closeRel(targetRel);
//             Schema::deleteRel(targetRel);
//             return ret;
//         }
//     }

//     // Close target relation
//     ret = Schema::closeRel(targetRel);
//     if (ret != SUCCESS)
//         return ret;

//     return SUCCESS;
// }

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {
    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    // Get source relation details
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    if (ret != SUCCESS)
        return ret;

    int numAttrs = relCatEntry.numAttrs;

    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

    // Get attribute details
    for (int i = 0; i < numAttrs; i++) {
        AttrCatEntry attrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

        if (ret != SUCCESS)
            return ret;

        strcpy(attrNames[i], attrCatEntry.attrName);
        attrTypes[i] = attrCatEntry.attrType;
    }

    /*** Creating and opening the target relation ***/

    ret = Schema::createRel(targetRel,numAttrs,attrNames,attrTypes);

    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if (targetRelId < 0) {
        ret = Schema::deleteRel(targetRel);
        return ret;
    }

    /*** Inserting projected records into the target relation ***/

    ret = RelCacheTable::resetSearchIndex(srcRelId);

    if (ret != SUCCESS) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
    }

    Attribute record[numAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS) {
        ret = BlockAccess::insert(targetRelId, record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close target relation
    ret = Schema::closeRel(targetRel);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {
    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    // Get source relation details
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    if (ret != SUCCESS)
        return ret;

    int src_nAttrs = relCatEntry.numAttrs;

    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];

    /*** Check target attributes ***/

    for (int i = 0; i < tar_nAttrs; i++) {
        AttrCatEntry attrCatEntry;

        ret = AttrCacheTable::getAttrCatEntry(
            srcRelId,
            tar_Attrs[i],
            &attrCatEntry
        );

        if (ret != SUCCESS)
            return E_ATTRNOTEXIST;

        attr_offset[i] = attrCatEntry.offset;
        attr_types[i] = attrCatEntry.attrType;
    }

    /*** Creating and opening target relation ***/

    ret = Schema::createRel(targetRel,tar_nAttrs,tar_Attrs,attr_types);

    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if (targetRelId < 0) {
        ret = Schema::deleteRel(targetRel);
        return ret;
    }

    /*** Insert projected records ***/

    ret = RelCacheTable::resetSearchIndex(srcRelId);

    if (ret != SUCCESS) {
        Schema::closeRel(targetRel);
        Schema::deleteRel(targetRel);
        return ret;
    }

    Attribute record[src_nAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS) {
        Attribute proj_record[tar_nAttrs];

        for (int i = 0; i < tar_nAttrs; i++)
            proj_record[i] = record[attr_offset[i]];

        ret = BlockAccess::insert(targetRelId, proj_record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close target relation
    ret = Schema::closeRel(targetRel);

    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}