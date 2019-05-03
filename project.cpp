#include "utils.cpp"
#pragma once


/**
 * @brief ͶӰ����
 * ͶӰR���еķ�����A����
 */

const addr_t projStart = 2000;  // ͶӰ�������ʵ��ŵ�ַ


/**
 * @brief ͶӰ
 * �Ӷ�ȡ�ı��г�ȡÿһ����¼��A����д����
 * 
 * @param projTable ��ͶӰ�����Ϣ
 * @param resTable ͶӰ��������Ϣ
 */
void project(table_t projTable, table_t resTable) {
    block_t blk, resBlk;
    row_t t_read, t_write;
    addr_t curAddr = 0;
    blk.loadFromDisk(projTable.start);
    resBlk.writeInit(resTable.start);
    bool isLastRow = false;
    while(1) {
        t_read = blk.getNewRow();
        if(t_read.isFilled == false) {
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                curAddr = endAddr;
            break;
        }
        t_write.A = t_read.A;
        t_read = blk.getNewRow();
        if(t_read.isFilled == false) {
            t_write.B = MAX_ATTR_VAL;
            isLastRow = true;
        } else {
            t_write.B = t_read.A;
        }
        resBlk.writeRow(t_write);
        resTable.size += 1;
    }
    curAddr = resBlk.writeLastBlock();
    resTable.end = curAddr;
}


/**************************** main ****************************/
// int main() {
//     bufferInit();
//     table_t projRes(projStart);
//     project(table_R, projStart);
//     showResult(projStart, sizeOfRow / 2);
//     printf("\nע�����д����̿飺%d-%d\n", projRes.start, projRes.end);
//     printf("���ι�����%ld��I/O\n", buff.numIO);
//     system("pause");
//     return OK;
// }