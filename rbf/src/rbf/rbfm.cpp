// CMPS 181 - Project 1
// Author:				Ian Gudger and Peter Czupil
// File description:	Implementing the "Variable length records" page structure
//						(ref. p. 329 Ramakrishnan, Gehrke).

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cstdio>
#include <cassert>

#include "rbfm.h"

// ------------------------------------------------------------
// RecordBasedFileManager Class
// ------------------------------------------------------------

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;
PagedFileManager* RecordBasedFileManager::_pf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
	// Singleton design pattern.
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
	// Initialize the internal PagedFileManager instance.
	_pf_manager = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

// Configures a new record based page, and puts it in "page".
void RecordBasedFileManager::newRecordBasedPage(void * page)
{
	// Writes the slot directory header.
	SlotDirectoryHeader slotHeader;
	setSlotDirectoryHeader(page, slotHeader);
}

RC RecordBasedFileManager::createFile(const string &fileName)
{
    // Creating a new paged file.
	if (_pf_manager->createFile(fileName.c_str()) != SUCCESS)
		return 1;

	// Setting up the first page.
	void * firstPageData = malloc(PAGE_SIZE);
	newRecordBasedPage(firstPageData);

	// Adds the first record based page.
	FileHandle handle;
	_pf_manager->openFile(fileName.c_str(), handle);
	handle.appendPage(firstPageData);
	_pf_manager->closeFile(handle);

	free(firstPageData);

	return 0;
}

RC RecordBasedFileManager::destroyFile(const string &fileName)
{
	return _pf_manager->destroyFile(fileName.c_str());
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
	return _pf_manager->openFile(fileName.c_str(), fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return _pf_manager->closeFile(fileHandle);
}

SlotDirectoryHeader RecordBasedFileManager::getSlotDirectoryHeader(void * page)
{
	// Getting the slot directory header.
	SlotDirectoryHeader slotHeader;
	memcpy (&slotHeader, page, sizeof(SlotDirectoryHeader));
	return slotHeader;
}

void RecordBasedFileManager::setSlotDirectoryHeader(void * page, SlotDirectoryHeader slotHeader)
{
	// Setting the slot directory header.
	memcpy (page, &slotHeader, sizeof(SlotDirectoryHeader));
}

SlotDirectoryRecordEntry RecordBasedFileManager::getSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber)
{
	// Getting the slot directory entry data.
	SlotDirectoryRecordEntry recordEntry;
	memcpy	(
			&recordEntry,
			((char*) page + sizeof(SlotDirectoryHeader) + recordEntryNumber * sizeof(SlotDirectoryRecordEntry)),
			sizeof(SlotDirectoryRecordEntry)
			);

	return recordEntry;
}

void RecordBasedFileManager::setSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber, SlotDirectoryRecordEntry recordEntry)
{
	// Setting the slot directory entry data.
	memcpy	(
			((char*) page + sizeof(SlotDirectoryHeader) + recordEntryNumber * sizeof(SlotDirectoryRecordEntry)),
			&recordEntry,
			sizeof(SlotDirectoryRecordEntry)
			);
}

// Computes the free space of a page (function of the free space pointer and the slot directory size).
unsigned RecordBasedFileManager::getPageFreeSpaceSize(void * page) {

	SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(page);

	return slotHeader.freeSpaceOffset - slotHeader.recordEntriesNumber * sizeof(SlotDirectoryRecordEntry) - sizeof(SlotDirectoryHeader);
}

unsigned RecordBasedFileManager::getRecordSize(const vector<Attribute> &recordDescriptor, const void *data)
{

	unsigned size = 0;
	unsigned varcharSize = 0;

	for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
		switch (recordDescriptor[i].type)
		{
			case TypeInt:
				size += INT_SIZE;
			break;
			case TypeReal:
				size += REAL_SIZE;
			break;
			case TypeVarChar:
				// We have to get the size of the VarChar field by reading the integer that precedes the string value itself.
				memcpy(&varcharSize, (char*) data + size, VARCHAR_LENGTH_SIZE);
				// We also have to account for the overhead given by that integer.
				size += INT_SIZE + varcharSize;
			break;
		}
    }

	return size;
}

