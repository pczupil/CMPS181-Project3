#include <string>
#include <QFile>

#ifndef _pfm_h_
#define _pfm_h_

using namespace std;

typedef int RC;
typedef unsigned PageNum;

const int PAGE_SIZE                         = 4096;
#define PAGED_FILE_HEADER_STRING            "PAGED_FILE__"
const int PAGED_FILE_HEADER_STRING_LENGTH   = 12;

#define SUCCESS								0

class FileHandle;

class PagedFileManager
{
public:
    static PagedFileManager* instance();                     // Access to the _pf_manager instance

    RC createFile    (const char *fileName);                         // Create a new file
    RC destroyFile   (const char *fileName);                         // Destroy a file
    RC openFile      (const char *fileName, FileHandle &fileHandle); // Open a file
    RC closeFile     (FileHandle &fileHandle);                       // Close a file

protected:
    PagedFileManager();                                   // Constructor
    ~PagedFileManager();                                  // Destructor

private:
    static PagedFileManager *_pf_manager;

    // Auxiliary methods.
    bool FileExists(string fileName);
};


class FileHandle
{
public:
    FileHandle();                                                    // Default constructor
    FileHandle(char* fileName);                                      // Overloaded constructor
    ~FileHandle();                                                   // Destructor

    RC readPage(PageNum pageNum, void *&data);                           // Get a specific page
    RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC appendPage(const void *data);                                    // Append a specific page
    RC appendEmptyPage();                                               // Append an empty page
    unsigned getNumberOfPages();                                        // Get the number of pages in the file

    // Auxiliary methods.
    QFile* getQFile();

private:
    QFile _qfile;
};

 #endif
