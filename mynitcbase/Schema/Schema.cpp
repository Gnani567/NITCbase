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