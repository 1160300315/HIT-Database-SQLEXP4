#include <map>
#include "utils.cpp"
#include "BplusTree/BplusTree.h"

/**
 * @brief ������ز���
 * 
 * �ڴ���һ�������Ų���mapʵ�ֵĵ�ַӳ���
 * �۴ص�ַӳ���(clusterTableMap)�������ڱ���ԭ���۴ر�Ĵ��̵�ַӳ���ϵ
 * ������ַӳ���(indexTableMap)�������ڱ���ԭ��������Ĵ��̵�ַӳ���ϵ
 * 
 * �¶������������
 * table_map_t: ���ӳ���ϵ����ʽΪ<ԭ���׵�ַ, (Ŀ����׵�ַ, Ŀ���ĩβ��ַ)>
 * pair_t: ���ӳ���ϵ�ԣ�����map��insert����
 */
typedef std::map<addr_t, index_t> table_map_t;
typedef std::pair<addr_t, index_t> pair_t;

const addr_t index_start = 10000;   // index����ʼ�洢λ��
const index_t invalid_index;

table_map_t clusterTableMap, indexTableMap; // ����ȫ�ֵ�ַӳ���
BPlusTree BPTR;


/**
 * @brief ��ľ۴ز�������table���о۴أ������۴ؽ�����������
 * �۴�ʵ���Ͼ���ʹ���ű�����һ����Ϊ�������ֱ��Ӧ���˹鲢����
 * ��һ��������������򣬶�Ӧscan_1_PartialSort
 * �ڶ��������������򣬶�Ӧscan_2_SortMerge
 * 
 * @param table ��ǰ���۴ر���Ϣ
 * @param clusterAddr �۴ؽ������ʼ�洢λ��
 */
void tableClustering(table_t table, addr_t clusterAddr) {
    addr_t scan_1_Index = addrOfScan_1; // һ��ɨ�����ʼ��ַ
    int rowTimes2Standard = table.rowSize / sizeOfRow;
    int numOfRows = rowTimes2Standard * (numOfRowInBlk / rowTimes2Standard);    // һ�����ж�������׼��С�ļ�¼
    int sizeOfSubTable = numOfRows * numOfBufBlock;      // �ɻ��������ֳ����ӱ��С����λ���У�
    if (sizeOfSubTable == 0)
        error("table��Ϣ��ȫ������table�Ĳ�����");
    int numOfSubTables = ceil(1.0 * rowTimes2Standard * table.size / sizeOfSubTable);   // ���ֳ����ӱ�����

    // �۴ز��������˹鲢����
    scan_1_PartialSort(numOfSubTables, scan_1_Index, table.start, sizeOfSubTable);
    addr_t endAddr = scan_2_SortMerge(numOfSubTables, scan_1_Index, clusterAddr);
    DropFiles(scan_1_Index);
    if (endAddr == ADDR_NOT_EXISTS)
        error("����ɨ����ִ���");

    // �����µľ۴���Ŀ
    index_t clusterIndex;
    clusterIndex.A = clusterAddr, clusterIndex.B = endAddr;
    clusterTableMap.insert(pair_t(table.start, clusterIndex));
}


/**
 * @brief ���������ļ�
 * ������ʽ��(�����ֶ�ֵ, ��һ�γ��ָ�ֵ���������ַ)
 * ǰ�᣺�������ı��辭���۴ز�������clusterTableMap���ж�Ӧ��ӳ����
 * 
 * @param clusteredTableStart �������ľ۴ر����ʼ��ַ
 * @param indexStart �����ļ�����ʼ��ַ
 * @return addr_t �����ļ����ڴ洢��������һ�����̿�ĵ�ַ
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
 * @brief �Ӵ����м��������ļ�
 * ������ʽ��(�����ֶ�ֵ, ��һ�γ��ֵ��������ַ)
 * 
 * @param indexStart �����ļ�����ʼ��ַ
 */
