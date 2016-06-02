#include <cstdint>
#include <cassert>

#include "ix.h"

bool RIDKeyPairSort(const pair<RID, void*>& pair1, const pair<RID, void*>& pair2)
{
    if(pair1.first.pageNum == pair2.first.pageNum)
    {
        return pair1.first.slotNum > pair2.first.slotNum;
    }
    else
    {
        return pair1.first.pageNum > pair2.first.pageNum;
    }
}

IndexManager* IndexManager::_index_manager = 0;

IndexManager* IndexManager::instance()
{
    if(!_index_manager)
        _index_manager = new IndexManager();

    return _index_manager;
}

IndexManager::IndexManager()
{
}

IndexManager::~IndexManager()
{
}

// Returns the page type of a specific page given in input.
PageType IndexManager::getPageType(void * pageData)
{
    PageType type;
    memcpy(&type, pageData, sizeof(PageType));
    return type;
}

// Sets the page type of a specific page given in input.
void IndexManager::setPageType(void * pageData, PageType pageType)
{
    memcpy(pageData, &pageType, sizeof(PageType));
}

// Returns the header of a specific non leaf page given in input.
NonLeafPageHeader IndexManager::getNonLeafPageHeader(void * pageData)
{
    NonLeafPageHeader nonLeafHeader;
    memcpy(&nonLeafHeader, (char*) pageData + sizeof(PageType), sizeof(NonLeafPageHeader));
    assert(nonLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    assert(nonLeafHeader.freeSpaceOffset >= NON_LEAF_PAGE_DATA_OFFSET);
    assert(nonLeafHeader.recordEntriesNumber < PAGE_SIZE);
    return nonLeafHeader;
}

// Sets the header of a specific non leaf page given in input.
void IndexManager::setNonLeafPageHeader(void * pageData, NonLeafPageHeader nonLeafHeader)
{
    assert(nonLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    assert(nonLeafHeader.freeSpaceOffset >= NON_LEAF_PAGE_DATA_OFFSET);
    assert(nonLeafHeader.recordEntriesNumber < PAGE_SIZE);
    memcpy((char*) pageData + sizeof(PageType), &nonLeafHeader, sizeof(NonLeafPageHeader));
}

// Sets the header of the meta page given in input.
void IndexManager::setMetaPageHeader(void * pageData, MetaPageHeader metaPageHeader)
{
    memcpy((char*) pageData + sizeof(PageType), &metaPageHeader, sizeof(MetaPageHeader));
}

// Returns the header of a specific leaf page given in input.
LeafPageHeader IndexManager::getLeafPageHeader(void * pageData)
{
    LeafPageHeader leafHeader;
    memcpy(&leafHeader, (char*) pageData + sizeof(PageType), sizeof(LeafPageHeader));
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    assert(leafHeader.recordEntriesNumber < PAGE_SIZE);
    assert(leafHeader.freeSpaceOffset >= LEAF_PAGE_DATA_OFFSET);
    return leafHeader;
}

// Sets the header of a specific leaf page given in input.
void IndexManager::setLeafPageHeader(void * pageData, LeafPageHeader leafHeader)
{
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    assert(leafHeader.recordEntriesNumber < PAGE_SIZE);
    assert(leafHeader.freeSpaceOffset >= LEAF_PAGE_DATA_OFFSET);
    memcpy((char*) pageData + sizeof(PageType), &leafHeader, sizeof(LeafPageHeader));
}

// Checks if a specific page is a leaf or not.
bool IndexManager::isLeafPage(void * pageData)
{
    return getPageType(pageData) == LeafPage;
}

// Gets the current root page ID by reading the first page of the file (which only task is containing it).
unsigned IndexManager::getRootPageID(FileHandle &fileHandle)
{
    void* pageData = nullptr;

    fileHandle.readPage(0, pageData);
    PageType pageType;
    memcpy(&pageType, pageData, sizeof(PageType));
    assert(pageType == MetaPage);
    MetaPageHeader metaPageHeader;
    memcpy(&metaPageHeader, (uchar*)pageData + sizeof(PageType), sizeof(MetaPageHeader));
    fileHandle.getQFile()->unmap((uchar*)pageData);

    return metaPageHeader.rootPage;
}

// Given a non-leaf page and a key, finds the correct (direct) son page ID in which the key "fits".
unsigned IndexManager::getSonPageID(const Attribute attribute, const void * key, void * pageData)
{
    return -1;
}

RC IndexManager::createFile(const string &fileName)
{
    PagedFileManager* pageFileManager = PagedFileManager::instance();

    // Creating a new paged file.
    if(pageFileManager->createFile(fileName.c_str()) != SUCCESS) return ERROR_PFM_CREATE;

    // Setting up the first pages.
    FileHandle handle;
    if(pageFileManager->openFile(fileName.c_str(), handle) != SUCCESS) return ERROR_PFM_OPEN;
    void * pageData = malloc(PAGE_SIZE);

    // Setup and add meta page
    const unsigned DEFAULT_ROOT = 1;
    MetaPageHeader metaPageHeader {DEFAULT_ROOT};
    setMetaPageHeader(pageData, metaPageHeader);
    setPageType(pageData, MetaPage);
    if(handle.appendPage(pageData) != SUCCESS) return ERROR_PFM_WRITEPAGE;

    // Setup and add the first record based page.
    const unsigned FIRST_LEAF = DEFAULT_ROOT + 1;
    NonLeafPageHeader nonLeafHeader;
    nonLeafHeader.freeSpaceOffset = NON_LEAF_PAGE_DATA_OFFSET;
    assert(nonLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    nonLeafHeader.recordEntriesNumber = 0;
    setNonLeafPageHeader(pageData, nonLeafHeader);
    setPageType(pageData, NonLeafPage);
    memcpy((uchar*)pageData + NON_LEAF_PAGE_DATA_OFFSET - sizeof(unsigned), &FIRST_LEAF, sizeof(unsigned));
    if(handle.appendPage(pageData) != SUCCESS) return ERROR_PFM_WRITEPAGE;

    // Setup first leaf
    LeafPageHeader leafHeader;
    leafHeader.freeSpaceOffset = LEAF_PAGE_DATA_OFFSET;
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    leafHeader.recordEntriesNumber = 0;
    leafHeader.nextPage = 0;
    leafHeader.prevPage = 0;
    setLeafPageHeader(pageData, leafHeader);
    setPageType(pageData, LeafPage);
    if(handle.appendPage(pageData) != SUCCESS) return ERROR_PFM_WRITEPAGE;

    // Cleanup
    if(pageFileManager->closeFile(handle) != SUCCESS) return ERROR_PFM_CLOSE;
    free(pageData);

    return SUCCESS;
}

RC IndexManager::destroyFile(const string &fileName)
{
    return PagedFileManager::instance()->destroyFile(fileName.c_str());
}

RC IndexManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    return PagedFileManager::instance()->openFile(fileName.c_str(), fileHandle);
}

RC IndexManager::closeFile(FileHandle &fileHandle) {
    return PagedFileManager::instance()->closeFile(fileHandle);
}

// Given a ChildEntry structure (<key, child page id>), writes it into the correct position within the non leaf page "pageData".
RC IndexManager::insertNonLeafRecord(const Attribute &attribute, ChildEntry &newChildEntry, void * pageData)
{
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(unsigned);
    uchar* page = (uchar*) pageData;
    NonLeafPageHeader nonLeafHeader = getNonLeafPageHeader(pageData);
    unsigned offset = NON_LEAF_PAGE_DATA_OFFSET;
    
    // Error if there is no room for insertion
    if(PAGE_SIZE - nonLeafHeader.freeSpaceOffset < attrEntrySize) return ERROR_NO_FREE_SPACE;
    
    // Find position for insertion and shift entries
    for(unsigned i = 0; i < nonLeafHeader.recordEntriesNumber; ++i)
    {
        if(compareKeys(attribute, newChildEntry.key, page + offset) < 0) break;
        offset += attrEntrySize;
    }
    
    unsigned moveSize = nonLeafHeader.freeSpaceOffset - offset;
    assert(nonLeafHeader.freeSpaceOffset >= offset);
    assert(offset <= PAGE_SIZE - attrEntrySize);

    if(moveSize > 0)
    {
        assert(offset + attrEntrySize + moveSize <= PAGE_SIZE);
        memmove(page + offset + attrEntrySize, page + offset, moveSize);
    }
    nonLeafHeader.freeSpaceOffset += attrEntrySize;
    assert(nonLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    nonLeafHeader.recordEntriesNumber++;
    
    // Insert 
    memcpy(page + offset, newChildEntry.key, attrSize);
    memcpy(page + offset + attrSize, &newChildEntry.childPage, sizeof(unsigned));
    setNonLeafPageHeader(page, nonLeafHeader);

    return SUCCESS;
}

// Given a record entry (<key, RID>), writes it into the correct position within the leaf page "pageData".
RC IndexManager::insertLeafRecord(const Attribute &attribute, const void *key, const RID &rid, void * pageData)
{
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(RID);
    uchar* page = (uchar*) pageData;
    LeafPageHeader leafHeader = getLeafPageHeader(pageData);
    unsigned offset = LEAF_PAGE_DATA_OFFSET;
    
    // Error if there is no room for insertion
    if(PAGE_SIZE - leafHeader.freeSpaceOffset < attrEntrySize) return ERROR_NO_FREE_SPACE;
    
    // Find position for insertion and shift entries
    for(unsigned i = 0; i < leafHeader.recordEntriesNumber; ++i)
    {
        if(compareKeys(attribute, key, page + offset) < 0) break;
        offset += attrEntrySize;
    }

    assert(offset <= PAGE_SIZE - attrEntrySize);
    unsigned moveSize = leafHeader.freeSpaceOffset - offset;
    assert(leafHeader.freeSpaceOffset >= offset);

    if(moveSize > 0)
    {
        assert(offset + attrEntrySize + moveSize <= PAGE_SIZE);
        memmove(page + offset + attrEntrySize, page + offset, moveSize);
    }
    leafHeader.freeSpaceOffset += attrEntrySize;
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    leafHeader.recordEntriesNumber++;
    
    // Insert 
    memcpy(page + offset, key, attrSize);
    memcpy(page + offset + attrSize, &rid, sizeof(RID));
    setLeafPageHeader(pageData, leafHeader);
    
    return SUCCESS;
}

RC IndexManager::splitLeaf(const Attribute &attribute, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID, ChildEntry &newParentEntry)
{
    uchar* page = pageFile + pageID * PAGE_SIZE;
    LeafPageHeader leafHeader = getLeafPageHeader(page);
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(RID);

    // Determine where to split
    unsigned splitPos;
    if(leafHeader.recordEntriesNumber % 2 == 1)
    {
        splitPos = leafHeader.recordEntriesNumber / 2;
    }
    else
    {
        splitPos = leafHeader.recordEntriesNumber / 2 + 1;
    }
    unsigned entriesBefore = splitPos;
    unsigned entriesAfter = leafHeader.recordEntriesNumber - splitPos;

    // Get split info
    newParentEntry.childPage = fileHandle.getNumberOfPages();  // Index of new page that we will insert later
    newParentEntry.key = malloc(attrSize);
    memcpy(
        newParentEntry.key,
        page + LEAF_PAGE_DATA_OFFSET + splitPos * attrEntrySize,
        attrSize);

    // Prepare new page
    uchar* newPage = (uchar*)malloc(PAGE_SIZE);
    LeafPageHeader newLeafHeader;
    newLeafHeader.recordEntriesNumber = entriesAfter;
    newLeafHeader.freeSpaceOffset = LEAF_PAGE_DATA_OFFSET + entriesAfter * attrEntrySize;
    assert(newLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    newLeafHeader.nextPage = leafHeader.nextPage;
    newLeafHeader.prevPage = pageID;
    setLeafPageHeader(newPage, newLeafHeader);
    setPageType(newPage, LeafPage);

    // Update next page's previous to point to new page
    if(leafHeader.nextPage != 0)
    {
        uchar* nextPage = pageFile + leafHeader.nextPage * PAGE_SIZE;
        LeafPageHeader nextLeafHeader = getLeafPageHeader(nextPage);
        nextLeafHeader.prevPage = newParentEntry.childPage;
        setLeafPageHeader(nextPage, nextLeafHeader);
    }

    // Copy data to new page
    memcpy(
        newPage + LEAF_PAGE_DATA_OFFSET,
        page + leafHeader.freeSpaceOffset - entriesAfter * attrEntrySize,
        entriesAfter * attrEntrySize);

    // Insert new page
    fileHandle.getQFile()->unmap((uchar*)pageFile);
    fileHandle.appendPage(newPage);
    free(newPage);
    pageFile = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);
    assert(pageFile != nullptr);
    page = pageFile + pageID * PAGE_SIZE;

    // Delete entries from old page
    leafHeader.freeSpaceOffset -= (entriesAfter) * attrEntrySize;
    leafHeader.recordEntriesNumber = entriesBefore;

    // Update old page's next pointer
    leafHeader.nextPage = newParentEntry.childPage;
    setLeafPageHeader(page, leafHeader);

    return SUCCESS;
}

RC IndexManager::splitNonLeaf(const Attribute &attribute, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID, ChildEntry &newParentEntry)
{
    uchar* page = pageFile + pageID * PAGE_SIZE;
    NonLeafPageHeader nonLeafHeader = getNonLeafPageHeader(page);
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(unsigned);

    // Determine where to split
    unsigned splitPos = nonLeafHeader.recordEntriesNumber / 2;
    unsigned entriesBefore = splitPos;
    unsigned entriesAfter = nonLeafHeader.recordEntriesNumber - splitPos - 1;

    // Get split infomration
    newParentEntry.childPage = fileHandle.getNumberOfPages();  // Index of new page that we will insert later
    newParentEntry.key = malloc(attrSize);
    memcpy(
        newParentEntry.key,
        page + nonLeafHeader.freeSpaceOffset - (entriesAfter + 1) * attrEntrySize,
        attrSize);

    // Prepare new page
    uchar* newPage = (uchar*)malloc(PAGE_SIZE);
    NonLeafPageHeader newNonLeafHeader;
    newNonLeafHeader.recordEntriesNumber = entriesAfter;
    newNonLeafHeader.freeSpaceOffset = nonLeafHeader.freeSpaceOffset - (entriesBefore + 1) * attrEntrySize;
    assert(newNonLeafHeader.freeSpaceOffset <= PAGE_SIZE);
    setNonLeafPageHeader(newPage, newNonLeafHeader);
    setPageType(newPage, NonLeafPage);

    // Copy data to new page
    memcpy(
        newPage + NON_LEAF_PAGE_DATA_OFFSET,
        page + nonLeafHeader.freeSpaceOffset - entriesAfter * attrEntrySize - sizeof(unsigned),
        entriesAfter * attrEntrySize + sizeof(unsigned));

    // Insert new page
    fileHandle.getQFile()->unmap((uchar*)pageFile);
    fileHandle.appendPage(newPage);
    free(newPage);
    pageFile = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);
    assert(pageFile != nullptr);
    page = pageFile + pageID * (unsigned)PAGE_SIZE;

    // Delete entries from old page
    nonLeafHeader.freeSpaceOffset -= (entriesAfter + 1) * attrEntrySize;
    nonLeafHeader.recordEntriesNumber = entriesBefore;
    setNonLeafPageHeader(page, nonLeafHeader);

    return SUCCESS;
}


// Recursive insert of the record <key, rid> into the (current) page "pageID".
// newChildEntry will store the return information of the "child" insert call.
// Following the exact implementation described in Ramakrishnan - Gehrke, p.349.
RC IndexManager::insertEntry(const Attribute &attribute, const void *key, const RID &rid, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID, ChildEntry &newChildEntry)
{
    uchar* page = pageFile + pageID * PAGE_SIZE;
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();
    PageType pageType = getPageType(page);
    unsigned attrSize;
    unsigned attrEntrySize;
    unsigned recursePage;
    LeafPageHeader leafHeader;
    NonLeafPageHeader nonLeafHeader;
    newChildEntry.childPage = 0;

    ChildEntry newParentEntry;
    uchar* curr;
    RC status;

    switch(pageType)
    {
        case NonLeafPage:
            attrSize = recordBasedFileManager->getMaxAttributeSize(attribute);
            attrEntrySize = attrSize + sizeof(unsigned);
            nonLeafHeader = getNonLeafPageHeader(page);
            curr = page + NON_LEAF_PAGE_DATA_OFFSET;
            for(unsigned i = 0; i < nonLeafHeader.recordEntriesNumber; i++)
            {
                if(compareKeys(attribute, key, curr) < 0)
                {
                    break;
                }
                curr += attrEntrySize;
            }
            assert(curr <= page + PAGE_SIZE);

            recursePage = *(unsigned*)(curr - sizeof(unsigned));
            status = insertEntry(attribute, key, rid, fileHandle, pageFile, recursePage, newChildEntry);
            if(status != SUCCESS) return status;

            // Splitting will remap file
            page = pageFile + pageID * PAGE_SIZE;
            nonLeafHeader = getNonLeafPageHeader(page);

            // Check for split to determine if new entry in current node is required
            if(newChildEntry.childPage == 0) return SUCCESS;

            // Check for free space for new entry
            if(PAGE_SIZE - nonLeafHeader.freeSpaceOffset >=  attrEntrySize)
            {
                // Enough space, insert new entry
                status = insertNonLeafRecord(attribute, newChildEntry, page);
                nonLeafHeader = getNonLeafPageHeader(page);
                newChildEntry.childPage = 0;
                free(newChildEntry.key);
                if(status != SUCCESS) return status;
            }
            else
            {
                // Do the splits
                status = splitNonLeaf(attribute, fileHandle, pageFile, pageID, newParentEntry);
                if(status != SUCCESS) return status;

                // Splitting will remap file
                page = pageFile + pageID * PAGE_SIZE;

                // Insert new child in one of the newly split nodes
                if(compareKeys(attribute, key, newParentEntry.key) < 0)  // Insert into old page
                {
                    status = insertNonLeafRecord(attribute, newChildEntry, page);
                    if(status != SUCCESS) return status;
                }
                else  // Insert into new page
                {
                    status = insertNonLeafRecord(attribute, newChildEntry, pageFile + newParentEntry.childPage * (unsigned)PAGE_SIZE);
                    if(status != SUCCESS) return status;
                }

                // Cleanup
                free(newChildEntry.key);
                newChildEntry = newParentEntry;
            }

            return SUCCESS;

        case LeafPage:
            attrSize = recordBasedFileManager->getMaxAttributeSize(attribute);
            attrEntrySize = attrSize + sizeof(RID);
            leafHeader = getLeafPageHeader(page);

            // Check for free space for new entry
            if(PAGE_SIZE - leafHeader.freeSpaceOffset >=  attrEntrySize)
            {
                // Enough space, insert new entry
                status = insertLeafRecord(attribute, key, rid, page);
                leafHeader = getLeafPageHeader(page);
                if(status != SUCCESS) return status;
            }
            else
            {
                // Do the splits
                status = splitLeaf(attribute, fileHandle, pageFile, pageID, newParentEntry);
                if(status != SUCCESS) return status;

                // Splitting will remap file
                page = pageFile + pageID * PAGE_SIZE;

                // Insert new child in one of the newly split nodes
                if(compareKeys(attribute, key, newParentEntry.key) < 0)  // Insert into old page
                {
                    status = insertLeafRecord(attribute, key, rid, page);
                    if(status != SUCCESS) return status;
                }
                else  // Insert into new page
                {
                    status = insertLeafRecord(attribute, key, rid, pageFile + newParentEntry.childPage * PAGE_SIZE);
                    if(status != SUCCESS) return status;
                }

                // Cleanup
                free(newChildEntry.key);
                newChildEntry = newParentEntry;
            }

            return SUCCESS;

        default:
            return ERROR_UNKNOWN;
    } // end switch

    return ERROR_UNKNOWN;
}

RC IndexManager::insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    ChildEntry newChildEntry;
    newChildEntry.key = NULL;
    newChildEntry.childPage = 0;

    uchar* pageFile = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);

    // Recursive insert, starting from the root page.
    RC status = insertEntry(attribute, key, rid, fileHandle, pageFile, getRootPageID(fileHandle), newChildEntry);

    if(newChildEntry.childPage != 0)
    {
        free(newChildEntry.key);
        MetaPageHeader metaPageHeader {newChildEntry.childPage};
        setMetaPageHeader(pageFile, metaPageHeader);
    }

    fileHandle.getQFile()->unmap((uchar*)pageFile);
    fileHandle.getQFile()->flush();

    return status;
}

