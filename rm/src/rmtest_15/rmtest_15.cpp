#include "test_util.h"

void TEST_RM_15(const string &tableName) {

    cout << "****In Test case 15****" << endl;

    RID rid;
    int tupleSize = 0;
    int numTuples = 500;
    void *tuple;
    void *returnedData = malloc(100);
    int ageVal = 25;

    RID rids[numTuples];
    vector<char *> tuples;

    RC rc = 0;
    int age;
    for(int i = 0; i < numTuples; i++)
    {
        tuple = malloc(100);

        // Insert Tuple
        float height = (float)i;

        age = (rand()%20) + 15;

        prepareTuple(6, "Tester", age, height, 123, tuple, &tupleSize);
        rc = rm->insertTuple(tableName, tuple, rid);
        assert(rc == success);

        tuples.push_back((char *)tuple);
        rids[i] = rid;
    }
    cout << "After Insertion!" << endl;

    // Set up the iterator
    RM_ScanIterator rmsi;
    string attr = "Age";
    vector<string> attributes;
    attributes.push_back(attr);
    rc = rm->scan(tableName, attr, GT_OP, &ageVal, attributes, rmsi);
    assert(rc == success);

    cout << "Scanned Data:" << endl;

    while(rmsi.getNextTuple(rid, returnedData) != RM_EOF)
    {
        cout << "Age: " << *(int *)returnedData << endl;
        assert ( (*(int *) returnedData) > ageVal );
    }
    rmsi.close();

    free(returnedData);
    for(int i = 0; i < numTuples; i++)
    {
        free(tuples[i]);
    }

    cout << "****Test case 15 passed****" << endl << endl;
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

    cout << endl << "Test Scan with conditions .." << endl;

	// Scan with conditions
    createTable("tbl_b_employee4");
    TEST_RM_15("tbl_b_employee4");

    return 0;
}
