#include "utils.cpp"
#include "index.cpp"
#include "condQuery.cpp"
#include "distinct.cpp"
#include "join.cpp"
#include "setOperations.cpp"


void print_IO_Info(tablt_t table) {
    printf("\n注：结果写入磁盘块：%d-%d\n", table.start, table.end);
    printf("本次共发生%ld次I/O\n\n", buff.numIO);
    system("pause");
}


int main() {
    bufferInit();
    int select;
    table_t condQueryTable(condQueryStart);
    table_t projectTable(projStart);
    table_t joinTable(joinResultStart);
    table_t setOperationTable(setOperationResultStart);

    do {
        system("cls");
        printf("您想看哪个功能演示呢？\n\n");
        printf("====================================\n");
        printf("0. 退出\n");
        printf("1. 检索\n");
        printf("2. 投影\n");
        printf("3. 连接\n");
        printf("4. 集合操作\n");
        printf("====================================\n\n");
        printf("请给出您的选择：");
        scanf("%d", &select);

        if (select == 0) {
            printf("\n退出演示...\n\n");
            break;
        }
        switch(select) {
            case 0:
                break;
            case 1: {
                do {
                    system("cls");
                    char tableName;
                    table_t table;
                    int val;
                    printf("您想看哪种检索方式的演示呢？\n\n");
                    printf("====================================\n");
                    printf("0. 回到上一级\n");
                    printf("1. 线性检索\n");
                    printf("2. 二分检索\n");
                    printf("3. 索引检索\n");
                    printf("====================================\n\n");
                    printf("请给出您的选择：");
                    scanf("%d", &select);
                    printf("\n您想看R表还是S表？(输入R或S)\n");
                    scanf("%c", &tableName);
                    if (tableName == 'R') {
                        table = table_R;
                        val = 40;
                    } else if (tableName = 'S') {
                        table = table_S;
                        val = 60;
                    } else {
                        printf("不可以输入R或S以外的任何东西噢~\n");
                        system("pause");
                        continue;
                    }

                    system("cls");
                    clear_Buff_IO_Count();
                    if (select == 1) {
                        linearQuery(table, condQueryTable, val, EQ_cond);
                        showResult(condQueryTable.start);
                        print_IO_Info(condQueryTable);
                    } else if (select == 2) {
                        binaryQuery(table, condQueryTable, val, cmp);
                        showResult(condQueryTable.start);
                        print_IO_Info(condQueryTable);
                    } else if (select == 3) {
                        searchByIndex_and_Show(table, condQueryTable, val);
                        system("pause");
                    } else {
                        printf("不可以做出0-3以外的选择哦~\n");
                        system("pause");
                        continue;
                    }
                } while (select);
                break;
            } case 2: {

                break;
            } case 3: {
                do {
                    system("cls");
                    printf("您想看哪种连接方式的演示呢？\n\n");
                    printf("====================================\n");
                    printf("0. 回到上一级\n");
                    printf("1. 嵌套循环连接(NEST-LOOP JOIN)\n");
                    printf("2. 排序归并连接(SORT-MERGE JOIN)\n");
                    printf("3. 散列连接(HASH JOIN)\n");
                    printf("====================================\n\n");
                    printf("请给出您的选择：");
                    scanf("%d", &select);

                    system("cls");
                    clear_Buff_IO_Count();
                    if (select == 0)
                        break;
                    else if (select == 1) {
                        printf("查看嵌套循环连接(NEST-LOOP JOIN)的结果：\n");
                        joinTable = NEST_LOOP_JOIN(table_R, table_S);
                    } else if (select == 2)
                        printf("查看排序归并连接(SORT-MERGE JOIN)的结果：\n");
                        joinTable = SORT_MERGE_JOIN(table_R, table_S);
                    } else if (select == 3)
                        printf("查看散列连接(HASH JOIN)的结果：\n");
                        joinTable = HASH_JOIN(table_R, table_S);
                    } else {
                        printf("不可以做出0-3以外的选择哦~\n");
                        system("pause");
                        continue;
                    }
                    showResult(joinTable.start);
                    print_IO_Info(joinTable);
                } while (select);
            } case 4: {
                do {
                    system("cls");
                    printf("您想看哪种集合操作的演示呢？\n\n");
                    printf("====================================\n");
                    printf("0. 回到上一级\n");
                    printf("1. 并\n");
                    printf("2. 交\n");
                    printf("3. 差\n");
                    printf("====================================\n\n");
                    printf("请给出您的选择：");
                    scanf("%d", &select);

                    system("cls");
                    clear_Buff_IO_Count();
                    if (select == 0)
                        break;
                    else if (select == 1) {
                        printf("查看R∪S的结果：\n");
                        tablesUnion(table_R, table_S, setOperationTable);
                    } else if (select == 2) {
                        printf("查看R∩S的结果：\n");
                        tablesIntersect(table_R, table_S, setOperationTable);
                    } else if (select == 3) {
                        char diffedTable, diffTable;
                        table_t table1, table2;
                        printf("您想做哪个表对哪个表的差呢？(输入R S或S R)\n");
                        scanf("%c%*c%c", &diffedTable, &diffTable);
                        if (diffedTable = 'R' && diffedTable == 'S') {
                            printf("查看R-S的结果：\n");
                            table1 = table_R, table2 = table_S;
                        } else if (diffedTable = 'S' && diffedTable == 'R') {
                            printf("查看S-R的结果：\n");
                            table1 = table_S, table2 = table_R;
                        } else {
                            printf("\"R S\"以及\"S R\"以外的输入都是不允许的哦~\n");
                            system("pause");
                            continue;
                        }
                        tablesDiff(table1, table2, setOperationTable);
                    } else {
                        printf("不可以做出0-3以外的选择哦~\n");
                        system("pause");
                        continue;
                    }
                    showResult(joinTable.start);
                    print_IO_Info(joinTable);
            } default: {
                printf("不可以做出0-4以外的选择哦~\n");
                system("pause");
                break;
            }
        }
    } while(select != 0);

    system("pause");
    return OK;
}