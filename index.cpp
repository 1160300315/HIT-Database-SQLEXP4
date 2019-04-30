#include <map>
#include "utils.cpp"
#include "BplusTree/BplusTree.h"

/**
 * ������ز���
 * 
 * �ڴ���һ�������Ų���mapʵ�ֵĵ�ַӳ���
 * �۴ص�ַӳ���(clusterTableMap)�������ڱ���ԭ���۴ر�Ĵ��̵�ַӳ���ϵ
 * ������ַӳ���(indexTableMap)�������ڱ���ԭ��������Ĵ��̵�ַӳ���ϵ
 * 
 * table_map_t: ���ӳ���ϵ����ʽΪ<ԭ���׵�ַ, (Ŀ����׵�ַ, Ŀ���ĩβ��ַ)>
 * pair_t: ���ӳ���ϵ�ԣ�����map��insert����
 */

typedef std::map<addr_t, index_t> table_map_t;
typedef std::pair<addr_t, index_t> pair_t;

const addr_t index_start = 10000;
const int maxNumOfIndex = 10;
const index_t invalid_index;

table_map_t clusterTableMap, indexTableMap; // ����ȫ�ֵ�ַӳ���
BPlusTree BPTR; // ��ȫ������1��B+��


/**
 * ��ľ۴ز�������table���о۴أ������۴ؽ�����������
 * �۴�ʵ���Ͼ���ʹ���ű�����һ����Ϊ�������ֱ��Ӧ���˹鲢����
 * ��һ��������������򣬶�Ӧscan_1_PartialSort
 * �ڶ��������������򣬶�Ӧscan_2_SortMerge
 * 
 * table: ��ǰ���۴ر���Ϣ
 * clusterAddr: �۴ؽ������ʼ�洢λ��
 * 
 * return: �۴ؽ�����ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t tableClustering(table_t table, addr_t clusterAddr) {
    addr_t scan_1_Index = addrOfScan_1;
    int sizeOfSubTable = numOfRowInBlk * numOfBufBlock;       // �ɻ��������ֳ����ӱ��С
    int numOfSubTables = ceil(1.0 * table.size / sizeOfSubTable);   // R���ֳ����ӱ�����
    scan_1_PartialSort(numOfSubTables, scan_1_Index, table.start, sizeOfSubTable);
    addr_t retAddr = scan_2_SortMerge(numOfSubTables, scan_1_Index, clusterAddr);

    // ���۴ص�ַ�Ƿ��ж�Ӧ����Ŀ�������򸲸�֮
    for (auto iter = clusterTableMap.begin(); iter != clusterTableMap.end(); ++iter) {
        index_t clusterIndex = iter->second;
        if (clusterIndex.A == clusterAddr) {
            printf("�˴����о۴���Ŀ��׼������...\n");
            clusterTableMap.erase(iter);
            printf("������ɣ�\n");
            break;
        }
    }
    index_t clusterIndex;
    clusterIndex.A = clusterAddr, clusterIndex.B = retAddr;
    clusterTableMap.insert(pair_t(table.start, clusterIndex));
    return retAddr;
}

/**
 * ���������ļ�
 * ������ʽ��(�����ֶ�ֵ, ��һ�γ��ָ�ֵ���������ַ)
 * ǰ�᣺�������ı��辭���۴ز���
 * 
 * clusteredTableStart: �������ľ۴ر����ʼ��ַ
 * indexStart: �����ļ�����ʼ��ַ
 * 
 * return: �����ļ����ڴ洢��������һ�����̿�ĵ�ַ
 */
