#include "Block.h"
#include "Block.cpp"
#include <cmath>
#pragma once

const int     R_start = 1;			// ������R��ַ��1��ʼ
const int     S_start = 20;		    // ������S��ַ��20��ʼ
const int     R_size = 112;         // R��ϵ�ļ�¼����
const int     S_size = 224;         // S��ϵ�ļ�¼����
const table_t table_R = {R_start, R_size};
const table_t table_S = {S_start, S_size};

/** 
 * ����¼��A�ֶ�ֵ����ɢ��
 * 
 * R: ��ɢ�еļ�¼
 * numOfBuckets: ɢ��Ͱ������
 * 
 * return: �ü�¼��ɢ�н����������Ͱ�ı�ţ�
 */
int hashRowsByA(row_t R, int numOfBuckets) { return R.A % numOfBuckets; }

/**
 * argmin��ʵ��
 * 
 * t[]: ��argmin������
 * size: ����Ĵ�С
 * 
 * return: ����t����Сֵ���±�
 */
template <typename T>
int argmin(T t[], int size) {
    int min = 0;
    for (int i = 0; i < size; ++i) {
        if (t[i] < t[min])
            min = i;
    }
    return min;
}

/**
 * ��������
 * 
 * array[]: �����������
 * size: ����������Ĵ�С
 */
template <typename T>
void insertSort(T array[], int size) {
    T tmp;
    int i, j;
    for (i = 1; i < size; ++i) {
        tmp = array[i];
        for (j = i; j > 0 && array[j - 1] > tmp; --j)
            array[j] = array[j - 1];
        array[j] = tmp;
    }
}

/**
 * ����һ���¼�����ÿһ����¼������
 * 
 * t[]: ��Ҫ����ļ�¼��
 * size: ��¼������
 * matchVal: ƥ��ֵ��ͨ�����ֶ����������ʾƥ��ļ�¼
 */
void printRows(row_t t[], int size, int matchVal = -1) {
    for (int i = 0; i < size; ++i) {
        printf("t[%d]: (%d, %d)\t", i, t[i].A, t[i].B);
        if (t[i].A == matchVal)
            printf("<-Matched here");
        printf("\n");
    }
    printf("\n");
}



/**
 * ���������̿��д�ŵ�����
 * 
 * resStartAddr: ������̿����ʼ��ַ
 * newSizeOfRow: ��¼�ĳ��ȣ����ڿ��ƻ��з���λ��
 */
void showResult(addr_t resStartAddr, int newSizeOfRow = sizeOfRow) {
    float timesToStandartRow = 1.0 * newSizeOfRow / sizeOfRow;
    int numOfRows = floor(numOfRowInBlk / timesToStandartRow);
    int numOfStandardRows = timesToStandartRow * numOfRows;
    // ������һЩ���������õ��Ĳ���
    bool rowStart, rowEnd, timesSmallerThanOne = (timesToStandartRow < 1);
    char delim = (timesSmallerThanOne) ? '\n' : '\t';
    int base = (timesSmallerThanOne) ? 1 : (int)timesToStandartRow;

    block_t readBlk;
    readBlk.loadFromDisk(resStartAddr, sizeOfRow * numOfStandardRows);
    row_t t[numOfStandardRows];
    int readRows, count = 0;
    printf("\n------------ ������ ------------\n");
    int curRows = 0;
    while(1) {
        readRows = read_N_Rows_From_1_Block(readBlk, t, numOfStandardRows);
        // ��������IO����������������IO����������Ҫ��ȥ
        // ���ﻹ�����˽���������洢�ģ��ҳ����һ�����������鶼��д����
        buff.numIO -= ceil(1.0 * readRows / numOfRowInBlk);
        count += readRows;
        if (timesSmallerThanOne) {
            rowStart = true;
            rowEnd = true;
        }
        for (int i = 0; i < readRows; ++i, ++curRows) {
            if (!timesSmallerThanOne) {
                rowStart = ((curRows % (int)timesToStandartRow) == 0);
                rowEnd = (((i + 1) % (int)timesToStandartRow) == 0);
            }
            if (rowStart)
                printf("%d\t", curRows / base + 1);
            printf("%d%c", t[i].A, delim);
            if (timesSmallerThanOne)
                printf("%d\t", ++curRows / base + 1);
            printf("%d%c", t[i].B, delim);
            if (rowEnd && !timesSmallerThanOne)
                printf("\n");
        }
        if (readRows < numOfStandardRows)
            break;
    }

    printf("----------------------------------\n");
    printf(" ��%d�н��\n\n", curRows / base);
    readBlk.freeBlock();
}


