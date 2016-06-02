
#ifndef _ix_h_
#define _ix_h_

#include <iostream>
#include <string>
#include <vector>

#include "../../../rbf/src/rbf/rbfm.h"

// Used for prevPage in the first page and for nextPage in the last one.
const RC NULL_PAGE_ID = -1;

// Return error codes.
const RC ERROR_PFM_CREATE = 1;
const RC ERROR_PFM_DESTROY = 2;
const RC ERROR_PFM_OPEN = 3;
const RC ERROR_PFM_CLOSE = 4;
const RC ERROR_PFM_READPAGE = 5;
const RC ERROR_PFM_WRITEPAGE = 6;
const RC ERROR_PFM_FILEHANDLE = 7;

const RC ERROR_NO_SPACE_AFTER_SPLIT = 8;
const RC ERROR_RECORD_EXISTS = 9;
const RC ERROR_RECORD_NOT_EXISTS = 10;

const RC ERROR_UNKNOWN = -1;

// Internal error.
const RC ERROR_NO_FREE_SPACE = 11;

const int IX_EOF = -1;  // end of the index scan

class IX_ScanIterator;

enum PageType{MetaPage, LeafPage, NonLeafPage}; // leaf and non-leaf pages have different headers

struct LeafPageHeader
{
    unsigned prevPage = 0;
    unsigned nextPage = 0;
    unsigned recordEntriesNumber = 0;
    unsigned freeSpaceOffset = PAGE_SIZE;
};
const unsigned LEAF_PAGE_DATA_OFFSET = sizeof(PageType) + sizeof(LeafPageHeader);

typedef SlotDirectoryHeader NonLeafPageHeader;
const unsigned NON_LEAF_PAGE_DATA_OFFSET = sizeof(PageType) + sizeof(NonLeafPageHeader) + sizeof(unsigned);

struct LeafPageChildEntry
{
    unsigned ridCount; // number of duplicate keys
    void* key;
    RID rid;
};

struct MetaPageHeader
{
    unsigned rootPage;
};

struct ChildEntry
{
    void* key;
    unsigned childPage;

};

class IndexManager
{
 public:
  static IndexManager* instance();
  
  bool isLeafPage(void * pageData);

  RC createFile(const string &fileName);

  RC destroyFile(const string &fileName);

  RC openFile(const string &fileName, FileHandle &fileHandle);

  RC closeFile(FileHandle &fileHandle);
  
  unsigned getRootPageID(FileHandle &fileHandle);
  
  int compareKeys(const Attribute attribute, const void * key1, const void * key2);
  
  RC treeSearch(uchar* pageFile, const Attribute attribute, const void * key, unsigned currentPageID, unsigned &returnPageID);

  // The following two functions are using the following format for the passed key value.
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  RC insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Insert new index entry
  RC deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Delete index entry

  // scan() returns an iterator to allow the caller to go through the results
  // one by one in the range(lowKey, highKey).
  // For the format of "lowKey" and "highKey", please see insertEntry()
  // If lowKeyInclusive (or highKeyInclusive) is true, then lowKey (or highKey)
  // should be included in the scan
  // If lowKey is null, then the range is -infinity to highKey
  // If highKey is null, then the range is lowKey to +infinity
  RC scan(FileHandle &fileHandle,
      const Attribute &attribute,
      const void        *lowKey,
      const void        *highKey,
      bool        lowKeyInclusive,
      bool        highKeyInclusive,
      IX_ScanIterator &ix_ScanIterator);

  LeafPageHeader getLeafPageHeader(void * pageData);
  NonLeafPageHeader getNonLeafPageHeader(void * pageData);

 protected:
  IndexManager   ();                            // Constructor
  ~IndexManager  ();                            // Destructor

 private:
  static IndexManager *_index_manager;

  // Auxiliary methods.

  bool recordExistsInLeafPage(const Attribute &attribute, const void *key, const RID &rid, void * pageData);

  PageType getPageType(void * pageData);
  void setPageType(void * pageData, PageType pageType);
  void setNonLeafPageHeader(void * pageData, NonLeafPageHeader nonLeafHeader);
  void setMetaPageHeader(void * pageData, MetaPageHeader metaPageHeader);
  void setLeafPageHeader(void * pageData, LeafPageHeader leafHeader);

  RC deleteEntryFromLeaf(const Attribute &attribute, const void *key, const RID &rid, void * pageData);

  RC insertNonLeafRecord(const Attribute &attribute, ChildEntry &newChildEntry, void * pageData);
  RC insertLeafRecord(const Attribute &attribute, const void *key, const RID &rid, void * pageData);
  RC insertEntry(const Attribute &attribute, const void *key, const RID &rid, FileHandle &fileHandle, uchar* &fileMap, unsigned pageID, ChildEntry &newChildEntry);
  RC deleteEntry(const Attribute &attribute, const void *key, const RID &rid, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID);

  RC setRootPageID(FileHandle &fileHandle, unsigned newPageID);

  unsigned getKeyLength(const Attribute &attribute, const void * key);

  unsigned getSonPageID(const Attribute attribute, const void * key, void * pageData);

  RC splitNonLeaf(const Attribute &attribute, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID, ChildEntry &newParentEntry);
  RC splitLeaf(const Attribute &attribute, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID, ChildEntry &newParentEntry);
};

class IX_ScanIterator
{
 public:
  IX_ScanIterator();  							// Constructor
  ~IX_ScanIterator(); 							// Destructor
  
  RC moveToLeaf(uchar* fileMap, const Attribute &attribute, unsigned &pageID, unsigned &offset, const void* lowKey, const bool lowKeyInclusive);
  RC getNextEntry(RID &rid, void *key);  		// Get next matching entry
  RC open(FileHandle &fileHandle,
    const Attribute &attribute,
    const void *lowKey,
    const void *highKey,
    bool lowKeyInclusive,
    bool highKeyInclusive);
  RC close();             						// Terminate index scan
  
  private:
  vector<pair<RID, void*>> results;
  unsigned attrSize;
};

// print out the error message for a given return code
void IX_PrintError (RC rc);


#endif
