#include <ctime>
#include <cmath>
#include <numeric>
#include <vector>
#include "utils.cpp"


// A, B, C, D�ĸ��ֶε�ȡֵ��Χ
const int A_LOW = 1,  A_HIGH = 40;
const int B_LOW = 1,  B_HIGH = 1000;
const int C_LOW = 20, C_HIGH = 60;
const int D_LOW = 1,  D_HIGH = 1000;
const int maxOverlaps = 100;
const int threshold = 15;
std::vector<row_t> overlaps;


/**
 * @brief �������һ���������д�뵽������
 * 
 * @param table �����Ϣ��������ʼ��ַ��������ַ�Լ���¼�����ͳ��ȵ�
 * @param range ÿ�����Ե����ɷ�Χȡֵ
 */
void generateANDwrite(table_t table, int range[][2]) {
    Block blk;
    blk.writeInit(table.start);
    row_t r;
    addr_t curAddr = 0;
    for (int i = 0, count = 0; i < table.size;) {
        // ����7����¼������blk��
        for (int j = 0; j < numOfRowInBlk && i < table.size; ++i, ++j) {
            int prob = rand() % 101;
            if (prob < threshold && count <= maxOverlaps) {
                r = overlaps[count++];
            } else {
                r.A = rand() % (range[0][1] - range[0][0]) + range[0][0];
                r.B = rand() % (range[1][1] - range[1][0]) + range[1][0];
            }
            curAddr = blk.writeRow(r);
            if (i == table.size - 1) {
                if (count == 101)
                    count -= 1;
                printf("ʹ�����ظ���¼���м�¼��������%d\n", count);
            }
        }
    }
    curAddr = blk.writeLastBlock();
    table.end = curAddr;
    printf("д����ɣ����д����̿飺%d-%d\n\n", table.start, table.end);
}

/**************************** main ****************************/
int main() {
    bufferInit();
    overlaps.resize(maxOverlaps);
    // �����ظ���¼��
    row_t t;
    srand(time(NULL));
    for (int i = 0; i < maxOverlaps; ++i) {
        t.A = rand() % (A_HIGH - C_LOW) + C_LOW;
        t.B = rand() % (B_HIGH - B_LOW) + B_LOW;
        overlaps.push_back(t);
    }
    // ���ɲ�ͬ��ļ�¼
    srand(time(NULL));
    int R_range[2][2] = {{A_LOW, A_HIGH}, {B_LOW, B_HIGH}};
    int S_range[2][2] = {{C_LOW, C_HIGH}, {D_LOW, D_HIGH}};
    printf("\n=================== ����R��ļ�¼ ===================\n");
    generateANDwrite(table_R, R_range);
    printf("\n=================== ����S��ļ�¼ ===================\n");
    generateANDwrite(table_S, S_range);
    system("pause");
    return OK;
}
