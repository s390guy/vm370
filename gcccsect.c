/**************************************************************************************************/
/* GCCCSECT                                                                                       */
/* Fix the unlabeled CSECT produced by the GCC compiler.  Creates fn ASMFIX fm.  Disk fm must be  */
/* read/write.                                                                                    */
/*                                                                                                */
/* Usage:  GCCCSECT fn ft fm                                                                      */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, June 2010.                                                 */
/**************************************************************************************************/
#include <cmssys.h>
#include <ctype.h>
#include <string.h>
 
int main(int argc, char * argv[]) {
#define RECLEN 80
char buffer[RECLEN];
char csectName[9];
char readFileid[19];
char writeFileid[19];
CMSFILE readFscb;
CMSFILE writeFscb;
int bytesRead;
int i;
int notyet;
int rc;
int recnum;
 
if (argc != 4) {
   CMSconsoleWrite("Usage is GCCCSECT fn ft fm", CMS_EDIT);
   return 24;
   }
 
// First build the fileid from the argument list.
strcpy(readFileid, argv[1]); strncat(readFileid, "       ", 8 - strlen(readFileid));
strcat(readFileid, argv[2]); strncat(readFileid, "       ", 16 - strlen(readFileid));
strcat(readFileid, argv[3]); strncat(readFileid, " ", 18 - strlen(readFileid));
readFileid[18] = 0;
for (i = 0; i < 18; i++) {                                    // now convert the fileid to uppercase
   readFileid[i] = toupper(readFileid[i]);
   }
memcpy(writeFileid, readFileid, 19);                // build the output fileid from the input fileid
memcpy(&writeFileid[8], "ASMFIX  ", 8);
 
// Open the input and output files.
rc = CMSfileOpen(readFileid, buffer, RECLEN, 'V', 1, 1, &readFscb);
if (rc != 0) {
   CMSconsoleWrite("Unable to open input file.", CMS_EDIT);
   return rc;
   }
rc = CMSfileOpen(writeFileid, buffer, RECLEN, 'F', 1, 1, &writeFscb);
if (!(rc == 0 || rc == 28)) {
   CMSconsoleWrite("Unable to open output file.", CMS_EDIT);
   return rc;
   }
 
// Commence processing in earnest.  Loop to search for the CSECT record, and add a label.
strcpy(csectName, argv[1]);
strncat(csectName, "@@@@@@@@", 8 - strlen(csectName));
for (i = 0; i < 8; i++) csectName[i] = toupper(csectName[i]);
notyet = 1;
rc = 0;
recnum = 1;
rc = CMSfileRead(&readFscb, 0, &bytesRead);
while (rc == 0) {                                                           // loop to read the file
   for (i = bytesRead; i < RECLEN; i++) buffer[i] = ' ';                          // pad with blanks
   if (notyet && (memcmp(&buffer[9], "CSECT", 5) == 0)) {                    // replace CSECT record
      memcpy(buffer, csectName, 8);
      notyet = 0;
      }
   rc = CMSfileWrite(&writeFscb, recnum, RECLEN);             // write the record to the output file
   recnum = 0;
   if (rc > 0) {
      CMSconsoleWrite("Error writing output file.", CMS_EDIT);
      break;
      }
   rc = CMSfileRead(&readFscb, 0, &bytesRead);
   }
if (rc == 12) rc = 0;                                 // rc of 12 means end of file, and all is well
else CMSconsoleWrite("Error reading input file.", CMS_EDIT);
CMSfileClose(&readFscb);
CMSfileClose(&writeFscb);
return rc;
}                                                                                  /* end of main */
