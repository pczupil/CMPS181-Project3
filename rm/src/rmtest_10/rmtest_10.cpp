#include "test_util.h"

void TEST_RM_10(const string &tableName, vector<RID> &rids, vector<int> &sizes)
{
    // Functions Tested:
    // 1. read tuple
    cout << "****In Test case 10****" << endl;

    int numTuples = 2000;
    void *tuple = malloc(1000);
    void *returnedData = malloc(1000);
    
    readRIDsFromDisk(rids, numTuples);
    readSizesFromDisk(sizes, numTuples);

    RC rc = 0;
    for(int i = 0; i < numTuples; i++)
    {
        memset(tuple, 0, 1000);
        memset(returnedData, 0, 1000);
        rc = rm->readTuple(tableName, rids[i], returnedData);
        assert(rc == success);

        int size = 0;
        prepareLargeTuple(i, tuple, &size);
        if(memcmp(returnedData, tuple, sizes[i]) != 0)
        {
            cout << "****Test case 10 failed****" << endl << endl;
            return;
        }
    }
    cout << "****Test case 10 passed****" << endl << endl;

    free(tuple);
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

    cout << endl << "Test Read Tuple .." << endl;

    vector<RID> rids;
    vector<int> sizes;

	// Read Tuple
    TEST_RM_10("tbl_employee4", rids, sizes);

    return 0;
}