unsigned RecordBasedFileManager::getMaxRecordSize(const vector<Attribute> &recordDescriptor)
{

    unsigned size = 0;

    for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                size += INT_SIZE;
            break;
            case TypeReal:
                size += REAL_SIZE;
            break;
            case TypeVarChar:
                size += INT_SIZE + recordDescriptor[i].length;
            break;
        }
    }

    return size;
}

unsigned RecordBasedFileManager::getMaxAttributeSize(const Attribute &attribute)
{
    switch (attribute.type)
    {
        case TypeInt:
            return INT_SIZE;
        case TypeReal:
            return REAL_SIZE;
        case TypeVarChar:
            return INT_SIZE + attribute.length;
    }

    return 0;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid)
{
	// Gets the size of the record.
	unsigned recordSize = getRecordSize(recordDescriptor, data);

	// Cycles through pages looking for enough free space for the new entry.

    void* pageFile = (void*)fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);
    if(pageFile == nullptr) return 1;

    void* pageData = pageFile;
	bool pageFound = false;
	unsigned i;
	for (i = 0; i < fileHandle.getNumberOfPages(); i++)
    {
		// When we find a page with enough space (accounting also for the size that will be added to the slot directory), we stop the loop.
		if (getPageFreeSpaceSize(pageData) >= sizeof(SlotDirectoryRecordEntry) + recordSize)
		{
			pageFound = true;
			break;
        }

        pageData = (char*)pageData + PAGE_SIZE;
	}

    void* pageBuffer = malloc(PAGE_SIZE);

	if(!pageFound)
	{
		// If we are here, there are no pages with enough space.
		// TODO (Project 2?): implementing the reorganizePage method and try to squeeze the records to get enough space
		// In this case, we just create a new white page.

        newRecordBasedPage(pageBuffer);

        fileHandle.appendPage(pageBuffer);
    }
    else
    {
        memcpy(pageBuffer, pageData, PAGE_SIZE);
    }

    fileHandle.getQFile()->unmap((uchar*)pageFile);
    pageData = nullptr;

    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageBuffer);

	// Setting the return RID.
	rid.pageNum = i;
	rid.slotNum = slotHeader.recordEntriesNumber;

	// Adding the new record reference in the slot directory.
	SlotDirectoryRecordEntry newRecordEntry;
	newRecordEntry.length = recordSize;
	newRecordEntry.offset = slotHeader.freeSpaceOffset - recordSize;
    setSlotDirectoryRecordEntry(pageBuffer, rid.slotNum, newRecordEntry);

	// Updating the slot directory header.
	slotHeader.freeSpaceOffset = newRecordEntry.offset;
	slotHeader.recordEntriesNumber += 1;
    setSlotDirectoryHeader(pageBuffer, slotHeader);

	// Adding the record data.
    memcpy	(((char*) pageBuffer + newRecordEntry.offset), data, recordSize);

	// Writing the page to disk.
    if(fileHandle.writePage(i, pageBuffer) != SUCCESS)
    {
        return 4;
    }

    free(pageBuffer);

	return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data)
{
    // Check if valid page
    if(fileHandle.getNumberOfPages() <= rid.pageNum) return 1;

    // Retrieve the specific page.
    void * pageData = nullptr;
    if (fileHandle.readPage(rid.pageNum, pageData) != SUCCESS) return 2;

	// Checks if the specific slot id exists in the page.
	SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageData);
    if(slotHeader.recordEntriesNumber < rid.slotNum) return 3;

	// Gets the slot directory record entry data.
	SlotDirectoryRecordEntry recordEntry = getSlotDirectoryRecordEntry(pageData, rid.slotNum);

    // Check if deleted
    if(recordEntry.meta & 1)
    {
        fileHandle.getQFile()->unmap((uchar*)pageData);
        return 1;
    }

    // Check if moved
    if(recordEntry.meta & 2)
    {
        RID newRid;
        newRid.pageNum = recordEntry.length;
        newRid.slotNum = recordEntry.offset;

        fileHandle.getQFile()->unmap((uchar*)pageData);
        return readRecord(fileHandle, recordDescriptor, newRid, data);
    }

	// Retrieve the actual entry data.
	memcpy	((char*) data, ((char*) pageData + recordEntry.offset), recordEntry.length);

    fileHandle.getQFile()->unmap((uchar*)pageData);
	return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data)
{
	unsigned offset = 0;

	int data_integer;
	float data_real;
	unsigned stringLength;
	char * stringData;

	for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
		switch (recordDescriptor[i].type)
		{
			case TypeInt:
				memcpy(&data_integer, ((char*) data + offset), INT_SIZE);
				offset += INT_SIZE;

				cout << "Attribute " << i << " (integer): " << data_integer << endl;
			break;
			case TypeReal:
				memcpy(&data_real, ((char*) data + offset), REAL_SIZE);
				offset += REAL_SIZE;

				cout << "Attribute " << i << " (real): " << data_real << endl;
			break;
			case TypeVarChar:
				// First VARCHAR_LENGTH_SIZE bytes describe the varchar length.
				memcpy(&stringLength, ((char*) data + offset), VARCHAR_LENGTH_SIZE);
				offset += VARCHAR_LENGTH_SIZE;

				// Gets the actual string.
				stringData = (char*) malloc(stringLength + 1);
				memcpy((void*) stringData, ((char*) data + offset), stringLength);
				// Adds the string terminator.
				stringData[stringLength] = '\0';
				offset += stringLength;

				cout << "Attribute " << i << " (string): " << stringData << endl;
				free(stringData);
			break;
		}

	return 0;
}