addr_t buildIndex(addr_t clusteredTableStart, addr_t indexStart) {
    addr_t next = clusteredTableStart, indexAddr = 0;
    int numOfReadBlocks = numOfBufBlock - 1;
    int numOfIndices = numOfRowInBlk * numOfReadBlocks;

    // ���Ҿ۴ص�ַӳ������Ƿ��ж�Ӧ����
    addr_t tableAddr;
    auto findOrigin = std::find_if(clusterTableMap.begin(), clusterTableMap.end(),
        [clusteredTableStart](const table_map_t::value_type item)
    {   // ���Ҿ۴ر��׵�ַ��Ӧ��ԭ���׵�ַ
        index_t clusterIndex = item.second;
        return (clusterIndex.A == clusteredTableStart);
    });
    if (findOrigin == clusterTableMap.end()) {
        // ����Ĭ����buildIndex֮ǰ�ñ��������۴�(tableClustering)
        // �������û���ҵ���Ӧ���׵�ַ��֤���þ۴ر����Ϣ��ʧ��
        printf("���ش��󣺾۴���Ϣ��ʧ��\n");
        system("pause");
        exit(0);
    }
    tableAddr = findOrigin->first;  // ��ȡ��Ӧ��ԭ���׵�ַ

    row_t R[numOfIndices], R_prior;
    block_t blk[numOfReadBlocks], resBlk;
    resBlk.writeInit(indexStart);
    for (int i = 0; i < numOfReadBlocks; ++i)
        blk[i].loadFromDisk(next++);
    int readRows, count = 0;
    while(1) {
        // ˳�������Ѱ��ÿ�������ֶ�ֵ��һ�γ��ֵĿ��ַ������д������
        readRows = read_N_Rows_From_M_Block(blk, R, numOfIndices, numOfReadBlocks);
        insertSort<row_t>(R, readRows);
        // printRows(R, readRows);
        for (int i = 0; i < readRows; ++i, ++count) {
            if (R_prior.isFilled == false || R_prior.A < R[i].A) {
                // �µ������ֶ�ֵ����һ�������ֶ�ֵ��ͬ������ֵ��һ�γ���
                index_t indexItem;
                indexItem.A = R[i].A;
                indexItem.B = clusteredTableStart + count / numOfRowInBlk;
                indexAddr = resBlk.writeRow(indexItem);
            }
            R_prior = R[i];
        }
        if (readRows < numOfIndices) {
            // �۴ر��Ѷ���
            addr_t endAddr = resBlk.writeLastBlock();
            if (endAddr != END_OF_FILE)
                indexAddr = endAddr;
            break;
        }
    }

    // д������ַӳ���
    table_map_t::iterator findIndex;
    findIndex = indexTableMap.find(tableAddr);
    if (findIndex != indexTableMap.end()) {
        printf("�ô����������׼������...\n");
        indexTableMap.erase(findIndex);
    }
    index_t addrItem;
    addrItem.A = indexStart, addrItem.B = indexAddr;
    indexTableMap.insert(pair_t(tableAddr, addrItem));
    return indexAddr;
}

/**
 * �Ӵ����м��������ļ�
 * ������ʽ��(�����ֶ�ֵ, ��һ�γ��ֵ��������ַ)
 * 
 * indexStart: �����ļ�����ʼ��ַ
 */
void loadIndex(addr_t indexStart) {
    addr_t next = indexStart;
    int numOfReadBlocks = numOfBufBlock;
    block_t blk[numOfReadBlocks];
    for (int i = 0; i < numOfReadBlocks; ++i) {
        blk[i].loadFromDisk(next++);
        if (blk[i].readNextAddr() == END_OF_FILE) {
            numOfReadBlocks = i + 1;
            break;
        }
    }
    int numOfIndices = numOfRowInBlk * numOfReadBlocks;
    index_t index[numOfIndices];
    BPTR.clear();

    int readRows, numOfUsedBlocks;
    while(1) {
        readRows = read_N_Rows_From_M_Block(blk, index, numOfIndices, numOfReadBlocks);
        numOfUsedBlocks = ceil(1.0 * readRows / numOfRowInBlk);
        insertSort<row_t>(index, readRows);
        // printRows(R, readRows);
        for (int i = 0; i < readRows; ++i) {
            BPTR.insert(index[i].A, index[i].B);
        }
        if (numOfUsedBlocks < numOfReadBlocks) {
            for (int i = 0; i < numOfUsedBlocks; ++i)
                blk[i].freeBlock();
            break;
        }
    }
    // BPTR.printData();
}