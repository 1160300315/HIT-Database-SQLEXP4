#include "utils.cpp"

/**
 * �����㷨
 * ������R.A ���� S.C
*/

const addr_t joinResultStart = 3000;    // ���ӽ������ʼ��ŵ�ַ

// -----------------------------------------------------------
//                       Nest Loop Join                       
// -----------------------------------------------------------

/**
 * ����Ƕ��ѭ���ķ������б������
 * 
 * R_addr: ��һ�����ڴ����е���ʼ��ַ
 * S_addr: �ڶ������ڴ����е���ʼ��ַ
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t NEST_LOOP_JOIN(addr_t R_addr, addr_t S_addr) {
    int numOfR = 6;     // R��ʹ�õĻ���������
    int numOfRows_R = numOfRowInBlk * numOfR, numOfRows_S = numOfRowInBlk;
    addr_t curAddr = 0, resAddr = joinResultStart;
    
    // R��S��ĳ�ʼ��
    block_t Rblk[numOfR], Sblk, resBlk;
    for (int i = 0; i < numOfR; ++i)
        Rblk[i].loadFromDisk(R_addr++);
    resBlk.writeInit(resAddr, numOfRowInBlk - 1);
    row_t t_R[numOfRows_R], t_S[numOfRows_S];

    int readRows_R, readRows_S;
    while(1) {
        // ÿ�δ�R�ж���6��
        readRows_R = read_N_Rows_From_M_Block(Rblk, t_R, numOfRows_R, numOfR);
        // printRows(t, readRows_R, val);
        Sblk.loadFromDisk(S_addr);
        while(1) {
            // ÿ�δ�S�ж���1����бȽ�
            readRows_S = read_N_Rows_From_1_Block(Sblk, t_S, numOfRows_S);
            for (int i = 0; i < numOfRows_R; ++i) {
                for (int j = 0; j < numOfRows_S; ++j) {
                    if (t_R[i].join_A(t_S[j])) {
                        curAddr = resBlk.writeRow(t_R[i]);
                        curAddr = resBlk.writeRow(t_S[j]);
                    }
                }
            }
            if (readRows_S < numOfRows_S) {
                // S������
                break;
            }
        }
        if (readRows_R < numOfRows_R) {
            // R������
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            break;
        }
    }
    return curAddr;
}

// -----------------------------------------------------------
//                      Sort Merge Join                       
// -----------------------------------------------------------
/**
 * ����ɨ��
 * �Ӵ����ж�ȡ��ȫ�����������������
 * ����������������������ӵļ�¼д�����洢����
 * ������ѡ����R���1�飬S���2��Ļ�������д����
 * 
 * R_addr: ����R��Ĵ洢��ʼ��ַ
 * S_addr: ����S��Ĵ洢��ʼ��ַ
 * resAddr: ���ӽ���Ĵ洢��ʼ��ַ
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t scan_3_SortedJoin(addr_t R_addr, addr_t S_addr, addr_t resAddr) {
    int maxNumOfRows_S = 2 * numOfRowInBlk; // ����һ�δӴ����϶�ȡ2��S��Ĳ���
    block_t Rblk, Sblk, resBlk;
    Row t_R, t_S[maxNumOfRows_S];
    resBlk.writeInit(resAddr, numOfRowInBlk - 1);
    Rblk.loadFromDisk(R_addr);
    Sblk.loadFromDisk(S_addr);

    int numOfRows_S = maxNumOfRows_S;
    int pt_S = numOfRows_S;     // ��¼S������һ�������Ӽ�¼�ļ���λ��
    int matchMark = 0;          // ��¼��һ�����Ӳ����ĵ�һ��ƥ��λ�ã��Ա��֦
    addr_t curAddr = resAddr;
    bool prior_match = false;
    bool R_over = false, S_over = false;
    while(1) {
        // ѭ��������������
        t_R = Rblk.getNewRow();
        R_over = (t_R.isFilled == false);
        if (R_over || S_over) {
            // ��������ѭ��������������
            // 1. R�������  ��  2. S�������
            if (!R_over)
                Rblk.freeBlock();
            else if (!S_over)
                Sblk.freeBlock();
            else
                ;
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            break;
        }
        if (pt_S == numOfRows_S) {
            // ���������м��ص��������ĵ�S�����ݣ���Ҫ������һ������
            pt_S = 0;       // ������ǵĸ�λ
            matchMark = 0;
            numOfRows_S = read_N_Rows_From_1_Block(Sblk, t_S, maxNumOfRows_S);
            S_over = (numOfRows_S < maxNumOfRows_S);
        }
        for (int i = matchMark; i < numOfRows_S; ++i) { 
            // R���¼��S���ȡ��ÿһ����¼���бȶ�&����
            if (t_R > t_S[i]) {
                prior_match = false;
                if (i == numOfRows_S - 1) {
                    // ĿǰΪֹû���ܹ������ϵģ���Ҫ���±�ǣ��Լ�����һ������
                    pt_S = numOfRows_S;
                }
                continue;
            } else if (t_R.join_A(t_S[i])) {
                matchMark = prior_match ? matchMark : i;
                prior_match = true;
                curAddr = resBlk.writeRow(t_R);
                curAddr = resBlk.writeRow(t_S[i]);
                pt_S = i + 1;   // �������һ�����ӵ�λ�ñ��
            } else {
                prior_match = false;
                break;
            }
        }
    }
    return curAddr;
}

/**
 * ���ö�·�鲢����ķ������б������
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t SORT_MERGE_JOIN() {
    int sizeOfSubTable = numOfRowInBlk * numOfBufBlock;           // �ɻ��������ֳ����ӱ��С
    int numOfSubTables_R = ceil(1.0 * R_size / sizeOfSubTable);   // R���ֳ����ӱ�����
    int numOfSubTables_S = ceil(1.0 * S_size / sizeOfSubTable);   // S���ֳ����ӱ�����
    /******************* һ��ɨ�� *******************/
    printf("============ һ��ɨ�迪ʼ ============\n");
    printf("---- �Ա�R���з������� ----\n");
    addr_t scan_1_next = addrOfScan_1;     // ��һ��ɨ�����洢����ʼ��ַ
    addr_t scan_1_Index_R, scan_1_Index_S; // R��S��ĵ�һ��ɨ�����洢����ʼλ������
    if ((scan_1_Index_R = scan_1_next) == ADDR_NOT_EXISTS)
        return FAIL;
    scan_1_next = scan_1_PartialSort(numOfSubTables_R, scan_1_next, R_start, sizeOfSubTable);
    printf("---- �Ա�S���з������� ----\n");
    if ((scan_1_Index_S = scan_1_next) == ADDR_NOT_EXISTS)
        return FAIL;
    scan_1_next = scan_1_PartialSort(numOfSubTables_S, scan_1_next, S_start, sizeOfSubTable);

    /******************* ����ɨ�� *******************/
    printf("============ ����ɨ�迪ʼ ============\n");
    printf("---- �Ա�R���й鲢���� ----\n");
    addr_t scan_2_next = addrOfScan_2;     // �ڶ���ɨ�����洢����ʼ��ַ
    addr_t scan_2_Index_R, scan_2_Index_S; // R��S��ĵڶ���ɨ�����洢����ʼλ������
    if ((scan_2_Index_R = scan_2_next) == ADDR_NOT_EXISTS)
        return FAIL;
    scan_2_next = scan_2_SortMerge(numOfSubTables_R, scan_1_Index_R, scan_2_Index_R);
    printf("---- �Ա�S���й鲢���� ----\n");
    if ((scan_2_Index_S = scan_2_next) == ADDR_NOT_EXISTS)
        return FAIL;
    scan_2_next = scan_2_SortMerge(numOfSubTables_S, scan_1_Index_S, scan_2_Index_S);

    /******************* ����ɨ�� *******************/
    printf("============ ����ɨ�迪ʼ ============\n");
    addr_t scan_3_result = joinResultStart;
    scan_3_result = scan_3_SortedJoin(scan_2_Index_R, scan_2_Index_S, scan_3_result);
    return scan_3_result;
}

