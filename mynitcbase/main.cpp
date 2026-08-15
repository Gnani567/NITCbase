#include <iostream>
#include <cstring>

#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  // Create buffers for relation catalog and attribute catalog
  RecBuffer relcatbuffer(RELCAT_BLOCK);
  RecBuffer attrcatbuffer(ATTRCAT_BLOCK);

  HeadInfo relcatheader;
  HeadInfo attrcatheader;

  // Get relation catalog header
  int ret = relcatbuffer.getHeader(&relcatheader);

  if (ret != SUCCESS)
  {
    printf("Error getting relation catalog header\n");
    return ret;
  }

  // Get attribute catalog header
  ret = attrcatbuffer.getHeader(&attrcatheader);

  if (ret != SUCCESS)
  {
    printf("Error getting attribute catalog header\n");
    return ret;
  }

  // Traverse all relation catalog entries
  for (int i = 0; i < relcatheader.numEntries; i++) {

    Attribute relcatrecord[RELCAT_NO_ATTRS];

    ret = relcatbuffer.getRecord(relcatrecord, i);

    if (ret != SUCCESS) {
      printf("Error getting relation catalog record\n");
      return ret;
    }

    printf("Relation : %s\n",relcatrecord[RELCAT_REL_NAME_INDEX].sVal);

    HeadInfo tempheader;

    int curr_block = ATTRCAT_BLOCK;

    // Traverse all blocks of the attribute catalog
    while (curr_block != -1) {

      RecBuffer tempbuffer(curr_block);

      ret = tempbuffer.getHeader(&tempheader);

      if (ret != SUCCESS) {
        printf("Error getting attribute catalog block header\n");
        return ret;
      }

      for (int j = 0; j < tempheader.numEntries; j++) {

        Attribute attrcatrecord[ATTRCAT_NO_ATTRS];

        ret = tempbuffer.getRecord(attrcatrecord, j);

        if (ret != SUCCESS) {
          printf("Error getting attribute catalog record\n");
          return ret;
        }

        if (strcmp(relcatrecord[RELCAT_REL_NAME_INDEX].sVal,attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0) {

          if (strcmp(attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal,"Students") == 0 &&strcmp(attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class") == 0) {
            
            strcpy( attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch");

            // Write the modified record back
            ret = tempbuffer.setRecord(attrcatrecord, j);

            if (ret != SUCCESS) {
              printf("Error setting attribute catalog record\n");
              return ret;
            }
          }

          const char *attrtype = attrcatrecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == 0.0  ? "NUM": "STR";

          printf("  %s: %s\n",attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrtype);
        }
      }

      // Move to the next attribute catalog block
      curr_block = tempheader.rblock;
    }

    printf("\n");
  }

  /*
  for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
  */

  /*
  for(int i=0;i<MAX_OPEN;i++)
  {
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(i,&relCatEntry);

    if (ret != SUCCESS) 
    {
      printf("Error getting relation catalog entry for rel-id %d\n", i);
      return ret;
    }

    printf("Relation: %s\n", relCatEntry.relName);

    // Get all attribute catalog entries for this relation
    for (int j = 0; j < relCatEntry.numAttrs; j++)
    {

      AttrCatEntry attrCatEntry;

      ret = AttrCacheTable::getAttrCatEntry(i,j,&attrCatEntry);

      if (ret != SUCCESS) {
        printf("Error getting attribute catalog entry " "for rel-id %d, offset %d\n",i,j);
        return ret;
      }

      const char *attrType = attrCatEntry.attrType == 0? "NUM": "STR";

      printf("  %s: %s\n",attrCatEntry.attrName,attrType);
    }
    printf("\n");
  }
*/

  return 0;
}