// Given a record entry <key, rid>, deletes it from the leaf page "pageData".
RC IndexManager::deleteEntryFromLeaf(const Attribute &attribute, const void *key, const RID &rid, void * pageData)
{
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(RID);
    uchar* page = (uchar*) pageData;
    LeafPageHeader leafHeader = getLeafPageHeader(page);
    unsigned offset = LEAF_PAGE_DATA_OFFSET;
    
    // Find position for deletion
    for(unsigned i = 0; i < leafHeader.recordEntriesNumber; ++i)
    {
        const RID entryRid = *(RID*)(page + offset + attrSize);
        if(compareKeys(attribute, key, page + offset) == 0 && entryRid.pageNum == rid.pageNum && entryRid.slotNum == rid.slotNum) break;
        offset += attrEntrySize;
    }
    
    // Error if we cannot find entry for deletion
    if(offset >= leafHeader.freeSpaceOffset) return ERROR_RECORD_NOT_EXISTS;
    
    // Shift entries (delete)
    assert(leafHeader.freeSpaceOffset >= offset + attrEntrySize);
    if(leafHeader.freeSpaceOffset != offset + attrEntrySize)
    {
        memmove(page + offset, page + offset + attrEntrySize, leafHeader.freeSpaceOffset - offset - attrEntrySize);
    }
    assert(leafHeader.freeSpaceOffset >= offset + attrEntrySize);
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    leafHeader.freeSpaceOffset -= attrEntrySize;
    assert(leafHeader.freeSpaceOffset >= LEAF_PAGE_DATA_OFFSET);
    assert(leafHeader.freeSpaceOffset <= PAGE_SIZE);
    leafHeader.recordEntriesNumber--;
    setLeafPageHeader(page, leafHeader);
    
    return SUCCESS;
}

