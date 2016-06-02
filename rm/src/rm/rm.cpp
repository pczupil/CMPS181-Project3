// CMPS 181 - Project 2
// Author:              Ian Gudger and Peter Czupil
//

#include <cerrno>
#include <system_error>
#include <iostream>

#include "rm.h"

RelationManager* RelationManager::_rm = nullptr;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
    maxTableID = 0;
    string tableFileName = tablesTableName;
    tableFileName += TABLE_FILE_EXTENSION;
    string columnFileName = columnsTableName;
    columnFileName += TABLE_FILE_EXTENSION;

    PagedFileManager* pageFileManager = PagedFileManager::instance();
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();

    // Create files if they don't exist. We ignore the return codes here because we don't know if the files exist.
    pageFileManager->createFile(tableFileName.c_str());
    pageFileManager->createFile(columnFileName.c_str());

    // Create table and column Table objects
    _tablesTable = new Table(tablesTableName);
    _columnsTable = new Table(columnsTableName);

    // Open files
    if(pageFileManager->openFile(tableFileName.c_str(), *_tablesTable->fileHandle) != SUCCESS)
    {
        cerr << "Error opening table file.\n";
        throw system_error(error_code(EIO, generic_category()));
    }
    if(pageFileManager->openFile(columnFileName.c_str(), *_columnsTable->fileHandle) != SUCCESS)
    {
        cerr << "Error opening column file.\n";
        throw system_error(error_code(EIO, generic_category()));
    }

    // Initilize _tables RecordDescriptor
    _tablesTable->columns.push_back(Attribute {"table_id", TypeInt, 4});
    _tablesTable->columns.push_back(Attribute {"table_name", TypeVarChar, 256});
    _tablesTable->columns.push_back(Attribute {"file_name", TypeVarChar, 256});

    // Load tables
    void* recordData = malloc(recordBasedFileManager->getMaxRecordSize(_tablesTable->columns));
    RID rid;

    RBFM_ScanIterator tableScanner;
    if(tableScanner.open(*_tablesTable->fileHandle, _tablesTable->columns) != SUCCESS)
    {
        cerr << "Error opening table scanner.\n";
        throw system_error(error_code(EIO, generic_category()));
    }
    while(tableScanner.getNextRecord(rid, recordData) != RBFM_EOF)
    {
        void* attributeData = nullptr;
        if(recordBasedFileManager->readRecordAttribute(recordData, _tablesTable->columns, "table_id", attributeData) != SUCCESS)
        {
            cerr << "Error reading record attribute table_id.\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        unsigned tableID = *((unsigned*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        // Update maxTableID if needed so that at the end of loop we have the correct maxTableID
        if(tableID > maxTableID) maxTableID = tableID;

        if(recordBasedFileManager->readRecordAttribute(recordData, _tablesTable->columns, "table_name", attributeData) != SUCCESS)
        {
            cerr << "Error reading record attribute table_name.\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        string tableName((char*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        if(recordBasedFileManager->readRecordAttribute(recordData, _tablesTable->columns, "file_name", attributeData) != SUCCESS)
        {
            cerr << "Error reading record attribute file_name.\n";
            throw system_error(error_code(EIO, generic_category()));
        }

        Table* table = new Table(tableName);
        _tables.insert({maxTableID, table});
        _tableNames[tableName] = tableID;

        // Open table
        if(pageFileManager->openFile((char*)attributeData, *_tables[tableID]->fileHandle) != SUCCESS)
        {
            cerr << "Error opening table for for table " << tableName << ".\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        free(attributeData);
        attributeData = nullptr;
    }
    free(recordData);
    recordData = nullptr;

    if(tableScanner.close() != SUCCESS)
    {
        cerr << "Error closing table scanner.\n";
        throw system_error(error_code(EIO, generic_category()));
    }

    // Load Columns
    // Initilize _columns RecordDescriptor
    _columnsTable->columns.push_back(Attribute {"table_id", TypeInt, 4});
    _columnsTable->columns.push_back(Attribute {"column_name", TypeVarChar, 256});
    _columnsTable->columns.push_back(Attribute {"type", TypeInt, 4});
    _columnsTable->columns.push_back(Attribute {"length", TypeInt, 4});

    RBFM_ScanIterator columnScanner;
    if(columnScanner.open(*_columnsTable->fileHandle, _columnsTable->columns) != SUCCESS)
    {
        throw system_error(error_code(EIO, generic_category()));
    }
    recordData = malloc(recordBasedFileManager->getMaxRecordSize(_columnsTable->columns));
    while(columnScanner.getNextRecord(rid, recordData) != RBFM_EOF)
    {
        void* attributeData = nullptr;
        if(recordBasedFileManager->readRecordAttribute(recordData, _columnsTable->columns, "table_id", attributeData) != SUCCESS)
        {
            cerr << "Error reading column record attribute table_id.\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        unsigned tableID = *((unsigned*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        if(recordBasedFileManager->readRecordAttribute(recordData, _columnsTable->columns, "column_name", attributeData) != SUCCESS)
        {
            cerr << "Error reading column record attribute column_name for table " << tableID << ".\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        string columnName((char*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        if(recordBasedFileManager->readRecordAttribute(recordData, _columnsTable->columns, "type", attributeData) != SUCCESS)
        {
            cerr << "Error reading column record attribute type for table " << tableID << ".\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        AttrType columnType = *((AttrType*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        if(recordBasedFileManager->readRecordAttribute(recordData, _columnsTable->columns, "length", attributeData) != SUCCESS)
        {
            cerr << "Error reading column record attribute length for table " << tableID << ".\n";
            throw system_error(error_code(EIO, generic_category()));
        }
        AttrLength columnLength = *((AttrLength*)attributeData);
        free(attributeData);
        attributeData = nullptr;

        auto table = _tables.find(tableID);
        if(table == _tables.end())
        {
            cerr << "Error: Column " << columnName << " references table ID " << tableID << " which does not exist.\n";
            throw system_error(error_code(EIO, generic_category()));
        }

        table->second->columns.push_back(Attribute{columnName, columnType, columnLength});
    }
    free(recordData);
    recordData = nullptr;
    if(columnScanner.close() != SUCCESS)
    {
        cerr << "Error closing column scanner.\n";
        throw system_error(error_code(EIO, generic_category()));
    }
}

RelationManager::~RelationManager()
{
    PagedFileManager* pageFileManager = PagedFileManager::instance();

    delete _tablesTable;
    delete _columnsTable;

    for(auto table_it = _tables.begin(); table_it != _tables.end(); table_it++)
    {
        delete table_it->second;
    }
}


RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    cout << "Running createTable function for table " << tableName << ".\n";

    PagedFileManager* pageFileManager = PagedFileManager::instance();
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();

    string tableFileName = tableName;
    tableFileName += TABLE_FILE_EXTENSION;

    // If file exists for tableName, error
    if(pageFileManager->createFile(tableFileName.c_str())) return 1;

    // Otherwise update tableFile and columnFile
    maxTableID++;
    RID rid;

    // Update tableFile
    void* tableRecord = nullptr;
    vector<const void*> tableRecordAttributeDataList {&maxTableID, tableName.c_str(), tableFileName.c_str()};
    recordBasedFileManager->createRecord(_tablesTable->columns, tableRecordAttributeDataList, tableRecord);
    if(recordBasedFileManager->insertRecord(*_tablesTable->fileHandle, _tablesTable->columns, tableRecord, rid)) return 2;
    free(tableRecord);
    tableRecord = nullptr;

    // Update table maps
    Table* table = new Table(tableName);
    _tables.insert({maxTableID, table});
    _tableNames[tableName] = maxTableID;

    // Open new table file
    if(pageFileManager->openFile(tableFileName.c_str(), *_tables[maxTableID]->fileHandle) != SUCCESS) return 3;

    // Update column information
    for(unsigned i = 0; i < attrs.size(); i++)
    {
        // Update columnFile
        void* columnRecord = nullptr;
        vector<const void*> columnRecordAttributeDataList {&maxTableID, attrs[i].name.c_str(), &attrs[i].type, &attrs[i].length};
        recordBasedFileManager->createRecord(_columnsTable->columns, columnRecordAttributeDataList, columnRecord);
        if(recordBasedFileManager->insertRecord(*_columnsTable->fileHandle, _columnsTable->columns, columnRecord, rid)) return 2;
        free(columnRecord);
        columnRecord = nullptr;

        // Update columns vector
        _tables[maxTableID]->columns.push_back(Attribute{attrs[i].name, attrs[i].type, attrs[i].length});
    }

    return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    // Delete table's file
    PagedFileManager* pageFileManager = PagedFileManager::instance();
    if(pageFileManager->destroyFile(table->fileHandle->getQFile()->fileName().toStdString().c_str()) != SUCCESS) return 2;

    // Find records to delete
    RID rid;
    vector<RID> ridsToDelete;
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();
    const void* table_ID = (void*) &(*_tableNames.find(tableName)).second;

    // Scan tables file for records to delete.
    RBFM_ScanIterator tableScanner;
    if(recordBasedFileManager->scan(
                *_tablesTable->fileHandle,
                _rm->_tablesTable->columns,
                "table_id",
                EQ_OP,
                table_ID,
                vector<string>(),
                tableScanner) != SUCCESS) return 3;
    void* recordData = malloc(recordBasedFileManager->getMaxRecordSize(_tablesTable->columns));
    while(tableScanner.getNextRecord(rid, recordData) != RBFM_EOF)
    {
        ridsToDelete.push_back(rid);
    }
    free(recordData);
    recordData = nullptr;
    if(tableScanner.close() != SUCCESS) return 4;

    // Delete table rids flagged for deletion.
    while(ridsToDelete.size() > 0)
    {
        recordBasedFileManager->deleteRecord(*_tablesTable->fileHandle, _tablesTable->columns, ridsToDelete.back());
        ridsToDelete.pop_back();
    }

    // Scan _columns file for records to delete.
    RBFM_ScanIterator columnScanner;
    if(recordBasedFileManager->scan(
                *_columnsTable->fileHandle,
                _rm->_columnsTable->columns,
                "table_id",
                EQ_OP,
                table_ID,
                vector<string>(),
                columnScanner) != SUCCESS) return 5;
    recordData = malloc(recordBasedFileManager->getMaxRecordSize(_columnsTable->columns));
    while(columnScanner.getNextRecord(rid, recordData) != RBFM_EOF)
    {
        ridsToDelete.push_back(rid);
    }
    free(recordData);
    recordData = nullptr;
    if(columnScanner.close() != SUCCESS) return 6;

    // Delete column rids flagged for deletion.
    while(ridsToDelete.size() > 0)
    {
        recordBasedFileManager->deleteRecord(*_columnsTable->fileHandle, _columnsTable->columns, ridsToDelete.back());
        ridsToDelete.pop_back();
    }

    // Delete data from maps.
    _tables.erase(_tableNames[tableName]);
    _tableNames.erase(tableName);

    return 0;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS)
    {
        if(tableName == tablesTableName)
        {
            attrs = _tablesTable->columns;
            return 0;
        }
        else if(tableName == columnsTableName)
        {
            attrs = _columnsTable->columns;
            return 0;
        }
        else
        {
            return 1;
        }
    }

    attrs = table->columns;

    return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    vector<Attribute> attrs;
    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(RecordBasedFileManager::instance()->insertRecord(*table->fileHandle, attrs, data, rid) != SUCCESS) return 3;

    return 0;
}

RC RelationManager::deleteTuples(const string &tableName)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    if(RecordBasedFileManager::instance()->deleteRecords(*table->fileHandle) != SUCCESS) return 2;

    return 0;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    vector<Attribute> attrs;
    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(RecordBasedFileManager::instance()->deleteRecord(*table->fileHandle, attrs, rid) != SUCCESS) return 3;

    return 0;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    vector<Attribute> attrs;
    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(RecordBasedFileManager::instance()->updateRecord(*table->fileHandle, attrs, data, rid) != SUCCESS) return 3;

    return 0;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    Table* table = nullptr;
    if(getReadOnlyTable(tableName, table) != SUCCESS) return 1;

    vector<Attribute> attrs;
    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(RecordBasedFileManager::instance()->readRecord(*table->fileHandle, attrs, rid, data) != SUCCESS) return 3;

    return 0;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    Table* table = nullptr;
    if(getReadOnlyTable(tableName, table) != SUCCESS) return 1;

    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();

    if(recordBasedFileManager->readAttribute(*table->fileHandle, table->columns, rid, attributeName, data) != SUCCESS) return 2;

    return 0;
}

RC RelationManager::reorganizePage(const string &tableName, const unsigned pageNumber)
{
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();
    vector<Attribute> attrs;

    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(recordBasedFileManager->reorganizePage(*table->fileHandle, attrs, pageNumber) != SUCCESS) return 3;

    return 0;
}

// Extra credit
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
    return -1;
}

// Extra credit
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
    return -1;
}

