#include "commonfunc.h"
#include <QString>
#include <fstream>

#include <iostream>

#ifdef __MINGW32__
    #include <windows.h>
#elif _WIN32
    #define _AFXDLL
    #include <afxwin.h>
#endif

double  cwz_timer::m_pc_frequency = 0;
__int64 cwz_timer::m_counter_start = 0;
double  cwz_timer::t_pc_frequency = 0;
__int64 cwz_timer::t_counter_start = 0;

void cwz_timer::start()
{
    //m_begin=clock();

    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
        std::cout << "QueryPerformanceFrequency failed!\n";

    m_pc_frequency = double(li.QuadPart);///1000.0;

    QueryPerformanceCounter(&li);
    m_counter_start = li.QuadPart;
}
void cwz_timer::t_start()
{
    //m_begin=clock();

    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
        std::cout << "QueryPerformanceFrequency failed!\n";

    t_pc_frequency = double(li.QuadPart);///1000.0;

    QueryPerformanceCounter(&li);
    t_counter_start = li.QuadPart;
}
double cwz_timer::t_stop(){
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-t_counter_start)/t_pc_frequency;
}
double cwz_timer::stop()
{
    //m_end=clock(); return ( double(m_end-m_begin)/CLOCKS_PER_SEC );
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-m_counter_start)/m_pc_frequency;
}
void cwz_timer::time_display(char *disp,int nr_frame){
    printf("Running time (%s) is: %5.5f Seconds.",disp,stop()/nr_frame);
    //std::cout << "Running time (" << disp << ") is: " << stop()/nr_frame << "Seconds." << std::endl;
    std::cout << std::endl;
}
void cwz_timer::t_time_display(char *disp,int nr_frame){ printf("Running time (%s) is: %5.5f Seconds.\n",disp,t_stop()/nr_frame);}


//IntArray處理
void cleanIntA(intArray &data){
    if(data.values != NULL){
        if(data.length > 0){
            delete[] data.values;
            data.length = 0;
        }
    }
}
intArray copyIntA(intArray data){
    intArray newData;
    newData.length = data.length;
    newData.values = new int[data.length];
    for(int i=0; i<data.length; i++){
        newData.values[i] = data.values[i];
    }
    return newData;
}

//dualIntArray處理
void cleanDualIntArray(dualIntArray &data){
    cleanIntA(data.A);
    cleanIntA(data.B);
}

//陣列處理
int **allcIntDArray(int rowN, int colN){
    int **newIntDArr = new int*[rowN];
    for(int i=0; i<rowN ; i++)
        newIntDArr[i] = new int[colN];

    return newIntDArr;
}
void freeIntDArray(int **dArr, int rowN){
    for(int i=0; i<rowN ; i++)
        delete[] dArr[i];
    delete[] dArr;
}

//數學運算
int getRound(double num){
    int sign = 0;
    int result = 0;

    //記錄正負
    if(num > 0){
        sign = 1;
    }else if(num < 0){
        sign = -1;
    }else{
        return 0;
    }

    //全部以正數處理
    num = abs(num);
    int integerpart = floor(num);

    double left = num - integerpart;

    if(left >=0.5){
        result = ceil(num);
    }else if( left < 0.5){
        result = floor(num);
    }

    return result * sign;
}
bool isEqualSign(int A, int B){
    if(A >= 0 && B >= 0){
        return true;
    }else if( A <= 0 && B <= 0){
        return true;
    }else{
        return false;
    }
}
int getSign(int v){
    if(v == 0){
        return 0;
    }else if(v > 0){
        return 1;
    }else{
        return -1;
    }
}

//檔案處理
bool cleanFile(std::string fname){

    //clean the file
    std::ofstream myfile (fname.c_str());
    myfile << "";
    myfile.close();

    return true;
}

//String處理
intArray splitAsInt(std::string str, std::string delimiter){
    int buffer[10];
    int idx = 0;

    int posi=0;
    std::string tmp = "";
    while( (posi = str.find(delimiter)) != (signed)std::string::npos )
    {
        tmp = str.substr(0, posi);
        //cout <<" " << tmp << " ";
        buffer[idx] = QString::fromUtf8(tmp.c_str()).toInt();
        //cout <<" " << result[idx] << " ";
        str.erase(0, posi + delimiter.length());
        idx++;
    }

    tmp = str.substr(0, str.length());
    buffer[idx] = QString::fromUtf8(tmp.c_str()).toInt();
    //cout <<" "<< result[idx] << " ";

    intArray result;
    result.length = (idx+1);
    result.values = new int[result.length];
    for(int i=0 ; i<result.length ;i++)
    {
        result.values[i] = buffer[i];
    }

    return result;
}
