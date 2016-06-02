#include "test_util.h"

void TEST_RM_3(const string &tableName, const int nameLength, const string &name, const int age, const float height, const int salary)
{
    // Functions Tested
    // 1. Insert Tuple    
    // 2. Update Tuple **
    // 3. Read Tuple
    cout << "****In Test Case 3****" << endl;
   
    RID rid; 
    int tupleSize = 0;
    int updatedTupleSize = 0;
    void *tuple = malloc(100);
    void *updatedTuple = malloc(100);
    void *returnedData = malloc(100);
   
    // Test Insert Tuple 
    prepareTuple(nameLength, name, age, height, salary, tuple, &tupleSize);
    RC rc = rm->insertTuple(tableName, tuple, rid);
    assert(rc == success);
    cout << "Original RID slot = " << rid.slotNum << endl;

    // Test Update Tuple
    prepareTuple(6, "Newman", age, height, 100, updatedTuple, &updatedTupleSize);
    rc = rm->updateTuple(tableName, updatedTuple, rid);
    assert(rc == success);
    cout << "Updated RID slot = " << rid.slotNum << endl;

    // Test Read Tuple 
    rc = rm->readTuple(tableName, rid, returnedData);
    assert(rc == success);
    cout << "Read RID slot = " << rid.slotNum << endl;
   
    // Print the tuples 
    cout << "Insert Data:" << endl; 
    printTuple(tuple, tupleSize);

    cout << "Updated data:" << endl;
    printTuple(updatedTuple, updatedTupleSize);

    cout << "Returned Data:" << endl;
    printTuple(returnedData, updatedTupleSize);
    
    if (memcmp(updatedTuple, returnedData, updatedTupleSize) == 0)
    {
        cout << "****Test case 3 passed****" << endl << endl;
    }
    else
    {
        cout << "****Test case 3 failed****" << endl << endl;
    }

    free(tuple);
    free(updatedTuple);
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

    cout << endl << "Test Update Tuple .." << endl;

    // Update Tuple
    TEST_RM_3("tbl_employee", 6, "Thomas", 28, 187.3, 4000);

    return 0;
}