void loadIndex(addr_t indexStart) {
    addr_t next = indexStart;
    int numOfReadBlocks = numOfBufBlock;
    block_t blk[numOfReadBlocks];
    for (int i = 0; i < numOfReadBlocks; ++i) {
        blk[i].loadFromDisk(next);
        next = blk[i].readNextAddr();
        if (next == END_OF_FILE) {
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


/**
 * @brief �۴ع�����ʹ�þ۴��ļ�ǰ��׼������
 * ��table���о۴��ļ����۴���Ŀ�Ƚṹ�����ɺ͹���
 * ���������ṩ�۴صĽӿ�
 * 
 * @param table �����۴ع���ı�
 * @param clusterAddr �۴��ļ�����ʼ��ַ����ΪĬ��ֵDEFAULT_ADDR��ʹ�����еľ۴��ļ�
 *  �����øò�������clusterTableMap�ж�Ӧ����Ŀ
 */
void useCluster(table_t table, addr_t clusterAddr = DEFAULT_ADDR) {
    if (clusterAddr == DEFAULT_ADDR) {
        // ��ʹ�õ���Ĭ�Ͼ۴ص�ַ�����鵱ǰ���Ƿ��ж�Ӧ�ľ۴���Ŀ
        table_map_t::iterator findCluster = clusterTableMap.find(table.start);
        if (findCluster == clusterTableMap.end()) {
            // ���޶�Ӧ�ľ۴���Ŀ���򴴽�֮�����о۴ز���
            printf("��ǰ���ұ�δ��δ�۴أ��ֽ��о۴ز���...\n");
            if (clusterTableMap.size() == 0) {
                // ��clusterTableMap��û�о۴���Ŀ
                // ���ָ���ĵ�ַaddrOfScan_2��ʼ�����۴��ļ�
                clusterAddr = addrOfScan_2;
            } else {
                // �����һ���۴���Ŀ����һ����ַ��ʼ�����۴��ļ�
                table_map_t::iterator iter = clusterTableMap.end();
                --iter;
                clusterAddr = (iter->second).B + 1;
            }
        } else {
            return;
        }
    } else {
        // ��ʹ�õ����Զ���۴ص�ַ�����Ȳ鿴�Ƿ��ж�Ӧ�ľ۴���Ŀ��Ҫ����
        for (auto iter = clusterTableMap.begin(); iter != clusterTableMap.end(); ++iter) {
            index_t clusterIndex = iter->second;
            if (clusterIndex.A == clusterAddr) {
                printf("�˴����о۴���Ŀ��׼������...\n");
                clusterTableMap.erase(iter);    // ���ԭ���ľ۴���Ŀ
                break;
            }
        }
    }
    tableClustering(table, clusterAddr);
    printf("\n�۴���ɣ�\n");
    printf("�۴�����IO: %d\n\n", buff.numIO);
}


/**
 * @brief ����������ʹ�������ļ�ǰ��׼������
 * ��table���������ļ���������Ŀ�Ƚṹ�����ɺ͹���
 * ���������ṩ�����Ľӿ�
 * 
 * @param table �����۴ع���ı�
 * @return addr_t table��Ӧ�����ļ�����ʼ��ַ
 */
addr_t useIndex(table_t table) {
    addr_t indexAddr;
    table_map_t::iterator findIndex = indexTableMap.find(table.start);
    // ���table�������Ƿ��ѽ���
    if (findIndex == indexTableMap.end()) {
        printf("��ǰ���ұ�δ��δ�����������ֽ�������...\n");
        addr_t indexStartAddr;
        if (indexTableMap.size() == 0) {
            // ��indexTableMap��û��������Ŀ
            // ���ָ���ĵ�ַindex_start��ʼ��������
            indexStartAddr = index_start;
        } else {
            // �����һ��������Ŀ����һ����ַ��ʼ��������
            table_map_t::iterator iter = indexTableMap.end();
            --iter;
            indexStartAddr = (iter->second).B + 1;
        }
        unsigned long prior_IO = buff.numIO;
        addr_t clusterAddr = clusterTableMap.at(table.start).A;
        addr_t indexEndAddr = buildIndex(clusterAddr, indexStartAddr);
        if (indexEndAddr != END_OF_FILE)
            printf("\n��ɣ�����д����̿飺%d-%d\n", indexStartAddr, indexEndAddr);
        indexAddr = indexStartAddr;
        printf("������������IO: %d\n\n", buff.numIO - prior_IO);
    } else {
        // ���ж�Ӧ��������Ŀ����ֱ������������Ŀ����
        index_t addrItem = indexTableMap.at(table.start);
        indexAddr = addrItem.A;
    }
    return indexAddr;
}
