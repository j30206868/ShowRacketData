#ifndef COMMONFUNC_H
#define COMMONFUNC_H

#include <math.h>
#include <string>

struct intArray{
    int *values;
    int length;
};
struct dualIntArray{
    intArray A;
    intArray B;
};

class cwz_timer{
public:
    static void start();
    static void t_start();
    static double stop();
    static double t_stop();
    static void time_display(char *disp,int nr_frame=1);
    static void t_time_display(char *disp,int nr_frame=1);
private:
    static double m_pc_frequency;
    static __int64 m_counter_start;
    static double t_pc_frequency;
    static __int64 t_counter_start;
};

//IntArray處理
void cleanIntA(intArray &data);
intArray copyIntA(intArray data);

//dualIntArray處理
void cleanDualIntArray(dualIntArray &data);

//陣列處理
int **allcIntDArray(int rowN, int colN);
void freeIntDArray(int **dArr, int rowN);

//數學運算
int getRound(double num);
bool isEqualSign(int A, int B);
int getSign(int v);

//檔案處理
bool cleanFile(std::string fname);

intArray splitAsInt(std::string str, std::string delimiter);

#endif // COMMONFUNC_H

