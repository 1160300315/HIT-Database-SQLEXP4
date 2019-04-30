#include "utils.cpp"


const addr_t setOperationResultStart = 4000;    // ���ϲ����������ʼ��ŵ�ַ


/**
 * ��Ĳ�����
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
void tablesUnion(addr_t R_addr, addr_t S_addr) {
}

/**
 * ��Ľ�����
 * ����Ƕ��ѭ���ķ������м���
 * 
 * R_addr: ��һ�����ڴ����е���ʼ��ַ
 * S_addr: �ڶ������ڴ����е���ʼ��ַ
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
addr_t tablesIntersect(addr_t R_addr, addr_t S_addr) {
    int numOfR = 6;
    int numOfRows_R = numOfRowInBlk * numOfR, numOfRows_S = numOfRowInBlk;
    addr_t curAddr = 0, resAddr = setOperationResultStart;
    
    block_t Rblk[numOfR], Sblk, resBlk;
    for (int i = 0; i < numOfR; ++i)
        Rblk[i].loadFromDisk(R_addr++);
    resBlk.writeInit(resAddr);
    row_t t_R[numOfRows_R], t_S[numOfRows_S];

    int readRows_R, readRows_S;
    while(1) {
        // ÿ�δ�R�ж���6��
        readRows_R = read_N_Rows_From_M_Block(Rblk, t_R, numOfRows_R, numOfR);
        // printRows(t, readRows, val);
        Sblk.loadFromDisk(S_addr);
        while(1) {
            // ÿ�δ�S�ж���1����бȽ�
            readRows_S = read_N_Rows_From_1_Block(Sblk, t_S, numOfRows_S);
            for (int i = 0; i < numOfRows_R; ++i) {
                for (int j = 0; j < numOfRows_S; ++j) {
                    if (t_R[i] == t_S[j]) {
                        curAddr = resBlk.writeRow(t_R[i]);
                    }
                }
            }
            if (readRows_S < numOfRows_S)   // S������
                break;
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

/**
 * ��Ĳ����
 * 
 * return: ���ӽ�������������һ��ĵ�ַ
 */
void tablesDiff(addr_t R_addr, addr_t S_addr) {

}


/**************************** main ****************************/
int main() {
    bufferInit();
    // tablesUnion(R_start, S_start);
    addr_t resEndAddr = tablesIntersect(R_start, S_start);
    // tablesDiff(R_start, S_start);

    showResult(setOperationResultStart);
    if (resEndAddr)
        printf("\nע�����д����ʼ���̿飺4000-%d\n", resEndAddr);
    printf("���ι�����%ld��I/O\n\n", buff.numIO);

    system("pause");
    return OK;
}