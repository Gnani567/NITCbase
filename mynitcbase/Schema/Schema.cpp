#include "Schema.h"

#include <cmath>
#include <cstring>

int Schema::openRel(char relName[ATTR_SIZE]) {
    int ret = OpenRelTable::openRel(relName);

    // If openRel() returns a valid relation ID, opening succeeded
    if (ret >= 0) {
        return SUCCESS;
    }

    // Otherwise return the error code
    return ret;
}


int Schema::closeRel(char relName[ATTR_SIZE]) {

    // RELCAT and ATTRCAT cannot be closed
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }

    // Get the relation ID
    int relId = OpenRelTable::getRelId(relName);

    // Check whether the relation is open
    if (relId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }

    // Close the relation
    return OpenRelTable::closeRel(relId);
}

int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]) {

    // Cannot rename RELATIONCAT or ATTRIBUTECAT
    if (strcmp(oldRelName, RELCAT_RELNAME) == 0 ||
        strcmp(oldRelName, ATTRCAT_RELNAME) == 0 ||
        strcmp(newRelName, RELCAT_RELNAME) == 0 ||
        strcmp(newRelName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }

    // Check if the relation is already open
    int relId = OpenRelTable::getRelId(oldRelName);

    if (relId != E_RELNOTOPEN) {
        return E_RELOPEN;
    }

    // Rename the relation
    int retVal = BlockAccess::renameRelation(oldRelName, newRelName);

    return retVal;
}

int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName) {

    // Cannot rename attributes of RELATIONCAT or ATTRIBUTECAT
    if (strcmp(relName, RELCAT_RELNAME) == 0 ||
        strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }

    // Check if the relation is already open
    int relId = OpenRelTable::getRelId(relName);

    if (relId != E_RELNOTOPEN) {
        return E_RELOPEN;
    }

    // Rename the attribute
    int retVal = BlockAccess::renameAttribute(
        relName,
        oldAttrName,
        newAttrName
    );

    return retVal;
}


int Schema::createRel(char relName[], int nAttrs,char attrs[][ATTR_SIZE], int attrtype[]) {

    Attribute relNameAsAttribute;
    strcpy(relNameAsAttribute.sVal, relName);

    RecId targetRelId;

    // Check if relation exists
    int retVal = RelCacheTable::resetSearchIndex(RELCAT_RELID);

    if (retVal != SUCCESS)
        return retVal;

    char attrName[ATTR_SIZE];
    strcpy(attrName, "RelName");

    targetRelId = BlockAccess::linearSearch(RELCAT_RELID, attrName, relNameAsAttribute, EQ);

    if (targetRelId.block != -1 && targetRelId.slot != -1)
        return E_RELEXIST;

    // Check duplicate attributes
    for (int i = 0; i < nAttrs; i++) {
        for (int j = i + 1; j < nAttrs; j++) {
            if (strcmp(attrs[i], attrs[j]) == 0)
                return E_DUPLICATEATTR;
        }
    }

    // Create relation catalog record
    Attribute relCatRecord[RELCAT_NO_ATTRS];

    strcpy( relCatRecord[RELCAT_REL_NAME_INDEX].sVal, relName);

    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal = nAttrs;
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal = 0;
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = 2016 / (16 * nAttrs + 1);

    retVal = BlockAccess::insert( RELCAT_RELID, relCatRecord);

    if (retVal != SUCCESS)
        return retVal;

    // Create attribute catalog records
    for (int i = 0; i < nAttrs; i++) {

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

        strcpy( attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relName);

        strcpy( attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrs[i]);

        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrtype[i];
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = -1;
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal = -1;
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal = i;

        retVal = BlockAccess::insert( ATTRCAT_RELID, attrCatRecord);

        if (retVal != SUCCESS) {
            Schema::deleteRel(relName);
            return E_DISKFULL;
        }
    }

    return SUCCESS;
}

int Schema::deleteRel(char *relName) {

    if (strcmp(relName, RELCAT_RELNAME) == 0 ||strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }

    int relId = OpenRelTable::getRelId(relName);

    // Relation must be closed
    if (relId >= 0 && relId < MAX_OPEN)
        return E_RELOPEN;

    return BlockAccess::deleteRelation(relName);
}