RC RecordBasedFileManager::deleteRecords(FileHandle &fileHandle)
{
    if(fileHandle.getQFile()->resize(PAGED_FILE_HEADER_STRING_LENGTH))
    {
        // Calling _qfile.size() after resize seems to be required.
        fileHandle.getQFile()->flush();
        assert(fileHandle.getQFile()->size() == PAGED_FILE_HEADER_STRING_LENGTH);

        return 0;
    }
    else
    {
        return 1;
    }
}

RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid)
{
    void* pageData = nullptr;
    if(fileHandle.readPage(rid.pageNum, pageData) != SUCCESS)
    {
        return 1;
    }

    // Checks if the specific slot id exists in the page.
    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageData);
    if(slotHeader.recordEntriesNumber < rid.slotNum) return 2;

    // Gets the slot directory record entry data.
    SlotDirectoryRecordEntry recordEntry = getSlotDirectoryRecordEntry(pageData, rid.slotNum);

    fileHandle.getQFile()->unmap((uchar*)pageData);

    // Check if already deleted
    if(recordEntry.meta & 1) return 3;

    // Check if moved
    if(recordEntry.meta & 2)
    {
        RID newRid;
        newRid.pageNum = recordEntry.length;
        newRid.slotNum = recordEntry.offset;
        return deleteRecord(fileHandle, recordDescriptor, newRid);
    }

    if(fileHandle.readPage(rid.pageNum, pageData) != SUCCESS)
    {
        return 4;
    }

    // Set deleted bit.
    recordEntry.meta |= 1;

    // Set size to zero.
    recordEntry.length = 0;
    setSlotDirectoryRecordEntry(pageData, rid.slotNum, recordEntry);

    fileHandle.getQFile()->unmap((uchar*)pageData);

    return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid)
{
    // Check if valid page
    if(fileHandle.getNumberOfPages() <= rid.pageNum) return 1;

    // Retrieve the specific page.
    void * pageData = nullptr;
    if (fileHandle.readPage(rid.pageNum, pageData) != SUCCESS) return 2;

    // Checks if the specific slot id exists in the page.
    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageData);
    if(slotHeader.recordEntriesNumber < rid.slotNum) return 3;

    // Gets the slot directory record entry data.
    SlotDirectoryRecordEntry recordEntry = getSlotDirectoryRecordEntry(pageData, rid.slotNum);

    // Check if deleted
    if(recordEntry.meta & 1) return 4;

    // Check if moved
    if(recordEntry.meta & 2)
    {
        RID newRid;

        newRid.pageNum = recordEntry.length;
        newRid.slotNum = recordEntry.offset;

        fileHandle.getQFile()->unmap((uchar*)pageData);
        return updateRecord(fileHandle, recordDescriptor, data, newRid);
    }

    // Cacluate new record size
    unsigned newSize = getRecordSize(recordDescriptor, data);

    if(newSize > recordEntry.length)
    {
        RID newRid;

        // Unmap memory before insert to allow moving within page.
        fileHandle.getQFile()->unmap((uchar*)pageData);

        // Move record
        if (insertRecord(fileHandle, recordDescriptor, data, newRid) != SUCCESS)
        {
            // Memory already unmapped so we can just return.
            return 5;
        }

        // Set moved data
        recordEntry.meta |= 2;
        recordEntry.length = newRid.pageNum;
        recordEntry.offset = newRid.slotNum;

        // Remap memory to allow slot directory update.
        if (fileHandle.readPage(rid.pageNum, pageData) != SUCCESS) return 5;
    }
    else
    {
        // Update record in place
        memcpy(((char*) pageData + recordEntry.offset), data, newSize);
        recordEntry.length = newSize;
    }

    // Update slot direcory header with new record entry
    setSlotDirectoryRecordEntry(pageData, rid.slotNum, recordEntry);

    fileHandle.getQFile()->unmap((uchar*)pageData);
    fileHandle.getQFile()->flush();

    return 0;
}

RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void* data)
{
    unsigned offset = 0;
    unsigned stringLength;
    bool notFound = true;

    void* record = malloc(recordDescriptor.capacity());
    instance()->readRecord(fileHandle, recordDescriptor, rid, record);

    for (unsigned i = 0; i < (unsigned) recordDescriptor.size() && notFound; i++)
    {
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                if(recordDescriptor[i].name == attributeName)
                {
                    memcpy(data, ((char*) record + offset), INT_SIZE);
                    notFound = false;
                }
                offset += INT_SIZE;

                break;
            case TypeReal:
                if(recordDescriptor[i].name == attributeName)
                {
                    memcpy(data, ((char*) record + offset), REAL_SIZE);
                    notFound = false;
                }
                offset += REAL_SIZE;

                break;
            case TypeVarChar:
                // First VARCHAR_LENGTH_SIZE bytes describe the varchar length.
                memcpy(&stringLength, ((char*) record + offset), VARCHAR_LENGTH_SIZE);
                offset += VARCHAR_LENGTH_SIZE;

                if(recordDescriptor[i].name == attributeName)
                {
                    // Gets the actual string.
                    memcpy(data, ((char*) record + offset), stringLength);

                    // Adds the string terminator.
                    ((char*)data)[stringLength] = '\0';
                    notFound = false;
                }

                offset += stringLength;
                break;
        }
    }

    free(record);
    return notFound;
}

RC RecordBasedFileManager::readRecordAttribute(void* record, const vector<Attribute> &recordDescriptor, const string attributeName, void *&data)
{
    unsigned offset = 0;
    unsigned stringLength;
    bool notFound = true;

    for (unsigned i = 0; i < (unsigned) recordDescriptor.size() && notFound; i++)
    {
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                if(recordDescriptor[i].name == attributeName)
                {
                    data = malloc(INT_SIZE);
                    memcpy(data, ((char*) record + offset), INT_SIZE);
                    notFound = false;
                }
                offset += INT_SIZE;

                break;
            case TypeReal:
                if(recordDescriptor[i].name == attributeName)
                {
                    data = malloc(REAL_SIZE);
                    memcpy(data, ((char*) record + offset), REAL_SIZE);
                    notFound = false;
                }
                offset += REAL_SIZE;

                break;
            case TypeVarChar:
                // First VARCHAR_LENGTH_SIZE bytes describe the varchar length.
                memcpy(&stringLength, ((char*) record + offset), VARCHAR_LENGTH_SIZE);
                offset += VARCHAR_LENGTH_SIZE;

                data = malloc(stringLength + 1);

                if(recordDescriptor[i].name == attributeName)
                {
                    // Gets the actual string.
                    memcpy(data, ((char*) record + offset), stringLength);

                    // Adds the string terminator.
                    ((char*)data)[stringLength] = '\0';
                    notFound = false;
                }

                offset += stringLength;
                break;
        }
    }

    return notFound;
}


