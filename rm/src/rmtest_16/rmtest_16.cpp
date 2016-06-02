#include "test_util.h"

void TEST_RM_16(const string &tableName)
{
    cout << "****In Test Case 16****" << endl;

    // Get Catalog Attributes
    vector<Attribute> attrs;
    RC rc = rm->getAttributes(tableName, attrs);
    assert(rc == success);

    // print attribute name
    cout << "Catalog schema: (";
    for(unsigned i = 0; i < attrs.size(); i++)
    {
        if (i < attrs.size() - 1) cout << attrs[i].name << ", ";
        else cout << attrs[i].name << ")" << endl << endl;
    }

    RID rid;
    void *returnedData = malloc(100);

    // Set up the iterator
    RM_ScanIterator rmsi;
    vector<string> projected_attrs;
    for (unsigned i = 0; i < attrs.size(); i++){
      projected_attrs.push_back(attrs[i].name);
    }

    rc = rm->scan(tableName, "", NO_OP, NULL, projected_attrs, rmsi);
    assert(rc == success);

    while(rmsi.getNextTuple(rid, returnedData) != RM_EOF)
    {
        rbfm->printRecord(attrs, returnedData);
    }
    rmsi.close();

    free(returnedData);
    cout<<"** Test case 16 is over"<<endl;
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

    // NOTE: your Tables table must be called "Tables"
    string catalog_table_name = "Tables";

    cout << endl << "Test Catalog Information .." << endl;

    // Test Catalog Information
    TEST_RM_16(catalog_table_name);

    return 0;
}
