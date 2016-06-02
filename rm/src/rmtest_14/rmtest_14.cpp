#include "test_util.h"

void TEST_RM_14(const string &tableName, vector<RID> &rids)
{
    // Functions Tested
    // 1. reorganize page
    // 2. delete tuples
    // 3. delete table
    cout << "****In Test case 14****" << endl;

	int numTuples = 2000;
    readRIDsFromDisk(rids, numTuples);

    RC rc;
    rc = rm->reorganizePage(tableName, rids[1000].pageNum);
    assert(rc == success);

    rc = rm->deleteTuples(tableName);
    assert(rc == success);

    rc = rm->deleteTable(tableName);
    assert(rc == success);

    cout << "****Test case 14 passed****" << endl << endl;
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

    cout << endl << "Test Delete Tuples and Delete Table  .." << endl;

    vector<RID> rids;

	// DeleteTuples/Table
    TEST_RM_14("tbl_employee4", rids);

    return 0;
}
