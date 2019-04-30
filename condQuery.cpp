#include "Block.h"
#include "Block.cpp"
#include "utils.cpp"
#include "index.cpp"


/**
 * ���������㷨
 * ������R.A = 40  ��  S.C = 60
 */

const addr_t condQueryStart = 1000; //�������������ʵ��ŵ�ַ

/** 
 * �ȽϺ���
 * 
 * return: ��¼R���val��λ��
 */
int cmp(row_t R, int val) {
    if (R.A == val)
        return EQ;
    else if (R.A < val)
        return LT;
    else
        return GT;
}

/**
 * �����������
 * 
 * return: ��¼��A�ֶε�ֵ�Ƿ���val���
 */
bool EQ_cond(row_t R, int val) { return (R.A == val); }

/**
 * ������Լ���
 * �Դ������ı��ռ�������cond�ͼ���ֵval�������Լ���
 * 
 * table: �������ı���Ϣ
 * resTable: �����������Ϣ
 * val: ����ֵ
 * cond: ������������
 * 
 * return: ������������������һ��ĵ�ַ
 */
addr_t linearQuery(const table_t &table, table_t &resTable, int val, bool (*cond)(row_t, int)) {
    int numOfUsedBlock = numOfBufBlock - 1;
    int numOfRows = numOfRowInBlk * numOfUsedBlock;
    addr_t curAddr = 0, readAddr = table.start;

    block_t blk[numOfUsedBlock], resBlk;
    resBlk.writeInit(resTable.start);
    for (int i = 0; i < numOfUsedBlock; ++i)
        blk[i].loadFromDisk(readAddr++);
    row_t t[numOfRows];

    int readRows;
    while(1) {
        readRows = read_N_Rows_From_M_Block(blk, t, numOfRows, numOfUsedBlock);
        // printRows(t, readRows, val);
        bool isLastBlock = (readRows < numOfRows);
        for (int i = 0; i < readRows; ++i) {
            if (cond(t[i], val)) 
                curAddr = resBlk.writeRow(t[i]);
        }
        if (isLastBlock) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            break;
        }
    }
    return curAddr;
}

/**
 * ���ּ���
 * �Դ������ı���ƥ������cmp��ƥ��ֵval�������Լ���
 * 
 * table: �������ı���Ϣ
 * resTable: �����������Ϣ
 * val: ƥ��ֵ
 * cmp: ���������ȽϺ������ܹ���ӳ��Ŀ���¼�뵱ǰ������¼��λ����Ϣ
 * 
 * return: ������������������һ��ĵ�ַ
 */