RC RecordBasedFileManager::reorganizeFile(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor)
{
    for(unsigned i = 0; i < fileHandle.getNumberOfPages(); i++)
    {
        reorganizePage(fileHandle, recordDescriptor, i);
    }

    return 0;
}


RC RecordBasedFileManager::reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber)
{
    (void)recordDescriptor;

    // Retrieve the specific page.
    void * pageData = nullptr;
    if (fileHandle.readPage(pageNumber, pageData) != SUCCESS) return 1;

    // Get slotHeader info
    SlotDirectoryHeader* slotHeader = (SlotDirectoryHeader*)pageData;
    unsigned recordEntriesNumber = slotHeader->recordEntriesNumber;

    // Get array of slotDirectoryRecordEntrys
    SlotDirectoryRecordEntry* slotDirectoryRecordEntrys = (SlotDirectoryRecordEntry*)((char*)pageData + sizeof(SlotDirectoryHeader));

    // Slot #0 will be flush with the end of the page, so we start with slot #1.
    for(unsigned i = 0, prevOffset = PAGE_SIZE; i < recordEntriesNumber; i++)
    {
        if(!(slotDirectoryRecordEntrys[i].meta | 2))
        {
            if(prevOffset > slotDirectoryRecordEntrys[i].offset + slotDirectoryRecordEntrys[i].length)
            {
                unsigned newOffset = prevOffset - slotDirectoryRecordEntrys[i].length;
                if(prevOffset != newOffset)
                {
                    memmove((char*)pageData + newOffset, (char*)pageData + slotDirectoryRecordEntrys[i].offset, slotDirectoryRecordEntrys[i].length);
                }
                slotDirectoryRecordEntrys[i].offset = newOffset;
            }
            prevOffset = slotDirectoryRecordEntrys[i].offset;
        }
    }

    // Update slotHeader freeSpaceOffset to account for new space
    slotHeader->freeSpaceOffset = slotDirectoryRecordEntrys[recordEntriesNumber - 1].offset;

    fileHandle.getQFile()->unmap((uchar*)pageData);
    fileHandle.getQFile()->flush();

    return 0;
}

RC RecordBasedFileManager::createRecord(const vector<Attribute> &recordDescriptor, vector<const void*> &attributes, void *&data)
{
    unsigned size = 0;

    // Figure our how big record is going to be
    for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                size += INT_SIZE;
            break;
            case TypeReal:
                size += REAL_SIZE;
            break;
            case TypeVarChar:
                unsigned varcharSize = strlen((char*)attributes[i]);

                // We also have to account for the overhead given by that integer.
                size += INT_SIZE + varcharSize;
            break;
        }
    }

    data = malloc(size);
    unsigned offset = 0;

    // Copy data into new record
    for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                memcpy((char*)data + offset, attributes[i], INT_SIZE);

                offset += INT_SIZE;
            break;
            case TypeReal:
                memcpy((char*)data + offset, attributes[i], REAL_SIZE);

                offset += REAL_SIZE;
            break;
            case TypeVarChar:
                // Copy size
                unsigned varcharSize = strlen((char*)attributes[i]);
                memcpy((char*)data + offset, &varcharSize, INT_SIZE);

                offset += INT_SIZE;

                // Copy string data
                memcpy((char*)data + offset, attributes[i], varcharSize);

                offset += varcharSize;
            break;
        }
    }

    return 0;
}


// scan returns an iterator to allow the caller to go through the results one by one.
RC RecordBasedFileManager::scan(
    FileHandle &fileHandle,
    const vector<Attribute> &recordDescriptor,
    const string &conditionAttribute,
    const CompOp compOp,                  // comparision type such as "<" and "="
    const void *value,                    // used in the comparison
    const vector<string> &attributeNames, // a list of projected attributes
    RBFM_ScanIterator &rbfm_ScanIterator)
{
    // Construct a ScanIterator with the requested criteria
    return rbfm_ScanIterator.open(fileHandle, recordDescriptor, conditionAttribute, compOp, value, attributeNames);
}

RBFM_ScanIterator::RBFM_ScanIterator()
{
    current.pageNum = 0;
    current.slotNum = 0;
    numPages = 0;
    fileHandle = nullptr;
    fileMap = nullptr;
    compOp = NO_OP;
    value = nullptr;
}

