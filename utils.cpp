#include <cmath>
#include <numeric>
#include "Block/Block.h"
#include "Block/Block.cpp"
#pragma once

const addr_t  R_start = 1;			// ������R��ַ��1��ʼ
const addr_t  S_start = 20;		    // ������S��ַ��20��ʼ
const addr_t  R_end = 16;           // ������R�����һ���ַΪ16
const addr_t  S_end = 51;           // ������S�����һ���ַΪ51
const int     R_size = 112;         // R��ϵ�ļ�¼����
const int     S_size = 224;         // S��ϵ�ļ�¼����
const table_t table_R(R_start, R_size, R_end);
const table_t table_S(S_start, S_size, S_end);


/**
 * @brief ������
 * ���������Ϣ���˳�
 * 
 * @param err ������Ϣ�ַ���
 */
void error(char *err) {
    printf("%s\n", err);
    system("pause");
    exit(FAIL);
}


/**
 * @brief ����¼��A�ֶ�ֵ����ɢ��
 * 
 * @param R ��ɢ�еļ�¼
 * @param numOfBuckets ɢ��Ͱ������
 * @return int �ü�¼��ɢ�н����������Ͱ�ı�ţ�
 */
int hashRowsByA(row_t R, int numOfBuckets) { return R.A % numOfBuckets; }


