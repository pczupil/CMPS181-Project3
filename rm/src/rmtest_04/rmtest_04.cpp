#include "test_util.h"

void TEST_RM_4(const string &tableName, const int nameLength, const string &name, const int age, const float height, const int salary)
{
    // Functions Tested
    // 1. Insert tuple
    // 2. Read Attributes **
    cout << "****In Test Case 4****" << endl;
    
    RID rid;    
    int tupleSize = 0;
    void *tuple = malloc(100);
    void *returnedData = malloc(100);
    
    // Test Insert Tuple 
    prepareTuple(nameLength, name, age, height, salary, tuple, &tupleSize);
    RC rc = rm->insertTuple(tableName, tuple, rid);
    assert(rc == success);

    // Test Read Attribute
    rc = rm->readAttribute(tableName, rid, "Salary", returnedData);
    assert(rc == success);
 
    cout << "Salary: " << *(int *)returnedData << endl;
    if (memcmp((char *)returnedData, (char *)tuple+18, 4) != 0)
    {
        cout << "****Test case 4 failed" << endl << endl;
    }
    else
    {
        cout << "****Test case 4 passed" << endl << endl;
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

    cout << endl << "Test Read Attributes .." << endl;

    // Read Attributes
    TEST_RM_4("tbl_employee", 6, "Veekay", 27, 171.4, 9000);

    return 0;
}