RC IndexManager::deleteEntry(const Attribute &attribute, const void *key, const RID &rid, FileHandle &fileHandle, uchar* &pageFile, unsigned pageID)
{
    uchar* page = pageFile + pageID * PAGE_SIZE;
    uchar* curr;
    
    PageType pageType = getPageType(page);
    NonLeafPageHeader nonLeafHeader;

    unsigned attrSize;
    unsigned attrEntrySize;
    unsigned recursePage;

    switch(pageType)
    {
        case NonLeafPage:
            attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
            attrEntrySize = attrSize + sizeof(unsigned);
            nonLeafHeader = getNonLeafPageHeader(page);
            curr = page + NON_LEAF_PAGE_DATA_OFFSET;
            for(unsigned i = 0; i < nonLeafHeader.recordEntriesNumber; i++)
            {
                if(compareKeys(attribute, key, curr) < 0)
                {
                    break;
                }
                curr += attrEntrySize;
            }

            assert(page + PAGE_SIZE >= curr + sizeof(unsigned));
            recursePage = *(unsigned*)(curr - sizeof(unsigned));
            return deleteEntry(attribute, key, rid, fileHandle, pageFile, recursePage);
        
        case LeafPage:          
            return deleteEntryFromLeaf(attribute, key, rid, page);
        
        default:
            return ERROR_UNKNOWN;
    }
}

