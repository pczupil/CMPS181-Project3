#include "test_util.h"

void TEST_RM_0(const string &tableName)
{
    // Functions Tested
    // 1. Get Attributes
    cout << "****In Test Case 0****" << endl;

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
    cout<<"** Test Case 0 passed"<<endl;
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
    
    cout << endl << "Test Get Attributes .." << endl;
        
    // Get Attributes
    TEST_RM_0("tbl_employee");

    return 0;
}

