#include <iostream>
#include "Block.h"
extern "C" {
    #include "extmem.c"
}
#pragma once

#define CHAR_ZERO_ASCII '0'
#define CHAR_NINE_ASCII '9'
#define CHAR_EMPTY_ASCII 0

Block::Block() {
    // memset(blkData, 0, endOfBlock);
    endAddrOfData = addrOfLastRow;
}

/**
 * @brief ����ͷ�
 * �ӻ�����buff���ͷŸÿ飬�����һЩ��Ӧ�Ļ��չ���
 */
void Block::freeBlock() {
    readBlkAddr = 0;
    writeBlkAddr = 0;
    cursor = BLK_START_ADDR;        // ��дָ�븴λ
    freeBlockInBuffer(blkData, &buff);
    memset(blkData, 0, endOfBlock); //����ոÿ������
}

/**
 * @brief �Ӵ����м��ص�ַΪaddr�Ĵ��̿鵽��������
 * 
 * @param addr ��Ҫ���صĴ��̿��ַ
 * @param endPos �ô��̿��ĩβλ�ã���һ���̶��Ͼ����˸ô��̶�ȡ�ļ�¼����
 */
void Block::loadFromDisk(addr_t addr, cursor_t endPos) {
    if ((blkData = readBlockFromDisk(addr, &buff)) == NULL) {
        printf("Fail to read from disk %d!\n", addr);
        system("pause");
        exit(FAIL);
    }
    // printf("װ�ص�%d��\n", addr);
    readBlkAddr = addr;
    cursor = BLK_START_ADDR;    // ָ�븴λ
    endAddrOfData = endPos;
}

/**
 * @brief ���д��ʼ��
 * 
 * @param filename д���ļ������ڴ˴�����ַ
 * @param maxNumOfRows �ÿ�д�������¼����
 */
void Block::writeInit(const file_t filename, int maxNumOfRows) {
    writeBlkAddr = filename;
    cursor = BLK_START_ADDR;
    if ((blkData = getNewBlockInBuffer(&buff)) == NULL){
        printf("Buffer Allocation Error!\n");
        system("pause");
        exit(FAIL);
    }
    memset(blkData, 0, endOfBlock);
    endAddrOfData = maxNumOfRows * sizeOfRow;
}

/**
 * @brief д�����һ�鲢�ͷ�д��
 * 
 * @return addr_t ��ǰд�����һ���д���ַ
 */
addr_t Block::writeLastBlock() {
    if (cursor > BLK_START_ADDR) {
        // ��ʾд�����滹��ʣ�������
        _writeAddr(END_OF_FILE);
        _writeToDisk(writeBlkAddr);
        writeBlkAddr += 1;
        return writeBlkAddr - 1;
    } else {
        freeBlock();
        return 0;
    }
}

/**
 * @brief �ӵ�ǰ��Block�л�ȡһ���¼�¼
 * ���Զ��������ܣ���ǰ�����ʱ���Զ���ȡ��һ����ַ��������
 * ��ǰ�ļ�����ʱ�����۽�����������ȡ���ٴΣ������ؿռ�¼
 * 
 * @return row_t ��ȡ�ļ�¼���
 */
row_t Block::getNewRow() {
    row_t R, emptyRow;
    if (cursor == endAddrOfData) {
        // ��block�����������Ѷ�ȡ��ɣ���Ҫ��ȡһ���µ�block����
        addr_t nextAddr = readNextAddr();
        freeBlock();    // ��Ҫ��������������ͷ�
        if (nextAddr == END_OF_FILE) {
            // ��ǰ�Ѷ����ļ�ĩβ��������һ��ɶ�ʱ�����ؿռ�¼
            return emptyRow;
        } else {
            loadFromDisk(nextAddr, endAddrOfData);
        }
    }
    // ��ȡRow�е�����ֵA, B
    char str1[5] = "\0", str2[5] = "\0";
    for (int i = 0; i < 4; ++i) {
        str1[i] = *(blkData + cursor + i);
        if (str1[i] == '\0')
            str1[i] = 32;
        str2[i] = *(blkData + cursor + 4 + i);
        if (str2[i] == '\0')
            str2[i] = 32;
    }
    int scan1 = sscanf(str1, "%4d", &R.A);
    int scan2 = sscanf(str2, "%4d", &R.B);
    R.isFilled = true;
    if (scan1 <= 0)
        R.isFilled = false;
    if (scan2 <= 0)
        R.B = MAX_ATTR_VAL;
    cursor += sizeOfRow;    // cursor�ƶ�����һ����¼
    return R;
}

