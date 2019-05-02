#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
extern "C" {
    #include "extmem.h"
}

#ifndef ADAPTER_H
#define ADAPTER_H

#define FAIL    -1
#define OK      0
#define BLK_START_ADDR  0
#define END_OF_FILE     0

typedef unsigned int    file_t;
typedef unsigned int    cursor_t;
typedef unsigned int    addr_t;
typedef unsigned char   blkData_t;

const int numOfBufBlock = 8;    // ��������block������
const int sizeOfAttr = 4;       // һ������ֵ�ĳ���
const int sizeOfRow = 8;        // һ����¼�ĳ���
const int numOfRowInBlk = 7;    // һ��block�п����ɵļ�¼����

const int addrOfLastRow = numOfRowInBlk * sizeOfRow;    // �������һ����¼�ĵ�ַ
const int endOfBlock = numOfRowInBlk * sizeOfRow + 8;   // ���ĩβ��ַ

// const int R_start = 1;			// ������R��ַ��1��ʼ
// const int S_start = 20;		    // ������S��ַ��20��ʼ
// const int R_size = 112;         // R��ϵ�ļ�¼����
// const int S_size = 224;         // S��ϵ�ļ�¼����
const int MAX_ATTR_VAL = 10000; // ���Ե����ֵ + 1

// ���̺��ڴ��һЩ����
const int ADDR_NOT_EXISTS = -1;
const int DEFAULT_ADDR = 0;
const int addrOfScan_1 = 100;   // ��һ������������ʼ��ַ
const int addrOfScan_2 = 200;   // �ڶ�������������ʼ��ַ

Buffer buff;   // ���建����

typedef struct Row {
    bool isFilled = false;
    int A, B;
    Row() {
        isFilled = false;
        A = MAX_ATTR_VAL, B = MAX_ATTR_VAL;
    }
    bool join_A(const Row &R) { return A == R.A; }
    bool operator>(const Row &R) { return A > R.A; }
    bool operator<(const Row &R) { return A < R.A; }
    bool operator==(const Row &R) { return ((A == R.A) && (B == R.B)); }
} row_t, index_t;

/**
 * @brief 
 * 
 */
class Block {
public:
    Block();
    void freeBlock();
    void loadFromDisk(addr_t addr, cursor_t endPos = addrOfLastRow);
    void writeInit(const file_t filename, int numOfRows = numOfRowInBlk);
    addr_t writeLastBlock();
    row_t getNewRow();
    addr_t writeRow(const row_t R);
    addr_t readNextAddr();

private:
    blkData_t *blkData;
    addr_t readBlkAddr, writeBlkAddr;
    cursor_t cursor;
    cursor_t endAddrOfData;
    void _writeAddr(unsigned int nextAddr);
    void _writeToDisk(addr_t addr);
};

typedef Block block_t;

/**
 * @brief ��¼���һЩ������Ϣ
 */
typedef struct Table {
    addr_t start;   // ��Ŀ�ʼ���̵�ַ
    addr_t end;     // ��Ľ������̵�ַ
    int size;       // ��ļ�¼������
    int rowSize;    // ��ļ�¼����
    Table(addr_t newStart = 0, int newSize = 0, addr_t newEnd = 0, int newRowSize = sizeOfRow): 
        start(newStart), end(newEnd), size(newSize), rowSize(newRowSize)
    {}
} table_t;

#endif // !ADAPTER_H