// Entry point for the recursive deletion of the (*key,rid) pair from the index file associated with fileHandle.
RC IndexManager::deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{   
    uchar* pageFile = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);

    // Recursive delete, starting from the root page.
    RC status = deleteEntry(attribute, key, rid, fileHandle, pageFile, getRootPageID(fileHandle));

    fileHandle.getQFile()->unmap((uchar*)pageFile);
    fileHandle.getQFile()->flush();

    return status;
}

// Recursive search through the tree, returning the page ID of the leaf page that should contain the input key.
RC IndexManager::treeSearch(uchar* pageFile, const Attribute attribute, const void * key, unsigned currentPageID, unsigned &returnPageID)
{
    uchar* page = pageFile + currentPageID * PAGE_SIZE;
    uchar* curr;
    
    PageType pageType = getPageType(page);
    NonLeafPageHeader nonLeafHeader;

    unsigned attrSize;
    unsigned attrEntrySize;
    unsigned recursePage;
    
    switch(pageType)
    {
        case NonLeafPage:
            attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
            attrEntrySize = attrSize + sizeof(unsigned);
            nonLeafHeader = getNonLeafPageHeader(page);
            curr = page + NON_LEAF_PAGE_DATA_OFFSET;
            for(unsigned i = 0; i < nonLeafHeader.recordEntriesNumber; i++)
            {
                if(key == nullptr || compareKeys(attribute, key, curr) < 0)
                {
                    break;
                }
                curr += attrEntrySize;
            }

            recursePage = *(unsigned*)(curr - sizeof(unsigned));
            return treeSearch(pageFile, attribute, key, recursePage, returnPageID);
        
        case LeafPage:          
            returnPageID = currentPageID;
            return SUCCESS;

        default:
            return ERROR_UNKNOWN;
    }

    return ERROR_RECORD_NOT_EXISTS;
}

