#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <stdlib.h> 
#include <string.h>
#include <stdexcept>
#include <stdio.h> 

#include "pfm.h"
#include "rbfm.h"

using namespace std;

const int success = 0;
unsigned total = 0;

// Check if a file exists
bool FileExists(string fileName) {
	struct stat stFileInfo;

	if (stat(fileName.c_str(), &stFileInfo) == 0)
		return true;
	else
		return false;
}

void prepareLargeRecord2(const int index, void *buffer, int *size) {
	int offset = 0;

	// compute the count
	int count = index % 60 + 1;

	// compute the letter
	char text = index % 26 + 65;

	for (int i = 0; i < 10; i++) {
		// compute the floating number
		float real = (float) (index + 1);
		memcpy((char *) buffer + offset, &real, sizeof(float));
		offset += sizeof(float);

		// compute the integer
		memcpy((char *) buffer + offset, &index, sizeof(int));
		offset += sizeof(int);

		memcpy((char *) buffer + offset, &count, sizeof(int));
		offset += sizeof(int);

		for (int j = 0; j < count; j++) {
			memcpy((char *) buffer + offset, &text, 1);
			offset += 1;
		}

	}
	*size = offset;
}

void createLargeRecordDescriptor2(vector<Attribute> &recordDescriptor) {
	int index = 0;
	char *suffix = (char *) malloc(10);
	for (int i = 0; i < 10; i++) {
		Attribute attr;
		sprintf(suffix, "%d", index);
		attr.name = "attr";
		attr.name += suffix;
		attr.type = TypeReal;
		attr.length = (AttrLength) 4;
		recordDescriptor.push_back(attr);
		index++;

		sprintf(suffix, "%d", index);
		attr.name = "attr";
		attr.name += suffix;
		attr.type = TypeInt;
		attr.length = (AttrLength) 4;
		recordDescriptor.push_back(attr);
		index++;

		sprintf(suffix, "%d", index);
		attr.name = "attr";
		attr.name += suffix;
		attr.type = TypeVarChar;
		attr.length = (AttrLength) 60;
		recordDescriptor.push_back(attr);
		index++;

	}
	free(suffix);
}

int RBFTest_11c(RecordBasedFileManager *rbfm) {
	// Test rbfm functionality for project 2
	// Functions tested
	// 1. Delete all records from file
	cout << "****In RBF Test Case 11c****" << endl;

	RC rc;
	string fileName = "test_11c";

	// Open the file "test_11c"
	FileHandle fileHandle;
	rc = rbfm->createFile(fileName.c_str());
	if (rc != success) {
        cout << "Failed" <<endl;
		return -1;
	}
	rc = rbfm->openFile(fileName.c_str(), fileHandle);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);

	vector<Attribute> recordDescriptor;
	createLargeRecordDescriptor2(recordDescriptor);

	for (unsigned i = 0; i < recordDescriptor.size(); i++) {
		cout << "Attribute Name: " << recordDescriptor[i].name << " Type: "
				<< (AttrType) recordDescriptor[i].type << " Length: "
				<< recordDescriptor[i].length << endl;
	}

    // Create some records
	RID rid;
	void *record = malloc(1000);
	void *returnedData = malloc(1000);
	int numRecords = 10;

	vector<RID> rids;
	for (int i = 0; i < numRecords; i++) {
		// Test insert Record
		memset(record, 0, 1000);
		int size = 0;
		prepareLargeRecord2(i, record, &size);

        cout << "Inserting record " << i << endl;
        rbfm->printRecord(recordDescriptor, record);
		rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
		if (rc != success) {
			return -1;
		}
		assert(rc == success);

		rids.push_back(rid);
	}

    // Update record with data from srcRecord
    void* srcRecord = malloc(rbfm->getMaxRecordSize(recordDescriptor));
    rbfm->readRecord(fileHandle, recordDescriptor, rids[0], srcRecord);
    rbfm->readRecord(fileHandle, recordDescriptor, rids[numRecords - 1], record);
    rc = rbfm->updateRecord(fileHandle, recordDescriptor, srcRecord, rids[numRecords - 1]);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);
    free(record);
	record = malloc(1000);
    memset(record, 0, 1000);
    rbfm->readRecord(fileHandle, recordDescriptor, rids[numRecords - 1], record);
    assert(string((char*) record) == string((char*) srcRecord));

    // Delete record
    vector<RID>::iterator ridIterator = rids.begin() + 1;
    rc = rbfm->deleteRecord(fileHandle, recordDescriptor, *ridIterator);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);
    rids.erase(ridIterator);

    ridIterator = rids.end() - 2;
    rc = rbfm->deleteRecord(fileHandle, recordDescriptor, *ridIterator);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);
    rids.erase(ridIterator);

    // Reorganize the page
    rc = rbfm->reorganizePage(fileHandle, recordDescriptor, 0);
	if (rc != success) {
        cout << "Failed to reoganize page" << endl;
		return -1;
	}
    assert(rc == success);

    // Iterate over remaining records
    RBFM_ScanIterator scanIterator;
    Attribute attr = recordDescriptor[0];
    void* scanCriteriaValue = malloc(rbfm->getMaxAttributeSize(attr));
    void* updateData = malloc(rbfm->getMaxRecordSize(recordDescriptor));
    rbfm->readRecordAttribute(record, recordDescriptor, attr.name, updateData);
    rc = rbfm->scan(fileHandle, recordDescriptor, attr.name, NO_OP, scanCriteriaValue, {}, scanIterator);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);
    cout << "Scanning Remaining Records.." << endl;
    int recordNum = 0;
    while (scanIterator.getNextRecord(rid, updateData) != RBFM_EOF)
    {
        ++recordNum;
        cout << "Record " << recordNum << ": " << endl;
        rbfm->printRecord(recordDescriptor, updateData);
    }

    // Delete all records
	rc = rbfm->deleteRecords(fileHandle);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);

    // Close the file
    rc = rbfm->closeFile(fileHandle);
    if (rc != success) {
        return -1;
    }
    assert(rc == success);

    // Destroy the file
	rc = rbfm->destroyFile(fileName);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);
	free(record);
	free(returnedData);

	return 0;
}

int main(int argc, char* argv[]) {
    (void)argv;
    if(argc > 1)
    {
        cout << "Attach debugger and press enter to continue.\n";
        getchar();
    }

	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance(); // To test the functionality of the record-based file manager

	int rc = RBFTest_11c(rbfm);
	if (rc == 0) {
		cout << "Test Case 11c Passed!" << endl << endl;
	} else {
		cout << "Test Case 11c Failed!" << endl << endl;
	}

	return rc;
}
