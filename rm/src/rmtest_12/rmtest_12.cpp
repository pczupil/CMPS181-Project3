#include "test_util.h"

void TEST_RM_12(const string &tableName, vector<RID> &rids)
{
    // Functions Tested
    // 1. delete tuple
    // 2. read tuple
    cout << "****In Test case 12****" << endl;

    int numTuples = 2000;
    RC rc = 0;
    void * returnedData = malloc(1000);
    
    readRIDsFromDisk(rids, numTuples);

    // Delete the first 1000 tuples
    for(int i = 0; i < 1000; i++)
    {
        rc = rm->deleteTuple(tableName, rids[i]);
        assert(rc == success);

        rc = rm->readTuple(tableName, rids[i], returnedData);
        assert(rc != success);
    }
    cout << "After deletion!" << endl;

    for(int i = 1000; i < 2000; i++)
    {
        rc = rm->readTuple(tableName, rids[i], returnedData);
        assert(rc == success);
    }
    cout << "****Test case 12 passed****" << endl << endl;

    free(returnedData);
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

    cout << endl << "Test Delete Tuple .." << endl;

    vector<RID> rids;
    vector<int> sizes;

	// Delete Tuple
    TEST_RM_12("tbl_employee4", rids);

    return 0;
}