int IndexManager::compareKeys(const Attribute attribute, const void * key1, const void * key2)
{
    switch (attribute.type)
    {
        float diff;
        case TypeInt:
            return *(int32_t*)key1 - *(int32_t*)key2;
        break;
        case TypeReal:
            diff = *(float*)key1 - *(float*)key2;

            // Check for equality
            if(fabs(diff) <= std::numeric_limits<float>::epsilon()) return 0;

            // Return int difference rounded up
            if(diff > 0) return (int)ceil(diff);
            else return (int)floor(diff);
        break;
        case TypeVarChar:
            // We have to get the size of the VarChar field by reading the integer that precedes the string value itself.
            unsigned varchar1Size;
            memcpy(&varchar1Size, (char*) key1, VARCHAR_LENGTH_SIZE);
            unsigned varchar2Size;
            memcpy(&varchar2Size, (char*) key1, VARCHAR_LENGTH_SIZE);
            unsigned varcharSize = min(varchar1Size, varchar2Size);

            // We also have to account for the overhead given by that integer.
            int cmp = memcmp((char*)key1 + INT_SIZE, (char*)key2 + INT_SIZE, varcharSize);
            if(cmp == 0 && varchar1Size != varchar2Size) return varchar1Size - varchar2Size;
            else return cmp;
        break;
    }
}

