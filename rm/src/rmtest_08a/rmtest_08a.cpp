#include "test_util.h"

void TEST_RM_8_A(const string &tableName)
{
    // Functions Tested
    // 1. Simple scan **
    cout << "****In Test Case 8_A****" << endl;

    RID rid;    
    int tupleSize = 0;
    int numTuples = 100;
    void *tuple;
    void *returnedData = malloc(100);

    RID rids[numTuples];
    vector<char *> tuples;
    set<int> ages; 
    RC rc = 0;
    for(int i = 0; i < numTuples; i++)
    {
        tuple = malloc(100);

        // Insert Tuple
        float height = (float)i;
        int age = 20+i;
        prepareTuple(6, "Tester", age, height, 123, tuple, &tupleSize);
        ages.insert(age);
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
    rc = rm->scan(tableName, "", NO_OP, NULL, attributes, rmsi);
    assert(rc == success);

    cout << "Scanned Data:" << endl;
    
    while(rmsi.getNextTuple(rid, returnedData) != RM_EOF)
    {
        cout << "Age: " << *(int *)returnedData << endl;
        if (ages.find(*(int *)returnedData) == ages.end())
        {
            cout << "****Test case 8_A failed****" << endl << endl;
            rmsi.close();
            free(returnedData);
            for(int i = 0; i < numTuples; i++)
            {
                free(tuples[i]);
            }
            return;
        }
    }
    rmsi.close();

    free(returnedData);
    for(int i = 0; i < numTuples; i++)
    {
        free(tuples[i]);
    }
    cout << "****Test case 8_A passed****" << endl << endl; 
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

    cout << endl << "Test Simple Scan .." << endl;

     // Simple Scan
    TEST_RM_8_A("tbl_employee3");

    return 0;
}
