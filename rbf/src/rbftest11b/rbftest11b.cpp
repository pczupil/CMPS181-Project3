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

/* More Public Test Cases
 *  RBFTest_11b: Reading 10,000 records
 */
int RBFTest_11b(RecordBasedFileManager *rbfm) {
	// Functions tested
	// 1. Open Record-Based File
	// 2. Read Multiple Records
	// 3. Close Record-Based File
	cout << "****In RBF Test Case 11b****" << endl;

	RC rc;
	string fileName = "test_11a";

	// Open the file "test_11a"
	FileHandle fileHandle;
	rc = rbfm->openFile(fileName.c_str(), fileHandle);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);

	void *record = malloc(1000);
	void *returnedData = malloc(1000);
    unsigned numRecords = 10000;

	vector<Attribute> recordDescriptor;
	createLargeRecordDescriptor2(recordDescriptor);

	for (unsigned i = 0; i < recordDescriptor.size(); i++) {
		cout << "Attribute Name: " << recordDescriptor[i].name << " Type: "
                << (AttrType) recordDescriptor[i].type << " Length: " //<< endl;
                << recordDescriptor[i].length << endl;
	}

	vector<RID> rids;
	RID tempRID;

	// Read rids from the disk - do not use this code. This is not a page-based operation. For test purpose only.
	ifstream ridsFileRead("test_11a_rids", ios::in | ios::binary);

	unsigned pageNum;
	unsigned slotNum;

	if (ridsFileRead.is_open()) {
		ridsFileRead.seekg(0,ios::beg);
        for (unsigned i = 0; i < numRecords; i++) {
			ridsFileRead.read(reinterpret_cast<char*>(&pageNum), sizeof(unsigned));
			ridsFileRead.read(reinterpret_cast<char*>(&slotNum), sizeof(unsigned));
			if (i % 1000 == 0) {
				cout << "loaded RID #" << i << ": " << pageNum << ", " << slotNum << endl;
			}
			tempRID.pageNum = pageNum;
			tempRID.slotNum = slotNum;
			rids.push_back(tempRID);
		}
		ridsFileRead.close();
	}

	if (rids.size() != numRecords) {
		return -1;
	}

	// Compare records from the disk read with the record created from the method
    for (unsigned i = 0; i < numRecords; i++) {
		memset(record, 0, 1000);
		memset(returnedData, 0, 1000);
		rc = rbfm->readRecord(fileHandle, recordDescriptor, rids[i],
				returnedData);
		if (rc != success) {
			return -1;
		}
		assert(rc == success);

		int size = 0;
		prepareLargeRecord2(i, record, &size);
		if (memcmp(returnedData, record, size) != 0) {
			cout << "Test Case 11b Failed!" << endl << endl;
			free(record);
			free(returnedData);
			return -1;
		}
	}


	// Close the file
	rc = rbfm->closeFile(fileHandle);
	if (rc != success) {
		return -1;
	}
	assert(rc == success);

	free(record);
	free(returnedData);

	return 0;
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance(); // To test the functionality of the record-based file manager

	int rc = RBFTest_11b(rbfm);
	if (rc == 0) {
		cout << "Test Case 11b Passed!" << endl << endl;
	} else {
		cout << "Test Case 11b Failed!" << endl << endl;
	}

	return rc;
}