// Extra credit
RC RelationManager::reorganizeTable(const string &tableName)
{
    RecordBasedFileManager* recordBasedFileManager = RecordBasedFileManager::instance();
    vector<Attribute> attrs;

    Table* table = nullptr;
    if(getTable(tableName, table) != SUCCESS) return 1;

    if(getAttributes(tableName, attrs) != SUCCESS) return 2;

    if(recordBasedFileManager->reorganizeFile(*table->fileHandle, attrs) != SUCCESS) return 3;

    return 0;
}

RC RelationManager::getTable(const string &tableName, Table* &table)
{
    auto tableID = _tableNames.find(tableName);
    if(tableID == _tableNames.end()) return 1;

    table = _tables[tableID->second];

    return 0;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,
      const void *value,
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    // Resolve tableName to fileHandle
    Table* table = nullptr;
    if(getReadOnlyTable(tableName, table) != SUCCESS) return 1;

    RBFM_ScanIterator* rbfm_scanIt = rm_ScanIterator.getRbfmScanIt();

    RecordBasedFileManager::instance()->scan(
                *table->fileHandle,
                table->columns,
                conditionAttribute,
                compOp,
                value,
                attributeNames,
                *rbfm_scanIt);

    return 0;
}

//RM_ScanIterator
RC RM_ScanIterator::close()
{
    if(this->rbfm_scanIt.close() != SUCCESS) return 1;

    return 0;
}

RC RM_ScanIterator::getNextTuple(RID &rid, void *data)
{
    return this->rbfm_scanIt.getNextRecord(rid, data);
}

RC RelationManager::getReadOnlyTable(const string &tableName, Table* &table)
{
    if(getTable(tableName, table) != SUCCESS)
    {
        if(tableName == tablesTableName)
        {
            table = _tablesTable;
        }
        else if(tableName == columnsTableName)
        {
            table = _columnsTable;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}