RC IndexManager::scan(FileHandle &fileHandle,
    const Attribute &attribute,
    const void      *lowKey,
    const void      *highKey,
    bool			lowKeyInclusive,
    bool        	highKeyInclusive,
    IX_ScanIterator &ix_ScanIterator)
{
    RC status = ix_ScanIterator.open(fileHandle, attribute, lowKey, highKey, lowKeyInclusive, highKeyInclusive);
    if(status != SUCCESS) return status;
     
	return SUCCESS;
}

IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}

RC IX_ScanIterator::moveToLeaf(uchar* fileMap, const Attribute &attribute, unsigned &pageID, unsigned &offset, const void* lowKey, const bool lowKeyInclusive)
{
    offset = LEAF_PAGE_DATA_OFFSET;
    unsigned attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(RID);
    IndexManager* indexManager = IndexManager::instance();

    if(lowKey == nullptr) return SUCCESS;
    
    // Move current to the right spot of the leafPage
    while(IndexManager::instance()->compareKeys(attribute, lowKey, fileMap + pageID * PAGE_SIZE + offset) != 0)
    {
        offset += attrEntrySize;
        if(offset >= PAGE_SIZE) return ERROR_RECORD_NOT_EXISTS;
    }
    
    // Ignore entries which share the same key as lowKey    
    if(!lowKeyInclusive)
    {
        while(indexManager->compareKeys(attribute, lowKey, fileMap + pageID * PAGE_SIZE + offset) == 0)
        {
            LeafPageHeader* leafPageHeader = (LeafPageHeader*)(fileMap + (pageID * PAGE_SIZE) + sizeof(PageType));
            offset += attrEntrySize;
            if(offset > leafPageHeader->freeSpaceOffset + attrEntrySize)
            {
                pageID = leafPageHeader->nextPage;
                offset = LEAF_PAGE_DATA_OFFSET;
            }
        }
    }
    
    return SUCCESS;
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
    if(results.empty()) return IX_EOF;
    rid = results.back().first;
    memcpy(key, results.back().second, attrSize);
    free(results.back().second);
    results.pop_back();

	return SUCCESS;
}