/**
 * @brief argmin��ʵ��
 * 
 * @tparam T t���������
 * @param t ��argmin������
 * @param size ����Ĵ�С
 * @return int ����t����Сֵ���±�
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
 * @brief ��������
 * 
 * @tparam T array���������
 * @param array �����������
 * @param size  ����������Ĵ�С
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
 * @brief ����һ���¼�����ÿһ����¼������
 * 
 * @param t ��Ҫ����ļ�¼��
 * @param size ��¼������
 * @param matchVal ƥ��ֵ��ͨ�����ֶ����������ʾƥ��ļ�¼
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
 * @brief ���������̿��д�ŵ�����
 * 
 * @param resStartAddr ������̿����ʼ��ַ
 * @param newSizeOfRow ��¼�ĳ��ȣ����ڿ��ƻ��з���λ��
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
        if (readRows < numOfStandardRows) {
            if (readRows % numOfRowInBlk != 0)
                readBlk.freeBlock();
            break;
        }
    }

    printf("----------------------------------\n");
    printf(" ��%d�н��\n\n", curRows / base);
}


/**
 * @brief һ��ɨ����м䲽�衪������ɢ��
 * �Ӵ����ж�ȡ���ݵ��������ϣ�ɢ�г������鲢��ÿ��ɢ�н���洢�ش�����
 * 
 * @param numOfBuckets �������ϵ����ӱ�����
 * @param startIndex �ñ�һ��ɨ�����洢����ʼ��ַ
 * @param scan_1_index �ñ�һ��ɨ����������̿����ʼ��ַ
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
 * @brief һ��ɨ����м䲽�衪����������
 * �Ӵ����ж�ȡ���ݵ��������ϣ����򲢽��������洢�ش�����
 * ͬʱ����¼����������еõ���ÿ���ӱ���׵�ַ
 * 
 * @param numOfSubTables  �������ϵ����ӱ�����
 * @param scan_1_index �ñ�һ��ɨ�����洢����ʼ��ַ
 * @param startIndex �ñ�һ��ɨ����������̿����ʼ��ַ
 * @param sizeOfSubTable ɨ���ӱ�Ĵ�С
 * @return addr_t һ��ɨ�������ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t scan_1_PartialSort(int numOfSubTables, addr_t scan_1_index, addr_t startIndex, int sizeOfSubTable) {
    int numOfUsedBlk = numOfBufBlock, numOfRows = sizeOfSubTable;
    addr_t curAddr = scan_1_index, nextStart = startIndex;
    block_t blk[numOfUsedBlk];
    row_t R_data[sizeOfSubTable], emptyRow;
    int readRows;
    for (int k = 0; k < numOfSubTables; ++k) {
        // printf("** ��ʼ��%d���ӱ������ **\n", k + 1);
        // һ�δӴ����ж�8���������Ϊһ���ӱ�
        for (int i = 0; i < numOfUsedBlk; ++i) {
            blk[i].loadFromDisk(nextStart);
            nextStart = (int)blk[i].readNextAddr();	// ȡ��һ���ַ��next��
            if (nextStart == 0) {
                numOfUsedBlk = i + 1;
                numOfRows = numOfRowInBlk * numOfUsedBlk;
                break;
            }
        }
        nextStart = blk[numOfUsedBlk - 1].readNextAddr();
        // ��ȡ��8���е�����
        readRows = read_N_Rows_From_M_Block(blk, R_data, numOfRows, numOfUsedBlk);
        if (readRows == numOfRows && k < numOfSubTables - 1) {
            // ����read_N_Rows_From_M_Block���Զ�������һ���������
            // ����������Ҫ�ֶ��ͷű����صĻ�����
            int loaded = numOfBufBlock - buff.numFreeBlk;
            for (int i = 0; i < loaded; ++i) {
                blk[i].freeBlock();
            }
        }
        // �Ե�ǰ�ӱ��������
        insertSort<row_t>(R_data, readRows);
        // printRows(R_data, readRows);
        // ��������ӱ�д�ش���
        addr_t nextWriteAddr = scan_1_index + k * 8;
        for (int i = 0; i < numOfUsedBlk; ++i) {
            int numOfRows = sizeOfSubTable / sizeOfRow;
            blk[i].writeInit(nextWriteAddr++, numOfRows);
            for (int j = 0, count; j < numOfRows; ++j) {
                count = i * numOfRows + j;
                if (count < readRows)
                    blk[i].writeRow(R_data[count]);
                else
                    break;
            }
            if (i < numOfUsedBlk - 1) {
                // ��Ҫ�ô˲���ǿ�ƽ��������д�ش��̣����ͷŶ�Ӧ������
                blk[i].writeRow(emptyRow);
                blk[i].freeBlock();
            } else
                blk[i].writeLastBlock();
            curAddr += 1;
        }
    }
    return curAddr;
}


/**
 * @brief ����ɨ����м䲽�衪�����ڹ鲢����
 * �Ӵ����ж�ȡһ���������з�����������ݵ���������
 * �����ܱ����򲢽��������洢�ش�����
 * 
 * @param numOfSubTables �������ϵ����ӱ�����
 * @param scan_1_index �ñ�һ��ɨ�����洢����ʼ��ַ
 * @param scan_2_index �ñ����ɨ�����洢����ʼ��ַ
 * @return addr_t ����ɨ�������ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t scan_2_SortMerge(int numOfSubTables, addr_t scan_1_index, addr_t scan_2_index) {
    addr_t resultAddr = scan_2_index;
    block_t readBlk[numOfSubTables];
    block_t resBlk;
    Row rtemp, rFirst[numOfSubTables];  // ��¼ÿ��ĵ�һ��Ԫ��
    resBlk.writeInit(resultAddr);
    // ��װ�ص�ǰ��ϵ�������ӱ��еĵ�һ��
    for (int i = 0; i < numOfSubTables; ++i) {
        addr_t curAddr = scan_1_index + i * 8;
        readBlk[i].loadFromDisk(curAddr);
        rFirst[i] = readBlk[i].getNewRow();
        if (rFirst[i].isFilled == false)
            return ADDR_NOT_EXISTS;
    }

    int readRows[numOfSubTables];
    for (int i = 0; i < numOfSubTables; ++i)
        readRows[i] = 0;
    int arg = argmin(rFirst, numOfSubTables);
    while(1) {
        // �ӵ�ǰλ��ÿ������ǰ��ļ�¼�У�ȡ�����ֶ�ֵ��С��һ����¼���±�
        // ��argΪ�ü�¼���ڵ��ӱ���;
        rtemp = rFirst[arg];
        // printf("arg: %d\trtemp.A: %d\t", arg, rtemp.A);
        // printRows(rFirst, numOfSubTables);
        rFirst[arg] = readBlk[arg].getNewRow();
        arg = argmin(rFirst, numOfSubTables);
        if (rtemp.isFilled == false) {
            // �����ӱ��Ѷ���
            int totalReadRows = std::accumulate(readRows, readRows + numOfSubTables, 0);
            resultAddr += ceil(1.0 * totalReadRows / numOfRowInBlk) - 1;
            for (int i = 0; i < numOfSubTables; ++i) {
                if (readRows[i] % numOfRowInBlk != 0) {
                    // ��ȡ����������7�ı�����˵����һ���¼����<7
                    // ����getNewRowֻ���ڶ���һ����(7����¼)֮��Ż��ͷţ������Ҫ�ֶ��ͷŻ�����
                    readBlk[i].freeBlock();
                }
            }
            int isLastBlock = resBlk.writeLastBlock();
            if (isLastBlock) {
                // ������л���ʣ�������
                resultAddr += 1;
            }
            break;
        }
        readRows[arg] += 1;
        resBlk.writeRow(rtemp);
    }
    return resultAddr;
}