/**
 * һ��ɨ����м䲽�衪������ɢ��
 * �Ӵ����ж�ȡ���ݵ��������ϣ�ɢ�г������鲢��ÿ��ɢ�н���洢�ش�����
 * 
 * numOfSubTables: �������ϵ����ӱ�����
 * scan_1_index: �ñ�һ��ɨ�����洢����ʼ��ַ
 * startIndex: �ñ�һ��ɨ����������̿����ʼ��ַ
 */
void scan_1_HashToBucket(int numOfBuckets, addr_t startIndex, addr_t scan_1_index[]) {
    int numOfRows = sizeOfRow;
    row_t R_data[numOfRows], R_Empty;
    R_Empty.A = MAX_ATTR_VAL, R_Empty.B = MAX_ATTR_VAL;
    block_t readBlk, bucketBlk[numOfBuckets];
    readBlk.loadFromDisk(startIndex);
    for (int i = 0; i < numOfBuckets; ++i)
        bucketBlk[i].writeInit(scan_1_index[i]);

    int readRows;
    while(1) {
        readRows = read_N_Rows_From_1_Block(readBlk, R_data, numOfRows);
        for (int k = 0; k < numOfRows; ++k) {
            int hashVal = hashRowsByA(R_data[k], numOfBuckets);
            bucketBlk[hashVal].writeRow(R_data[k]);
        }
        if (readRows < numOfRows) {
            addr_t endAddr;
            for (int i = 0; i < numOfBuckets; ++i) {
                endAddr = bucketBlk[i].writeLastBlock();
            }
            break;
        }
    }
}