RC IX_ScanIterator::open(FileHandle &fileHandle,
    const Attribute &attribute,
    const void *lowKey,
    const void *highKey,
    bool lowKeyInclusive,
    bool highKeyInclusive)
{
    // Map file, return if there are any errors
    if(!results.empty())
    {
        return 1;
    }
    
    uchar* fileMap = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);
    
    if(fileMap == nullptr)
    {
        return 2;
    }
    
    // Setup scanning environment
    IndexManager* indexManager = IndexManager::instance();
    unsigned currentPageID = indexManager->getRootPageID(fileHandle);
    unsigned offset;

    RC status;

    attrSize = RecordBasedFileManager::instance()->getMaxAttributeSize(attribute);
    unsigned attrEntrySize = attrSize + sizeof(RID);
    status = indexManager->treeSearch(fileMap, attribute, lowKey, currentPageID, currentPageID);
    if(status != SUCCESS) return status;
    status = moveToLeaf(fileMap, attribute, currentPageID, offset, lowKey, lowKeyInclusive);
    if(status != SUCCESS) return status;

    LeafPageHeader leafPageHeader = indexManager->getLeafPageHeader(fileMap + (currentPageID * PAGE_SIZE));

    while(true)
    {
        // if we have reached the end of a page, move onto the next one
        if(offset + attrEntrySize > leafPageHeader.freeSpaceOffset)
        {
            offset = LEAF_PAGE_DATA_OFFSET;
            currentPageID = leafPageHeader.nextPage;
            if(currentPageID == 0) break;
            leafPageHeader = indexManager->getLeafPageHeader(fileMap + (currentPageID * PAGE_SIZE));
        }

        uchar* current = fileMap + currentPageID * PAGE_SIZE + offset;

        // ignore highKey if not highKeyInclusive
        if(highKey != nullptr && !highKeyInclusive && indexManager->compareKeys(attribute, current, highKey) == 0)
        {
            break;
        }

        if(highKey != nullptr && indexManager->compareKeys(attribute, current, highKey) > 0)
        {
            break;
        }

        pair<RID, void*> entry;
        entry.second = malloc(attrSize);
        memcpy(entry.second, current, attrSize);
        entry.first = *(RID*)(current + attrSize);
        results.push_back(entry);

        offset += attrEntrySize;
    }

    // Sort results in desending order so that when we pull from the back we get ascending
    sort(results.begin(), results.end(), RIDKeyPairSort);

    fileHandle.getQFile()->unmap(fileMap);
    fileHandle.getQFile()->flush();
    return SUCCESS;
}