addr_t binaryQuery(const table_t &table, table_t &resTable, int val, int (*cmp)(row_t, int)) {
    int numOfUsedBlock = numOfBufBlock - 1;     // ����7��ͬʱ����
    int numOfRows = numOfRowInBlk * numOfUsedBlock;
    addr_t curAddr = 0, readAddr = table.start;

    printf("----- ��ÿ%d����ж��ּ��� -----\n", numOfUsedBlock);
    row_t t[numOfRows], res[numOfRows];
    int pRes = 0;       // ��¼���ϼ��������ļ�¼����
    block_t blk[numOfBufBlock - 1], resBlk;
    resBlk.writeInit(resTable.start);
    for (int i = 0; i < numOfUsedBlock; ++i)
        blk[i].loadFromDisk(readAddr++);

    int readRows;
    while(1) {
        readRows = read_N_Rows_From_M_Block(blk, t, numOfRows, numOfUsedBlock);
        // printRows(t, readRows, val);
        insertSort<row_t>(t, readRows);
        // ��ʼ���ֲ���
        int left = 0, right = readRows - 1;
        while (left <= right) {
            int mid = (right + left) / 2;
            if (cmp(t[mid], val) == EQ) {
                int forward = mid, back = mid;
                res[pRes++] = t[mid];
                // ������һ�μ�¼������ģ���˲��ҵ���һ�����������ļ�¼���丽�����ܻ���ͬ������������������¼
                // �����ü�¼���������м�¼�����ҷ��������ļ�¼
                while(cmp(t[--back], val) == EQ)
                    res[pRes++] = t[back];
                while(cmp(t[++forward], val) == EQ)
                    res[pRes++] = t[forward];
                break;
            } else if (cmp(t[mid], val) == LT) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        if (readRows < numOfRows)
            break;
    }
    if (pRes == 0) {
        printf("û�м��������������ļ�¼!\n");
        return 0;
    }
    insertSort<row_t>(res, pRes);
    for (int i = 0; i < pRes; ++i)
        curAddr = resBlk.writeRow(res[i]);
    addr_t endAddr = resBlk.writeLastBlock();
    if (endAddr != END_OF_FILE)
        curAddr = endAddr;
    return curAddr;
}

/**
 * ����������
 * �Դ������ı�����val��Ӧ��������Ϣ�����ض�Ӧ�Ĵ��̿���ٽ������Լ���
 * ���ñ�δ���۴�/���������Ĳ��������ȶԸñ����۴�/��������
 * 
 * table: �������ı���Ϣ
 * resTable: �����������Ϣ
 * val: ����ֵ
 * 
 * return: ������������������һ��ĵ�ַ
 */
addr_t indexQuery(const table_t &table, table_t &resTable, int val) {
    // ��δ�۴�ʱ����Ҫ�Ⱦ۴�
    addr_t clusterAddr, indexAddr, curAddr = 0;
    table_map_t::iterator findCluster = clusterTableMap.find(table.start);
    if (findCluster == clusterTableMap.end()) {
        printf("��ǰ���ұ�δ��δ�۴أ��ֽ��о۴ز���...\n");
        if (clusterTableMap.size() == 0)
            clusterAddr = addrOfScan_2;
        else {
            table_map_t::iterator iter = clusterTableMap.end();
            --iter;
            clusterAddr = (iter->second).B + 1;
        }
        tableClustering(table, clusterAddr);
        printf("\n�۴���ɣ�\n");
        printf("�۴�����IO: %d\n\n", buff.numIO);
    } else {
        index_t addrItem = indexTableMap.at(table.start);
        clusterAddr = addrItem.A;
    }

    // ��δ��������ʱ����Ҫ�Ƚ�������
    table_map_t::iterator findIndex = indexTableMap.find(table.start);
    if (findIndex == indexTableMap.end()) {
        printf("��ǰ���ұ�δ��δ�����������ֽ�������...\n");
        addr_t indexStartAddr;
        if (indexTableMap.size() == 0)
            indexStartAddr = index_start;
        else {
            table_map_t::iterator iter = indexTableMap.end();
            --iter;
            indexStartAddr = (iter->second).B + 1;
        }
        unsigned long prior_IO = buff.numIO;
        addr_t indexEndAddr = buildIndex(clusterAddr, indexStartAddr);
        if (indexEndAddr != END_OF_FILE)
            printf("\n��ɣ�����д����̿飺%d-%d\n", indexStartAddr, indexEndAddr);
        indexAddr = indexStartAddr;
        printf("������������IO: %d\n\n", buff.numIO - prior_IO);
    } else {
        index_t addrItem = indexTableMap.at(table.start);
        indexAddr = addrItem.A;
    }

    // ������������ȡ��Ӧ�ľ۴ش�ŵ�ַ
    unsigned long prior_IO = buff.numIO;
    loadIndex(indexAddr);
    if(buff.numIO)
        printf("������������IO: %d\n\n", buff.numIO - prior_IO);
    else
        printf("�����ѱ����ص��ڴ��У�������أ�\n");
    vector<tree_data_t> data = BPTR.select(val, EQ);
    addr_t loadAddr = data[0];

    // ������ָ��ľ۴ش�ŵ�ַ�в��ҽ��
    block_t readBlk, resBlk;
    row_t R[numOfRowInBlk];
    resBlk.writeInit(resTable.start);
    readBlk.loadFromDisk(loadAddr);

    // ������ָ��Ŀ��в�ѯ��Ӧ���
    int readRows, cursor = 0;
    while(1) {
        readRows = read_N_Rows_From_1_Block(readBlk, R, numOfRowInBlk);
        resTable.size += readRows;
        cursor = 0;
        if (readRows > 0) {
            if (R[cursor].A > val)  {
                // �ҵ���һ�����������ֶ�ֵʱ��������
                break;
            }
            while(R[cursor].A < val)
                cursor += 1;
            while(R[cursor].A == val) {
                curAddr = resBlk.writeRow(R[cursor++]);
                if (cursor == readRows)
                    break;
            }
        } else {
            // �Ѿ������ļ���ĩβ��
            break;
        }
    }
    addr_t endAddr = resBlk.writeLastBlock();
    if (endAddr != END_OF_FILE)
        curAddr = endAddr;
    return curAddr;
}

/**
 * ����������֪�����������������ʲô����
 * 
 * �����������еĴ��ζ�ֻ��Ϊ�˴��������indexQuery
 * ���Բ������ݾ͸�indexQuery��ȫ��ͬ������Ͳ�׸����
 * ��ϸ�������������indexQuery�Ĳ�������
 */
void searchByIndex_and_Show(const table_t &table, table_t &resTable, int val) {
    clear_Buff_IO_Count();
    addr_t resEndAddr = indexQuery(table, resTable, val);
    showResult(resTable.start);
    if (resEndAddr)
        printf("\nע�����д����̿飺%d-%d\n", resTable.start, resEndAddr);
    printf("���ι�����%ld��I/O\n\n", buff.numIO);
}


/**************************** main ****************************/
int main() {
    bufferInit();
    addr_t resEndAddr;
    table_t Result_table_R = {condQueryStart, 0};
    printf("===================== ��ʼ����R�� ====================\n");
    /********* ���Լ�������ּ������Բ��� *********/
    clear_Buff_IO_Count();
    // resEndAddr = linearQuery(table_R, Result_table_R, 40, EQ_cond);
    // resEndAddr = binaryQuery(table_R, Result_table_R, 40, cmp);
    // showResult(Result_table_R.start);
    // printf("\nע�����д����̿飺%d-%d\n", Result_table_R.start, resEndAddr);
    // printf("���ι�����%ld��I/O\n\n", buff.numIO);

    /********* �����������Բ��� *********/
    printf("----- δ��������ʱ���м��� -----\n");
    searchByIndex_and_Show(table_R, Result_table_R, 40);
    printf("----- �ѽ�������ʱ���м��� -----\n");
    searchByIndex_and_Show(table_R, Result_table_R, 40);

    printf("===================== ��ʼ����S�� ====================\n");

    /********* ���Լ�������ּ������Բ��� *********/
    clear_Buff_IO_Count();
    // addr_t newStartAddr = resEndAddr + 1;
    // table_t Result_table_S = {newStartAddr, 0};
    // resEndAddr = linearQuery(table_S, Result_table_S, 60, EQ_cond);
    // resEndAddr = binaryQuery(table_S, Result_table_S, 60, cmp);
    // showResult(Result_table_S.start);
    // printf("\nע�����д����̿飺%d-%d\n", Result_table_S.start, resEndAddr);
    // printf("���ι�����%ld��I/O\n\n", buff.numIO);

    /********* �����������Բ��� *********/
    addr_t newStartAddr = 1001;
    table_t Result_table_S = {newStartAddr, 0};
    printf("----- δ��������ʱ���м��� -----\n");
    searchByIndex_and_Show(table_S, Result_table_S, 60);
    printf("----- �ѽ�������ʱ���м��� -----\n");
    searchByIndex_and_Show(table_S, Result_table_S, 60);
    printf("=================== ����R�Ƿ������� ==================\n");
    searchByIndex_and_Show(table_R, Result_table_R, 40);

    system("pause");
    return OK;
}