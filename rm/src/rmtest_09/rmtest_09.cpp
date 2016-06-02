#include "test_util.h"

void TEST_RM_9(const string &tableName, vector<RID> &rids, vector<int> &sizes)
{
    // Functions Tested:
    // 1. getAttributes
    // 2. insert tuple
    cout << "****In Test case 9****" << endl;

    RID rid; 
    void *tuple = malloc(1000);
    int numTuples = 2000;

    // GetAttributes
    vector<Attribute> attrs;
    RC rc = rm->getAttributes(tableName, attrs);
    assert(rc == success);
	
    for(unsigned i = 0; i < attrs.size(); i++)
    {
        cout << "Attribute Name: " << attrs[i].name << endl;
        cout << "Attribute Type: " << (AttrType)attrs[i].type << endl;
        cout << "Attribute Length: " << attrs[i].length << endl << endl;
    }

    // Insert 2000 tuples into table
    for(int i = 0; i < numTuples; i++)
    {
        // Test insert Tuple
        int size = 0;
        memset(tuple, 0, 1000);
        prepareLargeTuple(i, tuple, &size);

        rc = rm->insertTuple(tableName, tuple, rid);
        assert(rc == success);

        rids.push_back(rid);
        sizes.push_back(size);        
    }
    cout << "****Test case 9 passed****" << endl << endl;
    free(tuple);
    writeRIDsToDisk(rids);
    writeSizesToDisk(sizes);
}

int main(int argc, char* argv[])
{
	(void)argv;
	if(argc > 1)
	{
		cout << "Attach debugger and press enter to continue.\n";
		getchar();
	}

    cout << endl << "Test Insert Tuple .." << endl;

    vector<RID> rids;
    vector<int> sizes;

	// Insert Tuple
    TEST_RM_9("tbl_employee4", rids, sizes);

    return 0;
}
