#include "utils.cpp"
#include "index.cpp"
#include "distinct.cpp"
#include "condQuery.cpp"
#include "project.cpp"
#include "join.cpp"
#include "setOperations.cpp"


/**
 * @brief �������������ڱ��ζ�д�е������Ϣ
 * 
 * @param table ������������Ϣ
 */
void print_IO_Info(table_t table) {
    if (table.start)
        printf("\nע�����д����̿飺%d-%d\n", table.start, table.end);
    printf("���ι�����%ld��I/O\n\n", buff.numIO);
    system("pause");
}


/**
 * @brief ɾ��res��ָ����ļ�
 * 
 * @param res ��ɾ���ı����Ϣ
 */
void dropResultTable(table_t res) {
    if (res.start && res.size)
        DropFiles(res.start);
    res.size = 0;
}


int main() {
    bufferInit();
    useCluster(table_R);
    useCluster(table_S);
    int select;
    table_t condQueryTable(condQueryStart);
    table_t projectTable(projStart);
    projectTable.rowSize = sizeOfRow / 2;
    table_t joinTable(joinResultStart);
    joinTable.rowSize = 2 * sizeOfRow;
    table_t setOperationTable(setOperationResultStart);

    while(1) {
        system("cls");
        printf("���뿴�ĸ�������ʾ�أ�\n\n");
        printf("====================================\n");
        printf("0. �˳�\n");
        printf("1. ����\n");
        printf("2. ͶӰ\n");
        printf("3. ����\n");
        printf("4. ���ϲ���\n");
        printf("====================================\n\n");
        printf("���������ѡ��");
        cin >> select;

        if (select == 0) {
            printf("\n�˳���ʾ...\n\n");
            break;
        }
        switch(select) {
            case 0:
                break;
            case 1: {
                while (1) {
                    system("cls");
                    char tableName;
                    table_t table;
                    int val;
                    printf("���뿴���ּ�����ʽ����ʾ�أ�\n\n");
                    printf("====================================\n");
                    printf("0. �ص���һ��\n");
                    printf("1. ���Լ���\n");
                    printf("2. ���ּ���\n");
                    printf("3. ��������\n");
                    printf("====================================\n\n");
                    printf("���������ѡ��");
                    cin >> select;

                    if (select == 0)
                        break;
                    else if (select < 0 || select > 3){
                        printf("����������0-3�����ѡ��Ŷ~\n");
                        system("pause");
                        continue;
                    }

                    system("cls");
                    if (select == 1)
                        printf("1. ���Լ���\n");
                    else if (select == 2)
                        printf("2. ���ּ���\n");
                    else if (select == 3)
                        printf("3. ��������\n");
                    
                    printf("\n���뿴R����S��(����R��S)\n");
                    cin >> tableName;
                    if (tableName == 'R') {
                        table = table_R;
                        // val = 40;
                    } else if (tableName == 'S') {
                        table = table_S;
                        // val = 60;
                    } else {
                        printf("����������R��S������κζ�����~\n");
                        system("pause");
                        continue;
                    }
                    printf("���������ֵ��");
                    cin >> val;

                    dropResultTable(condQueryTable);
                    condQueryTable.start = condQueryStart;
                    clear_Buff_IO_Count();
                    if (select == 1) {
                        linearQuery(table, condQueryTable, val, EQ_cond);
                        showResult(condQueryTable);
                        print_IO_Info(condQueryTable);
                    } else if (select == 2) {
                        binaryQuery(table, condQueryTable, val, cmp);
                        showResult(condQueryTable);
                        print_IO_Info(condQueryTable);
                    } else if (select == 3) {
                        searchByIndex_and_Show(table, condQueryTable, val);
                        system("pause");
                    }
                };
                break;
            } case 2: {
                char tableName;
                system("cls");
                printf("���ڹ������ƣ�����ֻ��Ϊ��ͶӰ��ĵ�һ������\n\n");
                do {
                    printf("����ͶӰ�ĸ���ĵ�һ�������أ�(R��S)\n");
                    cin >> tableName;
                    dropResultTable(projectTable);
                    if (tableName == 'R') {
                        printf("��Ϊ��ͶӰR�ĵ�һ�����ԣ�\n");
                        project(table_R, projectTable);
                        showResult(projectTable);
                        print_IO_Info(projectTable);
                    } else if (tableName == 'S') {
                        printf("��Ϊ��ͶӰS�ĵ�һ�����ԣ�\n");
                        project(table_S, projectTable);
                        showResult(projectTable);
                        print_IO_Info(projectTable);
                    } else {
                        printf("��������R���S�������ѡ��Ŷ~\n");
                        system("pause");
                    }
                    system("cls");
                } while (tableName != 'R' && tableName != 'S');
                break;
            } case 3: {
                while(1) {
                    system("cls");
                    printf("���뿴�������ӷ�ʽ����ʾ�أ�\n\n");
                    printf("====================================\n");
                    printf("0. �ص���һ��\n");
                    printf("1. Ƕ��ѭ������(NEST-LOOP JOIN)\n");
                    printf("2. ����鲢����(SORT-MERGE JOIN)\n");
                    printf("3. ɢ������(HASH JOIN)\n");
                    printf("====================================\n\n");
                    printf("���������ѡ��");
                    cin >> select;

                    system("cls");
                    clear_Buff_IO_Count();
                    if (select == 0)
                        break;

                    dropResultTable(joinTable);
                    joinTable.start = joinResultStart;
                    if (select == 1) {
                        printf("�鿴Ƕ��ѭ������(NEST-LOOP JOIN)�Ľ����\n");
                        joinTable = NEST_LOOP_JOIN(table_R, table_S);
                    } else if (select == 2) {
                        printf("�鿴����鲢����(SORT-MERGE JOIN)�Ľ����\n");
                        joinTable = SORT_MERGE_JOIN(table_R, table_S);
                    } else if (select == 3) {
                        printf("�鿴ɢ������(HASH JOIN)�Ľ����\n");
                        joinTable = HASH_JOIN(table_R, table_S);
                    } else {
                        printf("����������0-3�����ѡ��Ŷ~\n");
                        system("pause");
                        continue;
                    }
                    showResult(joinTable);
                    print_IO_Info(joinTable);
                }
                break;
            } case 4: {
                while(1) {
                    system("cls");
                    printf("���뿴���ּ��ϲ�������ʾ�أ�\n\n");
                    printf("====================================\n");
                    printf("0. �ص���һ��\n");
                    printf("1. ��\n");
                    printf("2. ��\n");
                    printf("3. ��\n");
                    printf("====================================\n\n");
                    printf("���������ѡ��");
                    cin >> select;

                    system("cls");
                    clear_Buff_IO_Count();
                    if (select == 0)
                        break;

                    dropResultTable(setOperationTable);
                    setOperationTable.start = setOperationResultStart;
                    if (select == 1) {
                        printf("�鿴R��S�Ľ����\n");
                        tablesUnion(table_R, table_S, setOperationTable);
                    } else if (select == 2) {
                        printf("�鿴R��S�Ľ����\n");
                        tablesIntersect(table_R, table_S, setOperationTable);
                    } else if (select == 3) {
                        char diffedTable, diffTable;
                        table_t table1, table2;
                        printf("�������ĸ�����ĸ���Ĳ��أ�(����R S��S R)\n");
                        cin >> diffedTable >> diffTable;
                        if (diffedTable == 'R' && diffTable == 'S') {
                            printf("�鿴R-S�Ľ����\n");
                            table1 = table_R, table2 = table_S;
                        } else if (diffedTable == 'S' && diffTable == 'R') {
                            printf("�鿴S-R�Ľ����\n");
                            table1 = table_S, table2 = table_R;
                        } else {
                            printf("\"R S\"�Լ�\"S R\"��������붼�ǲ������Ŷ~\n");
                            system("pause");
                            continue;
                        }
                        tablesDiff(table1, table2, setOperationTable);
                    } else {
                        printf("����������0-3�����ѡ��Ŷ~\n");
                        system("pause");
                        continue;
                    }
                    showResult(setOperationTable);
                    print_IO_Info(setOperationTable);
                }
                break;
            } default: {
                printf("����������0-4�����ѡ��Ŷ~\n");
                system("pause");
                break;
            }
        }
    }
    dropResultTable(condQueryTable);
    dropResultTable(projectTable);
    dropResultTable(joinTable);
    dropResultTable(setOperationTable);
    // ����۴���
    for (auto iter = clusterTableMap.begin(); iter != clusterTableMap.end(); ++iter) {
        index_t addrItem = iter->second;
        addr_t clusterAddr = addrItem.A;
        DropFiles(clusterAddr);
    }
    // ���������
    for (auto iter = indexTableMap.begin(); iter != indexTableMap.end(); ++iter) {
        index_t addrItem = iter->second;
        addr_t indexAddr = addrItem.A;
        DropFiles(indexAddr);
    }
    system("pause");
    return OK;
}