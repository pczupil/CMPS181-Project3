#include "test_util.h"

int TEST_RM_8_B(const string &tableName)
{
    // Functions Tested
    // 1. Simple scan **
    cout << "****In Test Case 8_B****" << endl;

    RID rid;    
    int numTuples = 100;
    void *returnedData = malloc(100);

    set<int> ages; 
    RC rc = 0;
    for(int i = 0; i < numTuples; i++)
    {
        int age = 20+i;
        ages.insert(age);
    }

    // Set up the iterator
    RM_ScanIterator rmsi;
    string attr = "Age";
    vector<string> attributes;
    attributes.push_back(attr);
    rc = rm->scan(tableName, "", NO_OP, NULL, attributes, rmsi);
    if(rc != success) {
        cout << "****Test case 8_B failed****" << endl << endl;
        return -1;
    }

    cout << "Scanned Data:" << endl;
    
    while(rmsi.getNextTuple(rid, returnedData) != RM_EOF)
    {
        cout << "Age: " << *(int *)returnedData << endl;
        if (ages.find(*(int *)returnedData) == ages.end())
        {
            cout << "****Test case 8_B failed****" << endl << endl;
            rmsi.close();
            free(returnedData);
            return -1;
        }
    }
    rmsi.close();
    
    // Delete a Table
    rc = rm->deleteTable(tableName);
    if(rc != success) {
        cout << "****Test case 8_B failed****" << endl << endl;
        return -1;
    }

    free(returnedData);
    cout << "****Test case 8_B passed****" << endl << endl; 
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

    cout << endl << "Test Simple Scan .." << endl;

     // Simple Scan
    TEST_RM_8_B("tbl_employee3");

    return 0;
}
