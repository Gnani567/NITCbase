#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include<cstring>

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  return FrontendInterface::handleFrontend(argc, argv);
  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, 7000);
  // char message[] = "hello";
  // memcpy(buffer + 20, message, 6);
  // Disk::writeBlock(buffer, 7000);

  // unsigned char buffer2[BLOCK_SIZE];
  // char message2[6];
  // Disk::readBlock(buffer2, 7000);
  // memcpy(message2, buffer2 + 20, 6);
  // std::cout << message2;
  // for(int i=0;i<4;i++){
  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, i);
  // char message2[2048];
  //  memcpy(message2, buffer, 2048);
  //  for(int i=0;i<2048;i++){
  //   printf("%d,", message2[i]);
  //  }

  // create objects for the relation catalog and attribute catalog
  // RecBuffer relCatBuffer(RELCAT_BLOCK);  // creates an object RecBufffer (Relation catalog)
  // RecBuffer attrCatBuffer(ATTRCAT_BLOCK); // creates an object AttributeBuffer (Attribute catalog)
  
  // HeadInfo relCatHeader; // stores the header of Relation Catalog
  // HeadInfo attrCatHeader; // stores the header of Attribute catalog

  // // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // relCatBuffer.getHeader(&relCatHeader); // read disk block 4
  // attrCatBuffer.getHeader(&attrCatHeader); // read disk block 5

  // RelCatEntry relCatentry;
  // AttrCatEntry attrCatentry;
  // for(int i=0;i<12;i++){
  //   if(RelCacheTable::getRelCatEntry(i,&relCatentry)!=SUCCESS){
  //     continue;
  //   }
  //   printf("Relation: %s\n", relCatentry.relName);
  //   for(int j=0;j<relCatentry.numAttrs;j++){
  //     AttrCacheTable::getAttrCatEntry(i,j,&attrCatentry);
  //     const char *attrType = attrCatentry.attrType== NUMBER ? "NUM" : "STR";
  //     printf(" %s: %s\n", attrCatentry.attrName, attrType);

  //   }
  // }

  /*
  for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
  */

//   for (int i=0;i<relCatHeader.numEntries;i++) { // no of relations in relation catalog

//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // Each Relation has several fields and attributes.

//     relCatBuffer.getRecord(relCatRecord, i); // 0 to 5 each corresponding to RelName, Attributes, Records, first block, last block

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal); // INDEX = 0
//      int curr=ATTRCAT_BLOCK;
//      while(curr!=-1){
//     RecBuffer buffer1(curr);
//     HeadInfo head;
//     buffer1.getHeader(&head);
//     for (int j=0;j<head.numEntries;j++) {

//       Attribute attrCatRecord[ATTRCAT_NO_ATTRS]; // 6

//        buffer1.getRecord(attrCatRecord, j); // 0 to 5 each corresponding to RelName, AttributeName, AttributeType, Primary flag, root block and offset
//       // declare attrCatRecord and load the attribute catalog entry into it
//       if(strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0){
//         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

//         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
//       }
//       if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,"Students")==0 && 
//     strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0) {
//       if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0){
//         strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch");
//         unsigned char buff[BLOCK_SIZE];
//         Disk::readBlock(buff,curr);
//         int recordSize=ATTR_SIZE*head.numAttrs;
//         memcpy(buff+HEADER_SIZE+head.numSlots+recordSize*j,attrCatRecord,recordSize); // we go to the exact location of that particular record 
//         Disk::writeBlock(buff,curr);
        
//       }
//     }
//   }
//     curr=head.rblock;
//     printf("\n");
// }
//   }
  //  for (int i=0;i<relCatHeader.numEntries;i++) { // no of relations in relation catalog

  //   Attribute relCatRecord[RELCAT_NO_ATTRS]; // Each Relation has several fields and attributes.

  //   relCatBuffer.getRecord(relCatRecord, i); // 0 to 5 each corresponding to RelName, Attributes, Records, first block, last block

  //   printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal); // INDEX = 0
  //    int curr=ATTRCAT_BLOCK;
  //    while(curr!=-1){
  //   RecBuffer buffer2(curr);
  //   HeadInfo head;
  //   buffer2.getHeader(&head);
  //   for (int j=0;j<head.numEntries;j++) {

  //     Attribute attrCatRecord[ATTRCAT_NO_ATTRS]; // 6

  //      buffer2.getRecord(attrCatRecord, j); // 0 to 5 each corresponding to RelName, AttributeName, AttributeType, Primary flag, root block and offset
  //     // declare attrCatRecord and load the attribute catalog entry into it

  //     if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0){
  //       const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

  //       printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        
  //     }
  //   }
  //   curr=head.rblock;
  //     printf("\n");
  // }
  //  }
}