RC IX_ScanIterator::close()
{
    // Empty results
    while(!results.empty())
    {
        free(results.back().second);
        results.pop_back();
    }

    return SUCCESS;
}

void IX_PrintError (RC rc)
{
    switch (rc)
    {
        case ERROR_PFM_CREATE:
            cout << "Paged File Manager error: create file." << endl;
        break;
        case ERROR_PFM_DESTROY:
            cout << "Paged File Manager error: destroy file." << endl;
        break;
        case ERROR_PFM_OPEN:
            cout << "Paged File Manager error: open file." << endl;
        break;
        case ERROR_PFM_CLOSE:
            cout << "Paged File Manager error: close file." << endl;
        break;
        case ERROR_PFM_READPAGE:
            cout << "Paged File Manager error: read page." << endl;
        break;
        case ERROR_PFM_WRITEPAGE:
            cout << "Paged File Manager error: write page." << endl;
        break;
        case ERROR_PFM_FILEHANDLE:
            cout << "Paged File Manager error: FileHandle problem." << endl;
        break;
        case ERROR_NO_SPACE_AFTER_SPLIT:
            cout << "Tree split error: There is no space for the new entry, even after the split." << endl;
        break;
        case ERROR_RECORD_EXISTS:
            cout << "Index insert error: record already exists." << endl;
        break;
        case ERROR_RECORD_NOT_EXISTS:
            cout << "Index delete error: record does not exists." << endl;
        break;
        case ERROR_UNKNOWN:
            cout << "Unknown error." << endl;
        break;
    }
}

