#include "test_util.h"

void TEST_RM_1(const string &tableName, const int nameLength, const string &name, const int age, const float height, const int salary)
{
    // Functions tested
    // 1. Insert Tuple **
    // 2. Read Tuple **
    // NOTE: "**" signifies the new functions being tested in this test case. 
    cout << "****In Test Case 1****" << endl;
   
    RID rid; 
    int tupleSize = 0;
    void *tuple = malloc(100);
    void *returnedData = malloc(100);

    // Insert a tuple into a table
    prepareTuple(nameLength, name, age, height, salary, tuple, &tupleSize);
    cout << "Insert Data:" << endl;
    printTuple(tuple, tupleSize);
    RC rc = rm->insertTuple(tableName, tuple, rid);
    assert(rc == success);
    
    // Given the rid, read the tuple from table
    rc = rm->readTuple(tableName, rid, returnedData);
    assert(rc == success);

    cout << "Returned Data:" << endl;
    printTuple(returnedData, tupleSize);

    // Compare whether the two memory blocks are the same
    if(memcmp(tuple, returnedData, tupleSize) == 0)
    {
        cout << "****Test case 1 passed****" << endl << endl;
    }
    else
    {
        cout << "****Test case 1 failed****" << endl << endl;
    }

    free(tuple);
    free(returnedData);
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
    
    cout << endl << "Test Insert/Read Tuple .." << endl;

    // Insert/Read Tuple
    TEST_RM_1("tbl_employee", 6, "Peters", 24, 170.1, 5000);

    return 0;
}