// -----------------------------------------------------------
//                          Hash Join                         
// -----------------------------------------------------------

/**
 * ����ɨ����м䲽�衪����Ͱ����
 * �����ӵļ�¼һ��������ͬ��ɢ��ֵ�����ֻ�����һ��Ͱ�е����м�¼����
 * 
 * numOfBuckets: ɢ�е�Ͱ������
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t scan_2_HashJoin(int numOfBuckets, addr_t scan_1_index_R[], addr_t scan_1_index_S[]) {
// addr_t scan_2_HashJoin(int numOfBuckets, addr_t readIndex, addr_t scan_1_index[], addr_t scan_2_index) {
    // addr_t curAddr = 0;
    // int numOfRows = numOfRowInBlk * (numOfBuckets / 2);
    // row_t R_data[numOfRows], S_data[numOfRows];
    // block_t Sblk, Rblk, resBlk;
    // resBlk.writeInit(scan_2_index, numOfRowInBlk - 1);

    // Rblk.loadFromDisk(readIndex);
    // int readRows_R, readRows_S;
    // while(1) {
    //     readRows_R = read_N_Rows_From_1_Block(Rblk, R_data, numOfRows);
    //     for (int i = 0; i < readRows_R; ++i) {
    //         int R_hash = hashRowsByA(R_data[i], numOfBuckets);
    //         Sblk.loadFromDisk(scan_1_index[R_hash]);
    //         while(1) {
    //             readRows_S = read_N_Rows_From_1_Block(Sblk, S_data, numOfRows);
    //             for (int j = 0; j < readRows_S; ++j) {
    //                 if (R_data[i].join_A(S_data[j])) {
    //                     curAddr = resBlk.writeRow(R_data[i]);
    //                     curAddr = resBlk.writeRow(S_data[j]);
    //                 }
    //             }
    //             if(readRows_S < numOfRows) {
    //                 Sblk.freeBlock();
    //                 break;
    //             }
    //         }
    //     }
    //     if (readRows_R < numOfRows) {
    //         Rblk.freeBlock();
    //         addr_t endAddr = resBlk.writeLastBlock();
    //         if (endAddr != END_OF_FILE)
    //             curAddr = endAddr;
    //         break;
    //     }
    // }
    // return curAddr;
    
    addr_t curAddr = 0;
    int numOfRows = numOfRowInBlk * (numOfBuckets / 2);
    row_t R_data[numOfRows], S_data[numOfRows];
    block_t Rblk, Sblk, resBlk;
    resBlk.writeInit(joinResultStart, numOfRowInBlk - 1);

    for (int k = 0; k < numOfBuckets; ++k) {
        Rblk.loadFromDisk(scan_1_index_R[k]);
        Sblk.loadFromDisk(scan_1_index_S[k]);
        int readRows_R, readRows_S;
        bool R_over, S_over;
        while(1) {
            readRows_R = read_N_Rows_From_1_Block(Rblk, R_data, numOfRows);
            insertSort<row_t>(R_data, readRows_R);
            R_over = (readRows_R < numOfRows); // R������¼����С��numOfRows����R���Ѷ���
            readRows_S = read_N_Rows_From_1_Block(Sblk, S_data, numOfRows);
            insertSort<row_t>(S_data, readRows_S);
            S_over = (readRows_S < numOfRows); // S������¼����С��numOfRows����S���Ѷ���
            for (int i = 0; i < readRows_R; ++i) {
                for (int j = 0; j < readRows_S; ++j) {
                    if (R_data[i].join_A(S_data[j])) {
                        curAddr = resBlk.writeRow(R_data[i]);
                        curAddr = resBlk.writeRow(S_data[j]);
                    }
                }
                if (S_over)
                    break;
            }
            if (R_over)
                break;
        }
        Rblk.freeBlock();
        Sblk.freeBlock();
        if (k == numOfBuckets - 1) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
        }
    }
    return curAddr;
}


/**
 * ����ɢ�еķ������б������
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t HASH_JOIN() {
    /******************* һ��ɨ�� *******************/
    printf("============ һ��ɨ�迪ʼ ============\n");
    int numOfBuckets = numOfBufBlock - 1;   // ɢ��Ͱ������һ��ȡ����������-1
    addr_t scan_1_Index_R[7] = {5300, 5400, 5500, 5600, 5700, 5800, 5900};
    addr_t scan_1_Index_S[7] = {6300, 6400, 6500, 6600, 6700, 6800, 6900};
    printf("---- �Ա�R���з���ɢ�� ----\n");
    scan_1_HashToBucket(numOfBuckets, R_start, scan_1_Index_R);
    printf("---- �Ա�S���з���ɢ�� ----\n");
    scan_1_HashToBucket(numOfBuckets, S_start, scan_1_Index_S);

    /******************* ����ɨ�� *******************/
    printf("============ ����ɨ�迪ʼ ============\n");
    // addr_t curAddr = scan_2_HashJoin(numOfBuckets, R_start, scan_1_Index_S, joinResultStart);
    addr_t curAddr = scan_2_HashJoin(numOfBuckets, scan_1_Index_R, scan_1_Index_S);

    return curAddr;
}


/**************************** main ****************************/
int main() {
    bufferInit();
    addr_t resEndAddr;
    // if ((resEndAddr = NEST_LOOP_JOIN(R_start, S_start)) == END_OF_FILE) {
    if ((resEndAddr = SORT_MERGE_JOIN()) == END_OF_FILE) {
    // if ((resEndAddr = HASH_JOIN()) == END_OF_FILE) {
        system("pause");
        return FAIL;
    }
    unsigned long numIO = buff.numIO;
    showResult(joinResultStart, 2 * sizeOfRow);
    printf("\nע�����д����̿���ţ�3000-%d\n", resEndAddr);
    printf("\n���ι�����%ld��I/O\n", numIO);
    system("pause");
    return OK;
}