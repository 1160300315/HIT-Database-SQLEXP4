#include "utils.cpp"
#include "distinct.cpp"


const addr_t setOperationResultStart = 4000;    // ���ϲ����������ʼ��ŵ�ַ


/**
 * @brief ��Ĳ�����
 * 
 * @param R_addr ��һ�����ڴ����е���ʼ��ַ
 * @param S_addr �ڶ������ڴ����е���ʼ��ַ
 */
void tablesUnion(addr_t R_addr, addr_t S_addr) {

}


/**
 * @brief ��Ľ�����
 * ����Ƕ��ѭ���ķ����������������ļ�¼
 * 
 * @param table1 ��һ����������Ϣ
 * @param table2 �ڶ�����������Ϣ
 * @return table_t �������������������Ϣ��
 */
table_t tablesIntersect(table_t table1, table_t table2) {
    table_t resTable(setOperationResultStart), bigTable, smallTable;
    if (table1.size > table2.size) {
        bigTable = table1;
        smallTable = table2;
    } else {
        bigTable = table2;
        smallTable = table1;
    }
    int numOfR = 6;
    int numOfRows_R = numOfRowInBlk * numOfR, numOfRows_S = numOfRowInBlk;
    addr_t readAddr_1 = bigTable.start, readAddr_2 = smallTable.start;
    addr_t curAddr = 0, resAddr = resTable.start;
    
    block_t seriesBlk[numOfR], singleBlk, resBlk;
    for (int i = 0; i < numOfR; ++i)
        seriesBlk[i].loadFromDisk(readAddr_1++);
    resBlk.writeInit(resAddr);
    row_t t_R[numOfRows_R], t_S[numOfRows_S];

    int readRows_1, readRows_2;
    while(1) {
        // ÿ�δ�R�ж���6��
        readRows_1 = read_N_Rows_From_M_Block(seriesBlk, t_R, numOfRows_R, numOfR);
        // printRows(t, readRows, val);
        singleBlk.loadFromDisk(readAddr_2);
        while(1) {
            // ÿ�δ�S�ж���1����бȽ�
            readRows_2 = read_N_Rows_From_1_Block(singleBlk, t_S, numOfRows_S);
            for (int i = 0; i < readRows_1; ++i) {
                for (int j = 0; j < readRows_2; ++j) {
                    if (t_R[i] == t_S[j]) {
                        curAddr = resBlk.writeRow(t_R[i]);
                        resTable.size += 1;
                    }
                }
            }
            if (readRows_2 < numOfRows_S)
                break;
        }
        if (readRows_1 < numOfRows_R) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
    }
    return resTable;
}


/**
 * @brief ��Ĳ����
 * ����Ƕ��ѭ���ķ����������������ļ�¼
 * 
 * @param diffedTable �����������Ϣ
 * @param diffTable ���������Ϣ
 * @return table_t ������������������Ϣ��
 */
table_t tablesDiff(table_t diffedTable, table_t diffTable) {
    table_t resTable(setOperationResultStart);
    int numOfSeriesBlock = 6;
    int numOfRows_1 = numOfRowInBlk * numOfSeriesBlock, numOfRows_2 = numOfRowInBlk;
    addr_t diffedTableAddr = diffedTable.start, diffTableAddr = diffTable.start;
    addr_t curAddr = 0;
    
    block_t seriesBlk[numOfSeriesBlock], singleBlk, resBlk;
    resBlk.writeInit(resTable.start);
    for (int i = 0; i < numOfSeriesBlock; ++i)
        seriesBlk[i].loadFromDisk(diffedTableAddr++);
    row_t tDiffed[numOfRows_1], tDiff[numOfRows_2];

    int readRows_1, readRows_2;
    while(1) {
        // ÿ�δӴ���ж���6��
        readRows_1 = read_N_Rows_From_M_Block(seriesBlk, tDiffed, numOfRows_1, numOfSeriesBlock);
        // insertSort<row_t>(tDiffed, readRows_1);
        // printRows(tDiffed, readRows_1);
        for (int i = 0; i < readRows_1; ++i) {
            bool isAppeared = false;
            singleBlk.loadFromDisk(diffTableAddr);
            while(1) {
                // ÿ�δ�С���ж���1����бȽ�
                readRows_2 = read_N_Rows_From_1_Block(singleBlk, tDiff, numOfRows_2);
                for (int j = 0; j < readRows_2; ++j) {
                    if (tDiff[j] == tDiffed[i]) {
                        isAppeared = true;
                        break;
                    }
                }
                if (readRows_2 < numOfRows_2 || isAppeared) {
                    // ���������˻����ڱ������������һ���ļ�¼����ֹͣ����
                    if (isAppeared)
                        singleBlk.freeBlock();
                    break;
                }
            }
            // ����ڲ����û�з��ָñ�����еļ�¼����˵���ü�¼�ǲ�����Ľ��
            if (!isAppeared) {
                curAddr = resBlk.writeRow(tDiffed[i]);
                resTable.size += 1;
            }
        }
        if (readRows_1 < numOfRows_1) {
            // ��������
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
    }
    return resTable;
}


/**************************** main ****************************/
int main() {
    bufferInit();
    // useCluster(table_R, 8000);
    // table_t a = {8000, 112};
    clear_Buff_IO_Count();
    // table_t res = tablesUnion(table_R, table_S);
    // table_t res = tablesIntersect(table_R, table_S);
    // table_t res = tablesDiff(table_R, table_S);
    table_t res = tablesDiff(table_S, table_R);
    // useCluster(res, 4000);
    // tableDistinct(res, res.start);
    showResult(res.start);
    int numOfUsedBlocks = ceil(1.0 * res.size / (numOfRowInBlk));
    printf("\nע�����д����ʼ���̿飺4000-%d\n", res.start + numOfUsedBlocks - 1);
    printf("���ι�����%ld��I/O\n\n", buff.numIO);

    system("pause");
    return OK;
}