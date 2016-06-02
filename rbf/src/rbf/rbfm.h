#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>

#include "pfm.h"

const int INT_SIZE            = 4;
const int REAL_SIZE           = 4;
const int VARCHAR_LENGTH_SIZE = 4;

using namespace std;

// Slot directory structure.

struct SlotDirectoryHeader
{
  unsigned freeSpaceOffset = PAGE_SIZE;
  unsigned recordEntriesNumber = 0;
};

struct SlotDirectoryRecordEntry
{
  char meta = 0; // First bit is deleted, second bit is forwarded (length becomes pageNum and offset becomes slotNum to store an RID)
  unsigned length;
  unsigned offset;
};

typedef SlotDirectoryRecordEntry* SlotDirectory;

// Record ID
struct RID
{
  unsigned pageNum;
  unsigned slotNum;
};


// Attribute
typedef enum { TypeInt = 0, TypeReal, TypeVarChar } AttrType;

typedef unsigned AttrLength;

struct Attribute {
    string   name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};

// Comparison Operator (NOT needed for part 1 of the project)
typedef enum { EQ_OP = 0,  // =
           LT_OP,      // <
           GT_OP,      // >
           LE_OP,      // <=
           GE_OP,      // >=
           NE_OP,      // !=
           NO_OP       // no condition
} CompOp;



/****************************************************************************
The scan iterator is NOT required to be implemented for part 1 of the project 
*****************************************************************************/

# define RBFM_EOF (-1)  // end of a scan operator

// RBFM_ScanIterator is an iteratr to go through records
// The way to use it is like the following:
//  RBFM_ScanIterator rbfmScanIterator;
//  rbfm.open(..., rbfmScanIterator);
//  while (rbfmScanIterator(rid, data) != RBFM_EOF) {
//    process the data;
//  }
//  rbfmScanIterator.close();


class RBFM_ScanIterator {
public:
  RBFM_ScanIterator();
  ~RBFM_ScanIterator() {}

  RC open(FileHandle &fileHandle,
    const vector<Attribute> &recordDescriptor,
    const string &conditionAttribute = "",
    const CompOp compOp = NO_OP,
    const void *value = nullptr,
    const vector<string> &attributeNames = vector<string>());
  // "data" follows the same format as RecordBasedFileManager::insertRecord()
  RC getNextRecord(RID &rid, void *data);
  RC close();
  RC projectAttributes(const vector<Attribute> &recordDescriptor, const vector<string> &attributeNames, void *data, void *projectedData);

private:
  RID current;
  unsigned numPages;
  FileHandle* fileHandle;
  uchar* fileMap;
  string conditionAttribute;
  CompOp compOp;
  const void *value;
  vector<Attribute> recordDescriptor;
  vector<string> attributeNames;
  Attribute attr;

  bool compareAttribute(AttrType attrType, void* data, const void* value);
  bool compareInt(void* data, const void* value);
  bool compareReal(void* data, const void* value);
  bool compareVarChar(void* data, const void* value);
};


class RecordBasedFileManager
{
public:
  static RecordBasedFileManager* instance();

  RC createFile(const string &fileName);
  
  RC destroyFile(const string &fileName);
  
  RC openFile(const string &fileName, FileHandle &fileHandle);
  
  RC closeFile(FileHandle &fileHandle);

  //  Format of the data passed into the function is the following:
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  //  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
  RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);

  RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
  
  // This method will be mainly used for debugging/testing
  RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

/**************************************************************************************************************************************************************
***************************************************************************************************************************************************************
IMPORTANT, PLEASE READ: All methods below this comment (other than the constructor and destructor) are NOT required to be implemented for part 1 of the project
***************************************************************************************************************************************************************
***************************************************************************************************************************************************************/
  RC deleteRecords(FileHandle &fileHandle);

  RC deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid);

  // Assume the rid does not change after update
  RC updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid);

  RC readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data);

  RC reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(FileHandle &fileHandle,
      const vector<Attribute> &recordDescriptor,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RBFM_ScanIterator &rbfm_ScanIterator);


// Extra credit for part 2 of the project, please ignore for part 1 of the project
public:

  RC reorganizeFile(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor);
  RC readRecordAttribute(void* record, const vector<Attribute> &recordDescriptor, const string attributeName, void *&data);
  unsigned getMaxRecordSize(const vector<Attribute> &recordDescriptor);
  unsigned getMaxAttributeSize(const Attribute &attribute);
  RC createRecord(const vector<Attribute> &recordDescriptor, vector<const void*> &attributes, void *&data);
  unsigned getRecordSize(const vector<Attribute> &recordDescriptor, const void *data);

protected:
  RecordBasedFileManager();
  ~RecordBasedFileManager();

private:
  static RecordBasedFileManager *_rbf_manager;
  static PagedFileManager *_pf_manager;

  // Auxiliary methods.

  void newRecordBasedPage(void * page);

  SlotDirectoryHeader getSlotDirectoryHeader(void * page);
  void setSlotDirectoryHeader(void * page, SlotDirectoryHeader slotHeader);

  SlotDirectoryRecordEntry getSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber);
  void setSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber, SlotDirectoryRecordEntry recordEntry);

  unsigned getPageFreeSpaceSize(void * page);
};

#endif