RC RBFM_ScanIterator::open(FileHandle &fileHandle,
                           const vector<Attribute> &recordDescriptor,
                           const string &conditionAttribute,
                           const CompOp compOp,
                           const void *value,
                           const vector<string> &attributeNames)
{
    if(fileMap != nullptr)
    {
        return 1;
    }

    fileMap = fileHandle.getQFile()->map(PAGED_FILE_HEADER_STRING_LENGTH, fileHandle.getQFile()->size() - PAGED_FILE_HEADER_STRING_LENGTH);

    if(fileMap == nullptr)
    {
        return 2;
    }

    this->fileHandle = &fileHandle;

    // Save search criteria
    this->numPages = fileHandle.getNumberOfPages();
    this->conditionAttribute = conditionAttribute;
    this->compOp = compOp;
    this->value = value;
    this->attributeNames = attributeNames;
    this->recordDescriptor = recordDescriptor;
    for (unsigned i = 0; i < recordDescriptor.size(); ++i)
    {
        if (recordDescriptor[i].name == conditionAttribute)
        {
            this->attr = recordDescriptor[i];
        }
    }

    return 0;
}

RC RBFM_ScanIterator::projectAttributes(
        const vector<Attribute> &recordDescriptor,
        const vector<string> &attributeNames,
        void *data,
        void *projectedData)
{
    RecordBasedFileManager* rbfm = RecordBasedFileManager::instance();
    unsigned offset = 0;
    for (string attr : attributeNames)
    {
        void* attributeData = nullptr;
        if(rbfm->readRecordAttribute(data, recordDescriptor, attr, attributeData) != SUCCESS) return 1;
        int type;
        for (unsigned i = 0; i < recordDescriptor.size(); ++i)
        {
            if (recordDescriptor[i].name == attr)
            {
                type = recordDescriptor[i].type;
            }
        }
        unsigned length = 0;
        switch(type)
        {
        case TypeInt:
            length = INT_SIZE;
            break;
        case TypeReal:
            length = REAL_SIZE;
            break;
        case TypeVarChar:
            length = strlen((char*)attributeData);
            memcpy(((char*) projectedData + offset), &length, VARCHAR_LENGTH_SIZE);
            offset += VARCHAR_LENGTH_SIZE;
            break;
        }
        memcpy(((char*) projectedData + offset), attributeData, length);
        free(attributeData);
        offset += length;
    }
    return 0;
}

RC RBFM_ScanIterator::getNextRecord(RID &rid, void *data)
{
    RecordBasedFileManager* rbfm = RecordBasedFileManager::instance();
    void* record = malloc(rbfm->getMaxRecordSize(recordDescriptor));
    void* attrData = malloc(rbfm->getMaxAttributeSize(attr));
    for(;;)
    {
        rid = current;
        if(current.pageNum >= numPages)
        {
            free(attrData);
            free(record);
            return RBFM_EOF;
        }

        unsigned pageOffset = current.pageNum * PAGE_SIZE;
        char* currentPage = (char*)(fileMap + pageOffset);

        // Get slotHeader info
        SlotDirectoryHeader* slotHeader = (SlotDirectoryHeader*)currentPage;
        unsigned recordEntriesNumber = slotHeader->recordEntriesNumber;

        if(current.slotNum >= recordEntriesNumber)
        {
            current.pageNum++;
            current.slotNum = 0;
            continue;
        }

        // Get array of slotDirectoryRecordEntrys
        SlotDirectoryRecordEntry* slotDirectoryRecordEntrys = (SlotDirectoryRecordEntry*)(currentPage + sizeof(SlotDirectoryHeader));

        // Check for deleted or moved bits.
        char meta = slotDirectoryRecordEntrys[current.slotNum].meta;
        if(meta & 1 || meta & 2)
        {
            current.slotNum++;
            continue;
        }

        // Increment slot number for next call or iteration.
        current.slotNum++;

        rbfm->readRecord(*fileHandle, recordDescriptor, rid, record);
        rbfm->readRecordAttribute(record, recordDescriptor, conditionAttribute, attrData);
        if (compOp == NO_OP || this->compareAttribute(this->attr.type, attrData, value))
        {
            free(attrData);
            if (attributeNames.empty())
            {
                memcpy(data, record, rbfm->getRecordSize(recordDescriptor, record));
            }
            else
            {
                projectAttributes(recordDescriptor, attributeNames, record, data);
            }
            //memcpy(data, record, rbfm->getRecordSize(recordDescriptor, record));
            free(record);
            return SUCCESS;
        }
    }
    free(record);
    free(attrData);
    return RBFM_EOF;
}

