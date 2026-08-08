#include <iostream>
#include <cstring>
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

int main(int argc, char *argv[]) {
  Disk disk_run;

  RecBuffer relcatbuffer(RELCAT_BLOCK);
  RecBuffer attrcatbuffer(ATTRCAT_BLOCK);

  HeadInfo relcatheader;
  HeadInfo attrcatheader;

  relcatbuffer.getHeader(&relcatheader);
  attrcatbuffer.getHeader(&attrcatheader);

  for(int i=0;i<relcatheader.numEntries;i++)
  {
    Attribute relcatrecord[RELCAT_NO_ATTRS];
    relcatbuffer.getRecord(relcatrecord,i);

    printf("Relation : %s\n",relcatrecord[RELCAT_REL_NAME_INDEX].sVal);

    HeadInfo tempheader;
    int curr_block=ATTRCAT_BLOCK;
    while(curr_block!=-1)
    {
      RecBuffer tempbuffer(curr_block);
      tempbuffer.getHeader(&tempheader);
      for(int j=0;j<tempheader.numEntries;j++)
      {
        Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
        tempbuffer.getRecord(attrcatrecord,j);
        if(strcmp(relcatrecord[RELCAT_REL_NAME_INDEX].sVal,attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal)==0)
        {
          if(strcmp(attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0 &&strcmp(attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0)
          {
            strcpy(attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");

            // Write the updated record back to disk
            tempbuffer.setRecord(attrcatrecord, j);
          }
          const char* attrtype=attrcatrecord[ATTRCAT_ATTR_TYPE_INDEX].nVal==0.0?"NUM":"STR";
          printf("  %s: %s\n",attrcatrecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrtype);
        }
      }
      curr_block=tempheader.rblock;
    }
    printf("\n");
  }

  return 0;
}