/**
 * @brief ��ǰ��Block��д��һ���¼�¼
 * ������д��ʱ���Զ�д�����
 * 
 * @param R д��ļ�¼
 * @return addr_t ��ǰд��Ŀ�ĵ�ַ
 */
addr_t Block::writeRow(const row_t R) {
    // ���ж�block�Ƿ���
    if (cursor == endAddrOfData) {
        // printf("��һ����Ľ����");
        try {
            addr_t next = writeBlkAddr + 1;
            _writeAddr(next);
            _writeToDisk(writeBlkAddr);
            if (next == END_OF_FILE) {
                writeBlkAddr = END_OF_FILE;
                cursor = endOfBlock;
            } else {
                // д������д���������뻺����
                if ((blkData = getNewBlockInBuffer(&buff)) == NULL)
                    throw std::bad_alloc();
                writeBlkAddr += 1;
                memset(blkData, 0, sizeOfRow * (numOfRowInBlk + 1));
                cursor = BLK_START_ADDR;
            }
        } catch(const std::bad_alloc &e) {
            std::cerr << e.what() << std::endl;
            printf("Buffer Allocation Error!\n");
            system("pause");
            exit(FAIL);
        }
    }
    // ��д���ݵ�block��
    char a_buf[5] = "\0", b_buf[5] = "\0";
    if (R.A < MAX_ATTR_VAL)
        snprintf(a_buf, 5, "%-4d", R.A);
    if (R.B < MAX_ATTR_VAL)
        snprintf(b_buf, 5, "%-4d", R.B);
    for (int i = 0; i < 4; ++i) {
        if (a_buf[i] < CHAR_ZERO_ASCII || a_buf[i] > CHAR_NINE_ASCII)
            a_buf[i] = CHAR_EMPTY_ASCII;
        *(blkData + cursor + i) = a_buf[i];
        if (b_buf[i] < CHAR_ZERO_ASCII || b_buf[i] > CHAR_NINE_ASCII)
            b_buf[i] = CHAR_EMPTY_ASCII;
        *(blkData + cursor + 4 + i) = b_buf[i];
    }
    cursor += sizeOfRow;
    return writeBlkAddr;
}

/**
 * @brief ��ȡ��ǰ��ָ�����һ����ַ
 * 
 * @return addr_t ��ȡ�ĵ�ַ
 */
addr_t Block::readNextAddr() {
    char nextAddr[8] = "\0";
    unsigned int offset = addrOfLastRow;
    for (int i = 0; i < 8; ++i) {
        nextAddr[i] = *(blkData + offset + i);
    }
    addr_t next;
    if (sscanf(nextAddr, "%8d", &next) < 0)
        next = END_OF_FILE;
    return next;
}

// -----------------------------------------------------------
//                       Private Methods                      
// -----------------------------------------------------------

/**
 * @brief д��ַ����ǰ����
 * 
 * @param nextAddr д��ĵ�ַ
 */
void Block::_writeAddr(addr_t nextAddr) {
    char buf[9];
    unsigned int offset = addrOfLastRow;
    snprintf(buf, 8, "%-8d", nextAddr);
    for (int i = 0; i < 8; ++i) {
        if (buf[i] < CHAR_ZERO_ASCII || buf[i] > CHAR_NINE_ASCII)
            buf[i] = CHAR_EMPTY_ASCII;
        *(blkData + offset + i) = buf[i];
    }
}

/**
 * @brief ����ǰ�������д�뵽������
 * 
 * @param addr д��Ĵ��̵�ַ
 */
void Block::_writeToDisk(addr_t addr) {
    if (writeBlockToDisk(blkData, addr, &buff) != 0) {
        printf("Fail to write to disk %d!\n", addr);
        system("pause");
        exit(FAIL);
    }
    // printf("д���̵���%d��\n", addr);
    cursor = BLK_START_ADDR;    // ָ�븴λ
    memset(blkData, 0, endOfBlock);
}


