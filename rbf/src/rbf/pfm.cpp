// CMPS 181 - Project 1
// Author:				Ian Gudger and Peter Czupil
// File description:	Implementation of a paged file manager.

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cstdio>
#include <cassert>

#include <QTextStream>

#include "pfm.h"

using namespace std;

// ------------------------------------------------------------
// PagedFileManager Class
// ------------------------------------------------------------

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
	// Singleton design pattern.
    if(!_pf_manager) _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const char *fileName)
{
    QFile file(QString::fromUtf8(fileName));

    // If file already exists, error.
    if (file.exists()) return 1;

    if(!file.open(QFile::ReadWrite)) return 2;

    QTextStream stream(&file);
    stream << PAGED_FILE_HEADER_STRING;
    //if(file.write(PAGED_FILE_HEADER_STRING) != PAGED_FILE_HEADER_STRING_LENGTH) return 3;

    file.flush();

    file.close();

	return 0;
}


RC PagedFileManager::destroyFile(const char *fileName)
{
    QFile file(QString::fromUtf8(fileName));

    // If file cannot be successfully removed, error.
    return !file.remove();
}


RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{
    fileHandle.getQFile()->setFileName(QString::fromUtf8(fileName));

    // Checks if fileHandle is already an handle for an open file.
    if(fileHandle.getQFile()->handle() != -1) return 1;

    // If file does not exist, error.
    if(!fileHandle.getQFile()->exists()) return 2;

    // Check if file is too small to possibly fit header.
    if(fileHandle.getQFile()->size() < PAGED_FILE_HEADER_STRING_LENGTH) return 3;

    if(!fileHandle.getQFile()->open(QFile::ReadWrite)) return 4;

    uchar* iomap = fileHandle.getQFile()->map(0, PAGED_FILE_HEADER_STRING_LENGTH);
    if (iomap == nullptr) return 5;

	// Check if the file is actually a paged file by looking at its header.
    if (memcmp((char*)iomap, PAGED_FILE_HEADER_STRING, PAGED_FILE_HEADER_STRING_LENGTH) != 0)
	{
        //cout << "header string = " << (char*)iomap << endl;
		// Not a page file, close it + error.
        fileHandle.getQFile()->unmap(iomap);
        fileHandle.getQFile()->close();
        return 6;
	}

    // Else, it is a page file. Unmap the memory.
    fileHandle.getQFile()->unmap(iomap);

	return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    // Check if fileHandle is actually an handle for an open file.
    if(fileHandle.getQFile()->handle() == -1) return 1;

	// Flushes the pending data.
    fileHandle.getQFile()->flush();

	// Closes the file.
    fileHandle.getQFile()->close();

	return 0;
}

// Checks if a file already exists.
bool PagedFileManager::FileExists(string fileName)
{
    QFile file(QString::fromStdString(fileName));
    return file.exists();
}

// ------------------------------------------------------------
// FileHandle Class
// ------------------------------------------------------------

FileHandle::FileHandle() : _qfile()
{
}

FileHandle::FileHandle(char* fileName) : _qfile(QString(fileName))
{
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *&data)
{
    // Check if the page exists.
    if (getNumberOfPages() < pageNum)
    	return 1;

    // Get mapped region.
    uchar* iomap = _qfile.map(PAGED_FILE_HEADER_STRING_LENGTH + PAGE_SIZE * pageNum, PAGE_SIZE);
    if (iomap == nullptr) return 2;

    // Update data pointer reference.
    data = (void*)iomap;

    return 0;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
	// Check if the page exists.
    if (getNumberOfPages() < pageNum) return 1;

    // Get mapped region.
    void* page = (void*)_qfile.map(PAGED_FILE_HEADER_STRING_LENGTH + PAGE_SIZE * pageNum, PAGE_SIZE);
    if (page == nullptr) return 2;

	// Write the page.
    memcpy(page, data, PAGE_SIZE);

    // Unmap page.
    _qfile.unmap((uchar*)page);
    _qfile.flush();

    return 0;
}


RC FileHandle::appendPage(const void *data)
{
    qint64 end = _qfile.size();


    // Resize file to fit new page;
    if(!_qfile.resize(end + PAGE_SIZE)) return 1;

    // Calling _qfile.size() after resize seems to be required.
    _qfile.flush();
    assert(_qfile.size() == end + PAGE_SIZE);

    // Get mapped region.
    void* page = (void*)_qfile.map(end, PAGE_SIZE);
    if (page == nullptr) return 2;

    // Write the page.
    memcpy(page, data, PAGE_SIZE);

    // Unmap page.
    _qfile.unmap((uchar*)page);
    _qfile.flush();

    return 0;
}

RC FileHandle::appendEmptyPage()
{
    qint64 end = _qfile.size();


    // Resize file to fit new page;
    if(!_qfile.resize(end + PAGE_SIZE)) return 1;

    // Calling _qfile.size() after resize seems to be required.
    _qfile.flush();
    assert(_qfile.size() == end + PAGE_SIZE);

    // Get mapped region.
    void* page = (void*)_qfile.map(end, PAGE_SIZE);
    if (page == nullptr) return 2;

    // Write the page.
    memset(page, 0, PAGE_SIZE);

    // Unmap page.
    _qfile.unmap((uchar*)page);
    _qfile.flush();

    return 0;
}


unsigned FileHandle::getNumberOfPages()
{
	// Number of pages is given by: (file size - paged file header size) / page size
    return (_qfile.size() - PAGED_FILE_HEADER_STRING_LENGTH) / PAGE_SIZE;
}

// Accessor methods.

QFile* FileHandle::getQFile()
{
    return &_qfile;
}