/**
 * һ��ɨ����м䲽�衪����������
 * �Ӵ����ж�ȡ���ݵ��������ϣ����򲢽��������洢�ش�����
 * ͬʱ����¼����������еõ���ÿ���ӱ���׵�ַ
 * 
 * numOfSubTables: �������ϵ����ӱ�����
 * scan_1_index: �ñ�һ��ɨ�����洢����ʼ��ַ
 * startIndex: �ñ�һ��ɨ����������̿����ʼ��ַ
 * 
 * return: һ��ɨ�������ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t scan_1_PartialSort(int numOfSubTables, addr_t scan_1_index, addr_t startIndex, int sizeOfSubTable) {
    unsigned int numOfUsedBlk = numOfBufBlock;
    addr_t curAddr = scan_1_index;
    block_t readBlk[numOfUsedBlk];
    row_t R_data[sizeOfSubTable];
    for (int k = 0; k < numOfSubTables; ++k) {
        // printf("** ��ʼ��%d���ӱ������ **\n", k + 1);
        // һ�δӴ����ж�8���������Ϊһ���ӱ�
        addr_t nextStart = startIndex + 8 * k;
        for (int i = 0; i < numOfUsedBlk; ++i) {
            readBlk[i].loadFromDisk(nextStart);
            nextStart = (int)readBlk[i].readNextAddr();	// ȡ��һ���ַ��next��
        }
        // ��ȡ��8���е�����
        for (int i = 0; i < numOfUsedBlk; ++i) {
            for (int j = 0, count; j < numOfRowInBlk; ++j) {
                count = i * numOfRowInBlk + j;
                R_data[count] = readBlk[i].getNewRow();
            }
            readBlk[i].freeBlock();
            readBlk[i].writeInit(scan_1_index + 8 * k + i);
        }
        // �Ե�ǰ�ӱ��������
        insertSort<row_t>(R_data, sizeOfSubTable);
        // ��������ӱ�д�ش���
        for (int i = 0; i < numOfUsedBlk; ++i) {
            for (int j = 0, count; j < numOfRowInBlk; ++j) {
                count = i * numOfRowInBlk + j;
                readBlk[i].writeRow(R_data[count]);
            }
            curAddr += 1;
            addr_t writeAddr = curAddr;
            if (i == numOfUsedBlk - 1)
                writeAddr = 0;
            readBlk[i].writeToDisk(writeAddr);
        }
    }
    return curAddr;
}

/**
 * ����ɨ����м䲽�衪�����ڹ鲢����
 * �Ӵ����ж�ȡһ���������з�����������ݵ���������
 * �����ܱ����򲢽��������洢�ش�����
 * 
 * numOfSubTables: �������ϵ����ӱ�����
 * scan_1_index: �ñ�һ��ɨ�����洢����ʼ��ַ
 * scan_2_index: �ñ����ɨ�����洢����ʼ��ַ
 * 
 * return: ����ɨ�������ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t scan_2_SortMerge(int numOfSubTables, addr_t scan_1_index, addr_t scan_2_index) {
    addr_t resultAddr = scan_2_index;
    block_t readBlk[numOfSubTables];
    block_t bucketBlk;
    Row rtemp, rFirst[numOfSubTables];  // ��¼ÿ��ĵ�һ��Ԫ��
    bucketBlk.writeInit(resultAddr);
    // ��װ�ص�ǰ��ϵ�������ӱ��еĵ�һ��
    for (int i = 0; i < numOfSubTables; ++i) {
        addr_t curAddr = scan_1_index + i * 8;
        readBlk[i].loadFromDisk(curAddr);
        rFirst[i] = readBlk[i].getNewRow();
        if (rFirst[i].isFilled == false)
            return FAIL;
    }
    
    int arg = argmin(rFirst, numOfSubTables);
    while(1) {
        // �ӵ�ǰλ��ÿ������ǰ��ļ�¼�У�ȡ�����ֶ�ֵ��С��һ����¼���±�
        // ��argΪ�ü�¼���ڵ��ӱ���;
        rtemp = rFirst[arg];
        // printf("arg: %d\trtemp: %d\t\t", arg, rtemp.A);
        // for (int i = 0; i < numOfSubTables; ++i)
        //     printf("rFirst[%d]: %d\t", i, rFirst[i].A);
        // printf("\n");
        rFirst[arg] = readBlk[arg].getNewRow();
        arg = argmin(rFirst, numOfSubTables);
        if (rtemp.isFilled == false) {
            // �����ӱ��Ѷ���
            resultAddr += numOfBufBlock * numOfSubTables - 1;
            int isLastBlock = bucketBlk.writeLastBlock();
            if (isLastBlock) {
                // ������л���ʣ�������
                resultAddr += 1;
            }
            break;
        }
        bucketBlk.writeRow(rtemp);
    }
    // showResult(scan_2_index);
    return resultAddr;
}

/**
 * ȥ�ز���
 * 
 * tableStartAddr: ��ȥ�صı����ʼ��ַ
 */
void tableDistinct(addr_t tableStartAddr) {
    int numOfUsedBlock = numOfBufBlock - 1;
    int numOfRows = numOfRowInBlk * numOfUsedBlock;
    addr_t readAddr = tableStartAddr, resAddr = tableStartAddr;
    
    block_t readBlk[numOfUsedBlock], bucketBlk;
    bucketBlk.writeInit(resAddr);
    for (int i = 0; i < numOfUsedBlock; ++i)
        readBlk[i].loadFromDisk(readAddr++);
    row_t t[numOfRows];

    int readRows;
    while(1) {
        readRows = read_N_Rows_From_M_Block(readBlk, t, numOfRows, numOfUsedBlock);
        // printRows(t, readRows, val);
        
    }
}