// -----------------------------------------------------------
//                 Integrated Block Operations                
// -----------------------------------------------------------

/**
 * @brief ��ʼ��ȫ�ֻ�����
 * ������ʹ�û�������main�����У���ʹ�û�����ǰ����Ҫ���ô˺�������һ��ȫ�ֻ���������
 * ������޷�����ʹ�û�����
 */
void bufferInit() {
    if (!initBuffer(520, 64, &buff)) {
        perror("Buffer Initialization Failed!\n");
        system("pause");
        exit(FAIL);
    }
}


/**
 * @brief ��ջ�������IO�ļ���
 */
void clear_Buff_IO_Count() { buff.numIO = 0; }


/**
 * @brief ֻ����1��Block��������ȡһ���������N����¼
 * ��ȡ����βʱ�����Զ��黹��������
 * ǰ�᣺blk�����ѱ���ʼ��(ͨ��loadFromDisk�ȷ���)
 * 
 * @param readBlk ���ڶ�ȡ��¼�Ļ�������
 * @param R ���ڶ�ȡ��¼����
 * @param N ��¼�������󳤶�
 * @return int ʵ�ʶ�ȡ�ļ�¼����
 */
int read_N_Rows_From_1_Block(block_t &readBlk, row_t *R, int N) {
    int totalRead = N;
    for (int i = 0; i < N; ++i) {
        R[i] = readBlk.getNewRow();
        if (R[i].isFilled == false) {
            // ��Щblock��һ����������7����¼��һ���Ǳ��ĩβ
            totalRead = i;
            break;
        }
    }
    return totalRead;
}


/**
 * @brief ����M��Block��������ȡһ���������N����¼
 * ��ȡ��N����¼�󣬻��Զ�������һ���鵽�������У�������һ�ζ�ȡ
 * ��ȡ����βʱ�����Զ��黹��������
 * ǰ�᣺����M��blk�������ѱ���ʼ��(ͨ��loadFromDisk�ȷ���)
 * 
 * @param readBlk ���ڶ�ȡ��¼��������������
 * @param R ���ڶ�ȡ��¼����
 * @param N ��¼�������󳤶�
 * @param M block���������
 * @return int ʵ�ʶ�ȡ�ļ�¼����
 */
int read_N_Rows_From_M_Block(block_t *readBlk, row_t *R, int N, int M) {
    int totalRead = 0;
    row_t rtemp;
    // �ӵ�ǰ��M���ж�ȡ��¼
    for (int i = 0; i < N; ) {
        for (int j = 0; j < M; ++j) {
            rtemp = readBlk[j].getNewRow();
            i += 1;
            if (rtemp.isFilled)
                R[totalRead++] = rtemp;
        }
    }
    addr_t next = readBlk[M - 1].readNextAddr();
    int count = 0;
    if (next == END_OF_FILE) {
        // ���һ����û��ʹ�ã�ֻ�����Ǵ����Ѿ����黹��״̬
        // ��ʱ�������ĵ�ַһ����END_OF_FILE���Դ��ж��Ƿ������β
        if (totalRead > 0) {
            do {    // �黹ʣ��Ļ�����
                next = readBlk[count].readNextAddr();
                readBlk[count].freeBlock();
                count += 1;
            } while(next != END_OF_FILE);
        } else {
            // ��һ�ָպö������ű���һ�ֲ���Ҫ�黹������
            // ��Ϊ����һ�����л��������ѱ��ͷ�
            ;
        }
    } else {
        while(count < M && next != END_OF_FILE) {
            // װ�غ���ʣ��Ŀ飬��next=END_OF_FILEʱ���Ѷ���
            readBlk[count].freeBlock();
            readBlk[count].loadFromDisk(next);
            next = readBlk[count].readNextAddr();
            count += 1;
        }
        for (int i = 0; i < M - count; ++i) {
            // �黹û�����ϵĻ�������
            readBlk[count + i].freeBlock();
        }
    }
    return totalRead;
}
