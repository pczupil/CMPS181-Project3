#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <fstream>
#include <iostream>
#include <cassert>

#include "rm.h"

using namespace std;

const int success = 0;

RelationManager *rm = RelationManager::instance();

// Function to prepare the data in the correct form to be inserted/read/updated.
void prepareTuple(const int nameLength, const string &name, const int age, const float height, const int salary, void *buffer, int *tupleSize)
{
    int offset = 0;

    memcpy((char *)buffer + offset, &(nameLength), sizeof(int));
    offset += sizeof(int);
    memcpy((char *)buffer + offset, name.c_str(), nameLength);
    offset += nameLength;

    memcpy((char *)buffer + offset, &age, sizeof(int));
    offset += sizeof(int);

    memcpy((char *)buffer + offset, &height, sizeof(float));
    offset += sizeof(float);

    memcpy((char *)buffer + offset, &salary, sizeof(int));
    offset += sizeof(int);

    *tupleSize = offset;
}


// Function to get the data in the correct form to be inserted/read after adding
// the attribute ssn
void prepareTupleAfterAdd(const int nameLength, const string &name, const int age, const float height, const int salary, const int ssn, void *buffer, int *tupleSize)
{
    int offset=0;

    memcpy((char*)buffer + offset, &(nameLength), sizeof(int));
    offset += sizeof(int);
    memcpy((char*)buffer + offset, name.c_str(), nameLength);
    offset += nameLength;

    memcpy((char*)buffer + offset, &age, sizeof(int));
    offset += sizeof(int);

    memcpy((char*)buffer + offset, &height, sizeof(float));
    offset += sizeof(float);

    memcpy((char*)buffer + offset, &salary, sizeof(int));
    offset += sizeof(int);

    memcpy((char*)buffer + offset, &ssn, sizeof(int));
    offset += sizeof(int);

    *tupleSize = offset;
}


// Function to parse the data in buffer
void printTuple(const void *buffer, const int tupleSize)
{
    (void)tupleSize;

    int offset = 0;
    cout << "****Printing Buffer: Start****" << endl;

    int nameLength = 0;
    memcpy(&nameLength, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "nameLength: " << nameLength << endl;

    char *name = (char *)malloc(100);
    memcpy(name, (char *)buffer+offset, nameLength);
    name[nameLength] = '\0';
    offset += nameLength;
    cout << "name: " << name << endl;

    int age = 0;
    memcpy(&age, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "age: " << age << endl;

    float height = 0.0;
    memcpy(&height, (char *)buffer+offset, sizeof(float));
    offset += sizeof(int);
    cout << "height: " << height << endl;

    int salary = 0;
    memcpy(&salary, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "salary: " << salary << endl;

    cout << "****Printing Buffer: End****" << endl << endl;
}


void printTupleAfterDrop( const void *buffer, const int tupleSize)
{
    (void)tupleSize;

    int offset = 0;
    cout << "****Printing Buffer: Start****" << endl;

    int nameLength = 0;
    memcpy(&nameLength, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "nameLength: " << nameLength << endl;

    char *name = (char *)malloc(100);
    memcpy(name, (char *)buffer+offset, nameLength);
    name[nameLength] = '\0';
    offset += nameLength;
    cout << "name: " << name << endl;

    int age = 0;
    memcpy(&age, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "age: " << age << endl;

    float height = 0.0;
    memcpy(&height, (char *)buffer+offset, sizeof(float));
    offset += sizeof(float);
    cout << "height: " << height << endl;

    cout << "****Printing Buffer: End****" << endl << endl;
}


void printTupleAfterAdd(const void *buffer, const int tupleSize)
{
    (void)tupleSize;

    int offset = 0;
    cout << "****Printing Buffer: Start****" << endl;

    int nameLength = 0;
    memcpy(&nameLength, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "nameLength: " << nameLength << endl;

    char *name = (char *)malloc(100);
    memcpy(name, (char *)buffer+offset, nameLength);
    name[nameLength] = '\0';
    offset += nameLength;
    cout << "name: " << name << endl;

    int age = 0;
    memcpy(&age, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "age: " << age << endl;

    float height = 0;
    memcpy(&height, (char *)buffer+offset, sizeof(float));
    offset += sizeof(float);
    cout << "height: " << height << endl;

    int ssn = 0;
    memcpy(&ssn, (char *)buffer+offset, sizeof(int));
    offset += sizeof(int);
    cout << "SSN: " << ssn << endl;

    cout << "****Printing Buffer: End****" << endl << endl;
}


// Create an employee table
void createTable(const string &tableName)
{
    cout << "****Create Table " << tableName << " ****" << endl;

    // 1. Create Table ** -- made separate now.
    vector<Attribute> attrs;

    Attribute attr;
    attr.name = "EmpName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    attrs.push_back(attr);

    attr.name = "Age";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    attrs.push_back(attr);

    attr.name = "Height";
    attr.type = TypeReal;
    attr.length = (AttrLength)4;
    attrs.push_back(attr);

    attr.name = "Salary";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    attrs.push_back(attr);

    int rc = rm->createTable(tableName, attrs);
    assert(rc == success);
    cout << "****Table Created: " << tableName << " ****" << endl << endl;
}

// delete an employee table
void deleteTable(const string &tableName)
{
    cout << "****Delete Table " << tableName << " ****" << endl;

  RC rc = rm->deleteTable(tableName);
    assert(rc == success);
    cout << "****Table Deleted: " << tableName << " ****" << endl << endl;
}

// Advanced Features:
void RM_TEST_EXTRA_1(const string &tableName, const int nameLength, const string &name, const int age, const int height, const int salary)
{
    // Functions Tested
    // 1. Insert tuple
    // 2. Read Attributes
    // 3. Drop Attributes **
    cout << "****In Extra Credit Test Case 1****" << endl;

    RID rid;
    int tupleSize = 0;
    void *tuple = malloc(100);
    void *returnedData = malloc(100);

    // Insert Tuple
    prepareTuple(nameLength, name, age, height, salary, tuple, &tupleSize);
    int rc = rm->insertTuple(tableName, tuple, rid);
    cout << "rc is " << rc << endl;
    assert(rc == success);

    // Read Attribute
    rc = rm->readAttribute(tableName, rid, "Salary", returnedData);
    assert(rc == success);
    cout << "Salary: " << *(int *)returnedData << endl;

    if(memcmp((char *)returnedData, (char *)tuple+18, 4) != 0)
    {
        cout << "Read attribute failed!" << endl;
    }
    else
    {
        //cout << "Read attribute passed!" << endl;

        // Drop the attribute
        rc = rm->dropAttribute(tableName, "Salary");
        assert(rc == success);

        // Read Tuple and print the tuple
        rc = rm->readTuple(tableName, rid, returnedData);
        assert(rc == success);
        printTupleAfterDrop(returnedData, tupleSize);
    }

    free(tuple);
    free(returnedData);

    cout<<"****Extra Credit Test Case 1 passed****"<<endl;
    return;
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

    string name1 = "Peters";
    string name2 = "Victors";

    // Extra Credits
    cout << "Test Extra Credit 1 .." << endl;
    // Drop Attribute
    createTable("tbl_employee100");
    RM_TEST_EXTRA_1("tbl_employee100", 6, name1, 24, 170, 5000);

    return 0;
}