bool RBFM_ScanIterator::compareInt(void* data, const void* value)
{
    int dataInt;
    int valueInt;
    memcpy (&dataInt, data, INT_SIZE);
    memcpy (&valueInt, value, INT_SIZE);
    switch (compOp)
    {
        case EQ_OP:  // =
            return dataInt == valueInt;
        case LT_OP:  // <
            return dataInt < valueInt;
        case GT_OP:  // >
            return dataInt > valueInt;
        case LE_OP:  // <=
            return dataInt <= valueInt;
        case GE_OP:  // >=
            return dataInt >= valueInt;
        case NE_OP:  // !=
            return dataInt != valueInt;
        case NO_OP:  // no condition
            return true;
    }
    return false;
}

bool RBFM_ScanIterator::compareReal(void* data, const void* value)
{
    float dataFloat;
    float valueFloat;
    memcpy (&dataFloat, data, REAL_SIZE);
    memcpy (&valueFloat, value, REAL_SIZE);
    switch (compOp)
    {
        case EQ_OP:  // =
            return dataFloat == valueFloat;
        case LT_OP:  // <
            return dataFloat < valueFloat;
        case GT_OP:  // >
            return dataFloat > valueFloat;
        case LE_OP:  // <=
            return dataFloat <= valueFloat;
        case GE_OP:  // >=
            return dataFloat >= valueFloat;
        case NE_OP:  // !=
            return dataFloat != valueFloat;
        case NO_OP:  // no condition
            return true;
    }
    return false;
}

bool RBFM_ScanIterator::compareVarChar(void* data, const void* value)
{
    string dataString, valueString;
    unsigned stringLength;
    void* tmpString;

    // Convert each field value to a string
    memcpy(&stringLength, (char*) data, VARCHAR_LENGTH_SIZE);
    tmpString = malloc(stringLength + 1);
    memcpy(tmpString, ((char*) value + VARCHAR_LENGTH_SIZE), stringLength);
    ((char*)tmpString)[stringLength] = '\0';
    dataString = string((char*) tmpString);

    memcpy(&stringLength, (char*) value, VARCHAR_LENGTH_SIZE);
    tmpString = malloc(stringLength + 1);
    memcpy(tmpString, ((char*) value + VARCHAR_LENGTH_SIZE), stringLength);
    ((char*)tmpString)[stringLength] = '\0';
    valueString = string((char*) tmpString);

    switch (compOp)
    {
        case EQ_OP:  // =
            return dataString == valueString;
        case LT_OP:  // <
            return dataString < valueString;
        case GT_OP:  // >
            return dataString > valueString;
        case LE_OP:  // <=
            return dataString <= valueString;
        case GE_OP:  // >=
            return dataString >= valueString;
        case NE_OP:  // !=
            return dataString != valueString;
        case NO_OP:  // no condition
            return true;
    }
    return false;
}

bool RBFM_ScanIterator::compareAttribute(AttrType attrType, void* data, const void* value)
{
    switch (attrType)
    {
        case TypeInt:
            return compareInt(data, value);
        case TypeReal:
            return compareReal(data, value);
        case TypeVarChar:
            return compareVarChar(data, value);
    }
    return false;
}

RC RBFM_ScanIterator::close()
{
    // Reset current position to allow reuse.
    current.pageNum = 0;
    current.slotNum = 0;

    // Unmap memory
    if(fileMap == nullptr)
    {
        return 1;
    }

    if(!fileHandle->getQFile()->unmap(fileMap))
    {
        return 2;
    }

    fileMap = nullptr;
    fileHandle = nullptr;

    return 0;
}
