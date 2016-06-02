
#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>
#include <unordered_map>

#include "../../../rbf/src/rbf/rbfm.h"

using namespace std;


#define RM_EOF (-1)                   // end of a scan operator
#define TABLE_FILE_EXTENSION ".bitch" // Binary Iterable Table and Column Holder

// RM_ScanIterator is an iteratr to go through tuples
class RM_ScanIterator
{
public:
  RM_ScanIterator() {}
  ~RM_ScanIterator() {}

  // "data" follows the same format as RelationManager::insertTuple()
  RC getNextTuple(RID &rid, void *data);
  RC close();
  RBFM_ScanIterator* getRbfmScanIt() {return &rbfm_scanIt;}
private:
  RBFM_ScanIterator rbfm_scanIt;
};

struct Table
{
    string name;
    FileHandle* fileHandle;
    vector<Attribute> columns;

    Table() { fileHandle = new FileHandle(); }
    Table(string tableName) : name(tableName) { fileHandle = new FileHandle(); }
    ~Table() { PagedFileManager::instance()->closeFile(*fileHandle); delete fileHandle; }
};


// Relation Manager
class RelationManager
{
public:
  static RelationManager* instance();

  RC createTable(const string &tableName, const vector<Attribute> &attrs);

  RC deleteTable(const string &tableName);

  RC getAttributes(const string &tableName, vector<Attribute> &attrs);

  RC insertTuple(const string &tableName, const void *data, RID &rid);

  RC deleteTuples(const string &tableName);

  RC deleteTuple(const string &tableName, const RID &rid);

  // Assume the rid does not change after update
  RC updateTuple(const string &tableName, const void *data, const RID &rid);

  RC readTuple(const string &tableName, const RID &rid, void *data);

  RC readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data);

  RC reorganizePage(const string &tableName, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one.
  RC scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RM_ScanIterator &rm_ScanIterator);


// Extra credit
public:
  RC dropAttribute(const string &tableName, const string &attributeName);

  RC addAttribute(const string &tableName, const Attribute &attr);

  RC reorganizeTable(const string &tableName);

  RC getTable(const string &tableName, Table* &table);

  RC getReadOnlyTable(const string &tableName, Table* &table);



protected:
  RelationManager();
  ~RelationManager();

private:
  static RelationManager*_rm;
  unordered_map<unsigned, Table*> _tables;
  unordered_map<string, unsigned> _tableNames;
  Table* _tablesTable;
  Table* _columnsTable;
  unsigned maxTableID;
  const string tablesTableName = "Tables";
  const string columnsTableName = "Columns";
};

#endif
