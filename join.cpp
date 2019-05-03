#include "utils.cpp"
#include "index.cpp"

/**
 * �����㷨
 * ������R.A ���� S.C
*/

const addr_t joinResultStart = 3000;    // ���ӽ������ʼ��ŵ�ַ

// -----------------------------------------------------------
//                       Nest Loop Join                       
// -----------------------------------------------------------

/**
 * @brief ����Ƕ��ѭ���ķ������б������
 * 
 * @param table1 ��һ����������Ϣ
 * @param table2 �ڶ�����������Ϣ
 * @return table_t ���ӽ������������Ϣ��
 */
table_t NEST_LOOP_JOIN(table_t table1, table_t table2) {
    table_t resTable(joinResultStart), bigTable, smallTable;
    resTable.rowSize = 2 * sizeOfRow;
    // ������С��
    if (table1.size > table2.size) {
        bigTable = table1;
        smallTable = table2;
    } else {
        bigTable = table2;
        smallTable = table1;
    }
    addr_t bigTableAddr = bigTable.start, smallTableAddr = smallTable.start;
    int numOfSeriesBlock = 6;     // ���ʹ�õĻ���������
    int numOfRows_1 = numOfRowInBlk * numOfSeriesBlock; // ���һ�ζ���ļ�¼����
    int numOfRows_2 = numOfRowInBlk;                    // С��һ�ζ���ļ�¼����
    int numOfRows_res = numOfRowInBlk - 1;              // ������еı�׼��¼����
    addr_t curAddr = 0;
    
    // ��ʼ��������
    block_t seriesBlk[numOfSeriesBlock], singleBlk, resBlk;
    resBlk.writeInit(joinResultStart, numOfRows_res);
    for (int i = 0; i < numOfSeriesBlock; ++i) {
        seriesBlk[i].loadFromDisk(bigTableAddr);
        bigTableAddr = seriesBlk[i].readNextAddr();
        if (bigTableAddr == END_OF_FILE) {
            numOfSeriesBlock = i + 1;
            numOfRows_1 = numOfRowInBlk * numOfSeriesBlock;
            break;
        }
    }

    row_t tSeries[numOfRows_1], tSingle[numOfRows_2];
    int readRows_1, readRows_2;
    while(1) {
        // С��ѭ��Ƕ����ѭ�������Լ���IO����
        // ���ѭ�������һ�δӴ����϶�ȡnumOfSeriesBlock�������
        readRows_1 = read_N_Rows_From_M_Block(seriesBlk, tSeries, numOfRows_1, numOfSeriesBlock);
        singleBlk.loadFromDisk(smallTableAddr);
        while(1) {
            // �ڲ�ѭ����С��һ�δӴ����϶�ȡ1�������
            readRows_2 = read_N_Rows_From_1_Block(singleBlk, tSingle, numOfRows_2);
            for (int i = 0; i < readRows_1; ++i) {
                for (int j = 0; j < readRows_2; ++j) {
                    if (tSeries[i].join_A(tSingle[j])) {
                        curAddr = resBlk.writeRow(tSeries[i]);
                        curAddr = resBlk.writeRow(tSingle[j]);
                        resTable.size += 1;
                    }
                }
            }
            if (readRows_2 < numOfRows_2) {
                // S������
                break;
            }
        }
        if (readRows_1 < numOfRows_1) {
            // R������
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
    }
    return resTable;
}

// -----------------------------------------------------------
//                      Sort Merge Join                       
// -----------------------------------------------------------
/**
 * @brief ����ɨ��
 * �Ӵ����ж�ȡ��ȫ�����������������
 * ����������������������ӵļ�¼д�����洢����
 * ������ѡ����R���1�飬S���2��Ļ�������д����
 * 
 * @param R_addr ����R��Ĵ洢��ʼ��ַ
 * @param S_addr ����S��Ĵ洢��ʼ��ַ
 * @param resTable ���ӽ���Ĵ洢��Ϣ��
 */
void scan_3_SortedJoin(table_t table1, table_t table2, table_t &resTable) {
    table_t bigTable, smallTable;
    // ������С��
    if (table1.size > table2.size) {
        bigTable = table1;
        smallTable = table2;
    } else {
        bigTable = table2;
        smallTable = table1;
    }
    int maxNumOfRows_1 = numOfRowInBlk;     // һ�δӴ����϶�ȡ1��С��
    int maxNumOfRows_2 = 2 * numOfRowInBlk; // һ�δӴ����϶�ȡ2����
    block_t blkSmall, blkBig, preLoadBlk, resBlk;
    Row t_R, t_S[maxNumOfRows_2];
    resBlk.writeInit(resTable.start, numOfRowInBlk - 1);
    blkSmall.loadFromDisk(smallTable.start);
    blkBig.loadFromDisk(bigTable.start);

    int readRows_2 = maxNumOfRows_2;
    int pt_S = readRows_2;     // ��¼S������һ�������Ӽ�¼�ļ���λ��
    int matchMark = 0;          // ��¼��һ�����Ӳ����ĵ�һ��ƥ��λ�ã��Ա��֦
    addr_t curAddr = resTable.start;
    bool prior_match = false;
    bool R_over = false, S_over = false;
    while(1) {
        // ѭ��������������
        t_R = blkSmall.getNewRow();
        R_over = (t_R.isFilled == false);
        if (R_over || S_over) {
            // ��������ѭ��������������
            // 1. R�������  ��  2. S�������
            if (!R_over)
                blkSmall.freeBlock();
            else if (!S_over)
                blkBig.freeBlock();
            else
                ;
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
        if (pt_S == readRows_2) {
            // ���������м��ص��������ĵ�S�����ݣ���Ҫ������һ������
            pt_S = 0;       // ������ǵĸ�λ
            matchMark = 0;
            readRows_2 = read_N_Rows_From_1_Block(blkBig, t_S, maxNumOfRows_2);
            S_over = (readRows_2 < maxNumOfRows_2);
        }
        for (int i = matchMark; i < readRows_2; ++i) { 
            // R���¼��S���ȡ��ÿһ����¼���бȶ�&����
            if (t_R > t_S[i]) {
                prior_match = false;
                if (i == readRows_2 - 1) {
                    // ĿǰΪֹû���ܹ������ϵģ���Ҫ���±�ǣ��Լ�����һ������
                    pt_S = readRows_2;
                }
                continue;
            } else if (t_R.join_A(t_S[i])) {
                matchMark = prior_match ? matchMark : i;
                prior_match = true;
                curAddr = resBlk.writeRow(t_R);
                curAddr = resBlk.writeRow(t_S[i]);
                resTable.size += 1;
                pt_S = i + 1;   // �������һ�����ӵ�λ�ñ��
            } else {
                prior_match = false;
                break;
            }
        }
    }
}


/**
 * @brief ���ö�·�鲢����ķ������б������
 * 
 * @return table_t ���ӽ�������������һ��ĵ�ַ
 */
table_t SORT_MERGE_JOIN(table_t table1, table_t table2) {
    table_t bigTable, smallTable, resTable(joinResultStart);
    resTable.rowSize = 2 * sizeOfRow;
    // ������С��
    if (table1.size > table2.size) {
        bigTable = table1;
        smallTable = table2;
    } else {
        bigTable = table2;
        smallTable = table1;
    }
    // �ȶ�������һ��۴�
    useCluster(bigTable);
    addr_t smallTableAddr = useCluster(smallTable);
    // �Դ��������
    addr_t indexAddr = useIndex(bigTable);
    loadIndex(indexAddr);
    BPTR.printData();

    block_t blk1, blk2, resBlk;
    resBlk.writeInit(resTable.start, numOfRowInBlk - 1);
    blk1.loadFromDisk(smallTableAddr);

    addr_t curAddr = 0, loadAddr;
    row_t t1[numOfRowInBlk], t2[numOfRowInBlk], t2_copy[numOfRowInBlk];
    int readRows_1, readRows_2, readRows_2_copy;
    int prior_1 = MAX_ATTR_VAL;
    bool earlyDie = false;
    cursor_t cursor_2;
    while(1) {
        readRows_1 = read_N_Rows_From_1_Block(blk1, t1, numOfRowInBlk);
        printRows(t1, readRows_1);
        for (int k = 0; k < readRows_1; ++k) {
            bool isSametoPrior = (t1[k].A == prior_1);
            if (!isSametoPrior) {
                // ����һ����¼��Aֵ��ͬʱ�ż��أ������ظ����ش�����IO����
                vector<addr_t> addrList = BPTR.select(t1[k].A, EQ);
                if (addrList.empty()) {
                    // û��ƥ���ֵ��ֱ������������¼�ĺ���ƥ�乤��
                    continue;
                }
                loadAddr = addrList[0];
                blk2.loadFromDisk(loadAddr);
                earlyDie = false;   // �����˲�ͬ��������Ȼû����������
            }
            prior_1 = t1[k].A;
            if (earlyDie) {
                // ��ƥ������г��������������
                // ��Ҫ���´Ӹü�¼�ĵ�һ������ָ��Ĵ��̿鿪ʼ����
                blk2.loadFromDisk(loadAddr);
            }
            int loadBlocks = 0;
            while(1) {
                // ���������ؿ��м�������ֵ
                readRows_2 = read_N_Rows_From_1_Block(blk2, t2, numOfRowInBlk);
                loadBlocks += 1;
                bool joinFinish = (readRows_2 < numOfRowInBlk);
                cursor_2 = 0;
                if (loadBlocks > 1 && t2[0].join_A(t1[k])) {
                    // �������󲶻񣺵����ش�������1��ʱ�����¼��ؿ�ĵ�һ����¼������ǰ�ļ�¼(t1[k])��������
                    // ˵����һ�鲻��ȫ�������п������ӵļ�¼��������
                    // ��Ҳ˵���������ļ��ʱ���ڼ���֮����������ע�⣡
                    earlyDie = true;
                }
                if (earlyDie || (isSametoPrior == false && loadBlocks == 1)) {
                    readRows_2_copy = readRows_2;
                    for (int i = 0; i < readRows_2_copy; ++i)
                        t2_copy[i] = t2[i];
                    // �����ų���һ�ֲ���Ҫ������һ�飨���ı���һ�εĶ�ȡ����t2_copy�������������
                    // С��(������)��һ�����ж�����ͬAֵ�ļ�¼��ͬʱ���(��������������)��û����������
                    // printRows(t2_copy, readRows_2_copy);
                }
                row_t *cmpRow = (loadBlocks == 1) ? t2_copy : t2;
                cursor_t cmpCursor = (loadBlocks == 1) ? readRows_2_copy : readRows_2;
                // �ƶ��������е�ָ�뵽��һ��ƥ��������ֵ��λ��
                while(cmpRow[cursor_2].A != t1[k].A && cursor_2 < cmpCursor)
                    cursor_2 += 1;
                if (cursor_2 == cmpCursor) {
                    if (joinFinish == false)
                        blk2.freeBlock();
                    // �߽���������һ����û�з������������ļ�¼
                    cursor_2 = readRows_2_copy; // �������Ӳ���
                    joinFinish = true;
                }
                for (; cursor_2 < readRows_2_copy; ++cursor_2) {
                    if (!t2_copy[cursor_2].join_A(t1[k])) {
                        if (joinFinish == false)
                            blk2.freeBlock();
                        joinFinish = true;
                        break;
                    }
                    curAddr = resBlk.writeRow(t1[k]);
                    curAddr = resBlk.writeRow(t2_copy[cursor_2]);
                    resTable.size += 1;
                }
                if (joinFinish)
                    break;
            }
        }
        if (readRows_1 < numOfRowInBlk) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
    }
    return resTable;
}

// -----------------------------------------------------------
//                          Hash Join                         
// -----------------------------------------------------------

/**
 * @brief ����ɨ����м䲽�衪����Ͱ����
 * �����ӵļ�¼һ��������ͬ��ɢ��ֵ�����ֻ����������Ӧ�����ͬ��Ͱ�е����м�¼����
 * 
 * @param numOfBuckets ɢ�е�Ͱ������
 * @param scan_1_index_R 
 * @param scan_1_index_S 
 */
void scan_2_HashJoin(int numOfBuckets, addr_t scan_1_index_R[], addr_t scan_1_index_S[], table_t &resTable) {
    addr_t curAddr = 0;
    int numOfRows = numOfRowInBlk * (numOfBuckets / 2);
    row_t R_data[numOfRows], S_data[numOfRows];
    block_t blk1, blk2, resBlk;
    resBlk.writeInit(resTable.start, numOfRowInBlk - 1);

    for (int k = 0; k < numOfBuckets; ++k) {
        blk1.loadFromDisk(scan_1_index_R[k]);
        int readRows_R, readRows_S;
        while(1) {
            readRows_R = read_N_Rows_From_1_Block(blk1, R_data, numOfRows);
            insertSort<row_t>(R_data, readRows_R);
            blk2.loadFromDisk(scan_1_index_S[k]);
            // printRows(R_data, readRows_R);
            while(1) {
                readRows_S = read_N_Rows_From_1_Block(blk2, S_data, numOfRows);
                insertSort<row_t>(S_data, readRows_S);
                // printRows(S_data, readRows_S);
                for (int i = 0; i < readRows_R; ++i) {
                    for (int j = 0; j < readRows_S; ++j) {
                        if (R_data[i].join_A(S_data[j])) {
                            curAddr = resBlk.writeRow(R_data[i]);
                            curAddr = resBlk.writeRow(S_data[j]);
                            resTable.size += 1;
                        }
                        if (R_data[i] < S_data[j])
                            break;
                    }
                }
                if (readRows_S < numOfRows) {
                    break;
                }
            }
            blk2.freeBlock();
            if (readRows_R < numOfRows)
                break;
        }
        blk1.freeBlock();
        if (k == numOfBuckets - 1) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
        }
    }
    resTable.end = curAddr;
}


/**
 * @brief ����ɢ�еķ������б������
 * 
 * @return addr_t ���ӽ�������������һ��ĵ�ַ
 */
table_t HASH_JOIN() {
    /******************* һ��ɨ�� *******************/
    int numOfBuckets = 6;   // ɢ��Ͱ����������������֤��ȡ��ǰֵʱIO������С
    addr_t scan_1_Index_R[7] = {5300, 5400, 5500, 5600, 5700, 5800, 5900};
    addr_t scan_1_Index_S[7] = {6300, 6400, 6500, 6600, 6700, 6800, 6900};
    // �Ա�R���з���ɢ��
    scan_1_HashToBucket(numOfBuckets, R_start, scan_1_Index_R);
    // �Ա�S���з���ɢ��
    scan_1_HashToBucket(numOfBuckets, S_start, scan_1_Index_S);

    /******************* ����ɨ�� *******************/
    table_t resTable(joinResultStart, 2 * sizeOfRow);
    scan_2_HashJoin(numOfBuckets, scan_1_Index_R, scan_1_Index_S, resTable);
    return resTable;
}


/**************************** main ****************************/
int main() {
    bufferInit();
    // table_t res = NEST_LOOP_JOIN(table_R, table_S);
    table_t res = SORT_MERGE_JOIN(table_R, table_S);
    // table_t res = HASH_JOIN();
    if (res.size == 0) {
        system("pause");
        return FAIL;
    }
    showResult(joinResultStart, 2 * sizeOfRow);
    int numOfUsedBlocks = ceil(1.0 * res.size / (numOfRowInBlk / 2));
    printf("\nע�����д����̿���ţ�3000-%d\n", res.start + numOfUsedBlocks - 1);
    printf("\n���ι�����%ld��I/O\n", buff.numIO);
    system("pause");
    return OK;
}