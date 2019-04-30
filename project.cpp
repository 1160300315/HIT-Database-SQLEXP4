#include "utils.cpp"


/**
 * ͶӰ������ͶӰR.A
 */

const addr_t projStart = 2000;  // ͶӰ�������ʵ��ŵ�ַ

/**
 * ͶӰ
 * �Ӷ�ȡ�ı��г�ȡÿһ����¼��A����д����
 * 
 * readStartAddr: ��ͶӰ�����ʼ��ַ
 * resStartAddr: ͶӰ�����ŵ���ʼ��ַ
 * 
 * return: ͶӰ��������������һ��ĵ�ַ
 */
addr_t project(addr_t readStartAddr, addr_t resStartAddr) {
    block_t blk, resBlk;
    row_t t_read, t_write;
    addr_t curAddr = 0;
    blk.loadFromDisk(readStartAddr);
    resBlk.writeInit(resStartAddr);
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
        curAddr = resBlk.writeRow(t_write);
    }
    return curAddr;
}


/**************************** main ****************************/
int main() {
    bufferInit();
    addr_t resEndAddr = project(R_start, projStart);
    showResult(projStart, sizeOfRow / 2);
    if (resEndAddr)
        printf("\nע�����д����̿飺2000-%d\n", resEndAddr);
    printf("���ι�����%ld��I/O\n", buff.numIO);
    system("pause");
    return OK;
}