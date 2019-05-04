#include "utils.cpp"
#include "index.cpp"
#pragma once


/**
 * @brief ���������㷨
 * ������R.A = 40  ��  S.C = 60
 */

const addr_t condQueryStart = 1000; //�������������ʵ��ŵ�ַ


/**
 * @brief �ȽϺ���
 * 
 * @param R ���Ƚϵļ�¼
 * @param val �Ƚ�ֵ
 * @return int ��¼R���val��λ��
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
 * @brief �����������
 * ����¼�е�A������Ƚ�ֵval�Ƚϣ������Ƿ�����ж�
 * 
 * @param R ���Ƚϵļ�¼
 * @param val �Ƚ�ֵ
 * @return true ��¼��A�ֶε�ֵ��val���
 * @return false ��¼��A�ֶε�ֵ��val����
 */
bool EQ_cond(row_t R, int val) { return (R.A == val); }


/**
 * @brief ������Լ���
 * �Դ������ı��ռ�������cond�ͼ���ֵval�������Լ���
 * 
 * @param table �������ı���Ϣ
 * @param resTable �����������Ϣ
 * @param val ����ֵ
 * @param cond ������������
 */
void linearQuery(const table_t &table, table_t &resTable, int val, bool (*cond)(row_t, int)) {
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
            if (cond(t[i], val))  {
                curAddr = resBlk.writeRow(t[i]);
                resTable.size += 1;
            }
        }
        if (isLastBlock) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            resTable.end = curAddr;
            break;
        }
    }
    // ����ս����
    if (resTable.size == 0)
        resTable.start = resTable.end = 0;
}


/**
 * @brief ���ּ���
 * �Դ������ı���ƥ������cmp��ƥ��ֵval���ж��ּ���
 * 
 * @param table �������ı���Ϣ
 * @param resTable �����������Ϣ
 * @param val ƥ��ֵ
 * @param cmp ���������ȽϺ������ܹ���ӳ��Ŀ���¼�뵱ǰ������¼��λ����Ϣ
 */
void binaryQuery(const table_t &table, table_t &resTable, int val, int (*cmp)(row_t, int)) {
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
        resBlk.freeBlock();                 // ��Ҫ�ֶ��ͷŽ��������
        resTable.start = resTable.end = 0;  // ����ս����
        return;
    }
    insertSort<row_t>(res, pRes);
    for (int i = 0; i < pRes; ++i) {
        curAddr = resBlk.writeRow(res[i]);
        resTable.size += 1;
    }
    addr_t endAddr = resBlk.writeLastBlock();
    if (endAddr != END_OF_FILE)
        curAddr = endAddr;
    resTable.end = curAddr;
}


/**
 * @brief ����������
 * �Դ������ı�����val��Ӧ��������Ϣ�����ض�Ӧ�Ĵ��̿���ٽ������Լ���
 * ���ñ�δ���۴�/���������Ĳ��������ȶԸñ����۴�/��������
 * 
 * @param table �������ı���Ϣ
 * @param resTable �����������Ϣ
 * @param val ����ֵ
 */
void indexQuery(const table_t &table, table_t &resTable, int val) {
    addr_t clusterAddr, indexAddr, curAddr = 0;
    clusterAddr = useCluster(table);
    indexAddr = useIndex(table);

    // ������������ȡ��Ӧ�ľ۴ش�ŵ�ַ
    unsigned long prior_IO = buff.numIO;
    loadIndex(indexAddr);
    if(buff.numIO)
        printf("������������IO: %d\n\n", buff.numIO - prior_IO);
    else
        printf("�����ѱ����ص��ڴ��У�������أ�\n");
    vector<tree_data_t> data = BPTR.select(val, EQ);
    if (data.begin() == data.end()) {
        resTable.start = resTable.end = resTable.size = 0;
        return;
    }
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
        cursor = 0;
        if (readRows > 0) {
            if (R[cursor].A > val)  {
                // �ҵ���һ�����������ֶ�ֵʱ��������
                readBlk.freeBlock();
                break;
            }
            while(R[cursor].A < val)
                cursor += 1;
            while(R[cursor].A == val) {
                curAddr = resBlk.writeRow(R[cursor++]);
                resTable.size += 1;
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
    resTable.end = curAddr;
}

/**
 * @brief ����������֪�����������������ʲô����
 * 
 * �����������еĴ��ζ�ֻ��Ϊ�˴��������indexQuery
 * ���Բ������ݾ͸�indexQuery��ȫ��ͬ������Ͳ�׸����
 * ��ϸ�������������indexQuery�Ĳ�������
 */
void searchByIndex_and_Show(const table_t &table, table_t &resTable, int val) {
    clear_Buff_IO_Count();
    indexQuery(table, resTable, val);
    showResult(resTable.start);
    printf("\nע�����д����̿飺%d-%d\n", resTable.start, resTable.end);
    printf("���ι�����%ld��I/O\n\n", buff.numIO);
}


/**************************** main ****************************/
// int main() {
//     bufferInit();
//     table_t Result_table_R(condQueryStart);
//     printf("===================== ��ʼ����R�� ====================\n");
//     clear_Buff_IO_Count();
//     /********* ���Լ�������ּ������Բ��� *********/
//     // linearQuery(table_R, Result_table_R, 40, EQ_cond);
//     // binaryQuery(table_R, Result_table_R, 40, cmp);
//     // showResult(Result_table_R.start);
//     // printf("\nע�����д����̿飺%d-%d\n", Result_table_R.start, Result_table_R.end);
//     // printf("���ι�����%ld��I/O\n\n", buff.numIO);

//     /********* �����������Բ��� *********/
//     printf("----- δ��������ʱ���м��� -----\n");
//     searchByIndex_and_Show(table_R, Result_table_R, 40);
//     printf("----- �ѽ�������ʱ���м��� -----\n");
//     searchByIndex_and_Show(table_R, Result_table_R, 40);

//     printf("===================== ��ʼ����S�� ====================\n");
//     clear_Buff_IO_Count();
//     addr_t newStartAddr = Result_table_R.end + 1;
//     /********* ���Լ�������ּ������Բ��� *********/
//     // table_t Result_table_S(newStartAddr);
//     // linearQuery(table_S, Result_table_S, 60, EQ_cond);
//     // binaryQuery(table_S, Result_table_S, 60, cmp);
//     // showResult(Result_table_S.start);
//     // printf("\nע�����д����̿飺%d-%d\n", Result_table_S.start, Result_table_S.end);
//     // printf("���ι�����%ld��I/O\n\n", buff.numIO);

//     /********* �����������Բ��� *********/
//     table_t Result_table_S(newStartAddr);
//     printf("----- δ��������ʱ���м��� -----\n");
//     searchByIndex_and_Show(table_S, Result_table_S, 60);
//     printf("----- �ѽ�������ʱ���м��� -----\n");
//     searchByIndex_and_Show(table_S, Result_table_S, 60);
//     printf("=================== ����R�Ƿ������� ==================\n");
//     searchByIndex_and_Show(table_R, Result_table_R, 40);

//     system("pause");
//     return OK;
// }