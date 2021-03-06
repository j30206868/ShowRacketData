#include "mpu6050reader.h"
//#include "symrecorder.h"
#include "commonfunc.h"
//#include "mousectrl.h"
#include <iostream>
#include <QString>
#include <fstream>
#include <sstream>
#include <ctime>

///
//#define USE_WIRE_CONNECTION
static const char *racketFilePath = "Racket/";
static std::string date_string = "";
//static const char *type_string = "5";
//static const char *person_string = "wu";
//static int racket_file_count = 2;
static std::stringstream fileNameSStream;
///

MpuReader::MpuReader(QObject *parent)
: QThread(parent)
{
    stopFlag = true;
    //get current date
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::stringstream datesstm;
    if( (now->tm_mon + 1) < 10)
        datesstm << "0" << (now->tm_mon + 1);
    else
        datesstm << (now->tm_mon + 1);
    datesstm <<  now->tm_mday;
    date_string = datesstm.str();

    racket_file_count = 1;

    writeRawToFile = false;
}

MpuReader::~MpuReader()
{
    stopFlag = true;
    wait();//使執行緒在Stop被改為true時 會等到run那邊的迴圈結束才把執行緒destory掉
           //避免不可預期之錯誤發生
}

void MpuReader::setGlWidget(glwidget *widg){
    this->my_glwidget = widg;
}

void MpuReader::run()
{
    //連線藍芽
    #ifndef USE_WIRE_CONNECTION
        blue_connector = new cwz_c_blue();
        int result = blue_connector->connect(this->stopFlag);
        if(result == 1)//如果成功連線
            emit connectionCreated();
    #endif

    //********************************************************//
    //							變數宣告
    //********************************************************//
    //Handle Serial Buffer
    const int incomming_buf_len = 2048;
    char incomingData[incomming_buf_len] = "";			// don't forget to pre-allocate memory
    int dataLength = 256;
    const int bSize = 10000;
    char buffer[bSize] = "";
    int bLen = 0;

    //Decode information from buffer string
    int count = 0;
    int flag = -1; //-1等於上次是不合法結束的情況
    int accl[3]; // x, y, z
    int gyro[3]; // x, y, z
    float gravity[3];
    float quatern[4];
    int buttons[2];
    int period;

    //中位值濾波
    int MFLen = 5;   //濾波器長度為5
    int **Accls = allcIntDArray(MFLen, 3);
    int **Gyros = allcIntDArray(MFLen, 3);
    int MFCount = 0; // 只用於前五次, 確認filter的data set是不是被裝滿了, 裝滿了才開始濾波

    //Symbol record
    //SymbolRecorder SR = SymbolRecorder();

    //fileNameSStream << racketFilePath << date_string << "_" << type_string << "_" << person_string << "_" << racket_file_count << ".txt";

    //********************************************************//
    //					連接Com port並確認連線
    //********************************************************//
    #ifdef USE_WIRE_CONNECTION
        std::cout << "Arduino with GY521 board and chip MPU6050!" << std::endl;
        char *COM_NUM = "COM5";
        Serial* SP = new Serial(COM_NUM);    // adjust as needed
        if (SP->IsConnected()){
            emit connectionCreated();
            std::cout << "We're connected" << std::endl;
        }else{
            stopFlag = true;
        }
        //read few set of data until it's getting stable
        waitUntilSerialStable(SP, incomingData, dataLength);

        std::cout << "Data should be stable, start to read MPU6050" << std::endl;
    #else
        blue_connector->send("Start to read mpu6050 data from bluetooth.\n");
    #endif

    while(!stopFlag){
        #ifdef USE_WIRE_CONNECTION
            if(!SP->IsConnected()){
                std::cout << "Failed to read " << COM_NUM << " Mpu6050 reader is off." << std::endl;
                emit readingEnded();
                break;
            }
            if( !readSerialIntoBuffer(SP, incomingData, dataLength, readResult, bLen, bSize, buffer) )
                continue;//If no valid serial data are read or buffer overflow, skip this round
        #else
            dataLength = blue_connector->receive(incomingData, incomming_buf_len);
            if(dataLength > 0){//有收到東西
                incomingData[dataLength] = '\0';
                strcpy_s(&buffer[bLen], bSize, incomingData);
                bLen += dataLength;
            }
        #endif
        //decode mpu6050 data from buffer str, take off processed part from buffer
        strcpy_s(buffer, bSize, getAccelAndGyro(&count, &flag, &bLen, buffer, accl, gyro, quatern, buttons, &period, bSize));

        //   每一組合法數值為 (acclX,acclY,acclZ,gyroX,gyroY,gyroZ,quatern1,quatern2,quatern3,quatern4,buttonRight,buttonLeft,readperiod)
        if( flag != GETAANDG_NEW_VALID_DATA )
            continue;//尚未讀到整組完整資料 不處理 繼續讀

        SHORT downKeyState = GetAsyncKeyState( VK_DOWN );
        //std::cout << "gravity: " << gravity[0] << ", " << gravity[1] << ", " << gravity[2] << std::endl;
        if( writeRawToFile )
        {//按ctrl鍵
            std::cout << "period: " << period << std::endl;

            if( ( 1 << 16 ) & downKeyState ){
                //writeAcclAndGyroAndGravity(fileNameSStream.str().c_str(), accl, gyro, gravity, 1);
                writeMpu6050RawToFile(fileNameSStream.str().c_str(), accl, gyro, quatern, period, 1);
                //std::cout << "Down Key State: " << 1 << std::endl;
            }else{
                //writeAcclAndGyroAndGravity(fileNameSStream.str().c_str(), accl, gyro, gravity, 0);
                writeMpu6050RawToFile(fileNameSStream.str().c_str(), accl, gyro, quatern, period, 0);
                //std::cout << "Down Key State: " << 0 << std::endl;
            }
        }

        //********************************************************//
        //				處理加速度計與陀螺儀Raw Data
        //********************************************************//
        //get gravity
        QtoGravity(quatern, gravity);
        //std::cout << "Gyro: " << gyro[0] << "," << gyro[1] << "," << gyro[2] << std::endl;
        //去重力
        removeGravity(accl, gravity);

        double total_q = sqrt(gravity[0]*gravity[0] + gravity[1]*gravity[1] + gravity[2]*gravity[2]);

        this->my_glwidget->setNewZ(gravity[0], gravity[1], gravity[2]);

        //check if gravity value is invalid
        if( total_q >= 1.1 ){
            std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            std::cout << "total_q: " << total_q << std::endl;
            std::cout << "Wrong gravity ............................................" << std::endl;
            std::cout << "quatern: " << quatern[0] << "," << quatern[1] << "," << quatern[2] << "," << quatern[3] << std::endl;
            std::cout << "gravity: " << gravity[0] << "," << gravity[1] << "," << gravity[2] << std::endl;
            std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            system("PAUSE");

        }

        //Median Filter(中位值濾波)
        agMedianFilter(Accls, Gyros, accl, gyro, MFCount, MFLen, true, true); //是否做濾波 accl=true gyro=true
    }
#ifdef USE_WIRE_CONNECTION
    SP->~Serial();
#else
    blue_connector->disconnect();
#endif
    //釋放中位值濾波使用的記憶體空間
    freeIntDArray(Accls, MFLen);
    freeIntDArray(Gyros, MFLen);
}
std::string MpuReader::getType_string() const
{
    return type_string;
}

void MpuReader::setType_string(std::string value)
{
    type_string = value;
}

std::string MpuReader::getPerson_string() const
{
    return person_string;
}

void MpuReader::setPerson_string(std::string value)
{
    person_string = value;
}

int MpuReader::getRacket_file_count() const
{
    return racket_file_count;
}

void MpuReader::setRacket_file_count(int value)
{
    racket_file_count = value;
}

bool MpuReader::getWriteRawToFile() const
{
    return writeRawToFile;
}

void MpuReader::setWriteRawToFile(bool value)
{
    if(value){
        fileNameSStream.str("");
        fileNameSStream << racketFilePath << date_string << "_" << type_string << "_" << person_string << "_" << racket_file_count << ".txt";
        cleanFile(fileNameSStream.str());
        std::cout << std::endl << std::endl << std::endl << std::endl << std::endl;
        std::cout << std::endl << std::endl << std::endl << std::endl << std::endl;
        std::cout << "====> Start New File: " << fileNameSStream.str() << std::endl;
        std::cout << std::endl << std::endl << std::endl << std::endl << std::endl;
        std::cout << std::endl << std::endl << std::endl << std::endl << std::endl;
    }else{
        racket_file_count++;
    }

    writeRawToFile = value;
}


void MpuReader::stopReading(){
    stopFlag = true;
}
void MpuReader::startReading(){
    stopFlag = false;
    this->start();
}
bool MpuReader::isReading(){
    if(stopFlag){
        return false;
    }else{
        return true;
    }
}


//********************************************************//
//														  //
//			加速度計與陀螺儀Raw Data處理
//														  //
//********************************************************//
//flag = -1 不合法結束
//flag = 0 合法結束 新一組數值出現
//flag = 1 正在讀取
//極限了 再多讀三個數字就會炸掉 原因無從得知
char* getAccelAndGyro(int *count, int *flag, int *bLen, char *buffer, int *accl, int *gyro, float *quatern, int *button, int *time, const int bSize){
    while(true){
        int value = 0;
        float dvalue = 0.0;
        int result = 0;

        if(*count < 6)
        {//accl and gyro are integer
            strcpy_s(buffer, bSize, readValue(&value, bLen, buffer, &result, count));
        }else if(*count >= 6 && *count <= 9)
        {//gravity and quaternion are double
            strcpy_s(buffer, bSize, readValue(&dvalue, bLen, buffer, &result, count));
        }else
        {//time is integer
            strcpy_s(buffer, bSize, readValue(&value, bLen, buffer, &result, count));
        }

        if( result > 0 ){ //有value讀出
            if(result == 2){
                // end with ','
                //assign value
                if(*count < 3){//accl(int)
                    int idx = (*count);
                    accl[idx] = value;
                }else if(*count >= 3 && *count<=5){//gyro(int)
                    int idx = (*count-3);
                    gyro[idx] = value;
                }else if(*count >= 6 && *count <= 9){//Quaternion(double)
                    int idx = (*count-6);
                    quatern[idx] = dvalue;
                }else if(*count >= 10 && *count <= 11){
                    int idx = (*count-10);
                    button[idx] = value;
                }

                *flag = 1;
                *count = *count + 1;
//				printf("in count = %d\n", *count);
            }else if(result == 1){
                //end of a round
                if(*count == 12){
                    //合法結束
                    *time = value;
                    //quatern[3] = 10.0;
                    *flag = 0;
                    *count = 0;
                    return buffer;
                }else{
                    //不合法結束
                    *flag = -1;
                    *count = 0;
                    printf("getAccelAndGyro: abnormal end of data, abort.\n");
                }
            }

        }else if( result < 0){ //沒有value讀出
            //不合法情況出現
            if( result == -1 ){
                //不合法結束
                *flag = -1;
                *count = 0;
            }
        }else if(result == 0){
            return buffer;
        }
    }

    return buffer;
}
//if value assigned
//if stop at | return 1 else if stop at , return 2
//else if value not assign
//'|' return -1, ',' return -2
//if no end sign or number return 0;
char* readValue(float *value, int *bLen, char *buffer, int *result, int *countFlag){
    std::string tmp = "";
    int count = 0;
    int total = *bLen;

    // nothing
    if(total == 0){
        *result = 0;
        return buffer;
    }

    //read first
    total--;

    int id = 0;
    char c = buffer[id];
    do{
        if(c == '|'){
            *result = 1;
            break;
        }else if(c == ','){
            *result = 2;
            break;
        }else{
            //
            tmp = tmp + c;
            count++;
        }

        //read next
        total--;
        if(total <= 0){
            //no end sign, cancel reading
            *result = 0;
            return buffer;
        }

        id++;
        c = buffer[id];
    }while(true);

    if(count > 0){
        bool validate;
        *value = QString::fromStdString(tmp).toFloat(&validate);
        if(validate == false){
            std::cout<<"tmp = "<<tmp<< std::endl;
            *result = 1;
            *countFlag = 0;
            std::cout << "Exception caugth!" << std::endl;
        }
        //cut buffer and deduct bLen
        *bLen-= (id+1);

        //printf("in buffer = %.*s, bLen = %d, id+1 = %d\n", *bLen, buffer, *bLen, (id+1));

        return &buffer[id+1];
    }else if(count == 0){

        //cut buffer and deduct bLen
        *bLen-= (id+1);

        *result *= -1;
        return &buffer[id+1];
    }else{
        *result = 0;
        //error
        return buffer;
    }
}
char* readValue(int *value, int *bLen, char *buffer, int *result, int *countFlag){
    std::string tmp = "";
    int count = 0;
    int total = *bLen;

    // nothing
    if(total == 0){
        *result = 0;
        return buffer;
    }

    //read first
    total--;

    int id = 0;
    char c = buffer[id];
    do{
        if(c == '|'){
            *result = 1;
            break;
        }else if(c == ','){
            *result = 2;
            break;
        }else{
            //
            tmp = tmp + c;
            count++;
        }

        //read next
        total--;
        if(total <= 0){
            //no end sign, cancel reading
            *result = 0;
            return buffer;
        }

        id++;
        c = buffer[id];
    }while(true);

    if(count > 0){
        bool validate;
        *value = QString::fromStdString(tmp).toInt(&validate);
        if(validate == false){
            std::cout<<"tmp = "<<tmp<<std::endl;
            *result = 1;
            *countFlag = 0;
            std::cout << "Exception caugth!" << std::endl;
        }
        //cut buffer and deduct bLen
        *bLen-= (id+1);

        //printf("in buffer = %.*s, bLen = %d, id+1 = %d\n", *bLen, buffer, *bLen, (id+1));

        return &buffer[id+1];
    }else if(count == 0){

        //cut buffer and deduct bLen
        *bLen-= (id+1);

        *result *= -1;
        return &buffer[id+1];
    }else{
        *result = 0;
        //error
        return buffer;
    }
}
void QtoGravity(float *quatern, float *gravity){
    int w=0, x=1, y=2, z=3;
    gravity[0] = 2 * (quatern[x] * quatern[z] - quatern[w] * quatern[y]);
    gravity[1] = 2 * (quatern[w] * quatern[x] + quatern[y] * quatern[z]);
    gravity[2] = quatern[w] * quatern[w] - quatern[x] * quatern[x] - quatern[y] * quatern[y] + quatern[z] * quatern[z];
}
void removeGravity(int *accl, float *gravity){
    accl[0] = accl[0] - (gravity[0]*2048);
    accl[1] = accl[1] - (gravity[1]*2048);
    accl[2] = accl[2] - (gravity[2]*2048);
}
int agMedianFilter(int *Accls[3], int *Gyros[3], int *accl, int *gyro, int &MFCount, int MFLen, bool applyToAccl, bool applyToGyro){
    int MFLastIdx = MFLen - 1;
    double MFMedianLen = MFLen - 2; // 去掉頭尾(max跟min)的長度
    int x=0, y=1, z=2;
    int gMax[3] = {-32769, -32769, -32769}; //normal range from -32768 to 32767
    int gMin[3] = {32768, 32768, 32768};
    int gAll[3] = {0, 0, 0};
    int aMax[3] = {-32769, -32769, -32769}; //normal range from -32768 to 32767
    int aMin[3] = {32768, 32768, 32768};
    int aAll[3] = {0, 0, 0};

    if(MFCount < MFLastIdx){
        //紀錄最初的n-1個數值
        int idx = MFCount;
        Accls[idx][0] = accl[0];
        Accls[idx][1] = accl[1];
        Accls[idx][2] = accl[2];

        Gyros[idx][0] = gyro[0];
        Gyros[idx][1] = gyro[1];
        Gyros[idx][2] = gyro[2];

        MFCount++;
        return false;
    }else{// 0 = x , 1 = y , 2 = z

        //printf("Accl Move = %d : %d : %d | ", axMove, ayMove, azMove);
        //printf("Accl = %d : %d : %d | ", accl[0], accl[1], accl[2]);
        //printf("Accl = %d : %d : %d | ", axMove, ayMove, azMove);
        //printf("Gyro = %d : %d : %d | bLen = %d\n", gyro[0], gyro[1], gyro[2], bLen);

        //防止overflow的值爆衝
        if( (gyro[z] > 1000 || gyro[z] < -1000) &&
            (gyro[y] > 1000 || gyro[y] < -1000) &&
            (gyro[x] > 1000 || gyro[x] < -1000) ){
            //continue;
            //printf("Gyro = %d : %d : %d | bLen = %d\n", gyro[0], gyro[1], gyro[2], bLen);
            //printf("detected \n\n\n\n\n\n\n\n\n");
        }

        //中位值率波 (去頭去尾)
        //新的值都放在最後一個
        Accls[MFLastIdx][x] = accl[x];
        Accls[MFLastIdx][y] = accl[y];
        Accls[MFLastIdx][z] = accl[z];

        Gyros[MFLastIdx][x] = gyro[x];
        Gyros[MFLastIdx][y] = gyro[y];
        Gyros[MFLastIdx][z] = gyro[z];

        //去掉頭尾取平均
        for(int i=0 ; i < MFLen ; i++){
            for(int j=0 ; j<3 ; j++){

                //accl
                if( Accls[i][j] > aMax[j] ){
                    //取最大值
                    aMax[j] = Accls[i][j];
                }
                if( Accls[i][j] < aMin[j] ){
                    //取最小值
                    aMin[j] = Accls[i][j];
                }
                //3軸各自累加
                aAll[j] += Accls[i][j];

                //gyro
                if( Gyros[i][j] > gMax[j] ){
                    //取最大值
                    gMax[j] = Gyros[i][j];
                }
                if( Gyros[i][j] < gMin[j] ){
                    //取最小值
                    gMin[j] = Gyros[i][j];
                }
                //3軸各自累加
                gAll[j] += Gyros[i][j];

                //覆蓋舊的數值
                if(i > 0){
                    //第1個丟棄 第2個變成第1個 ; 第3個變成第2個 以此類推
                    Accls[i-1][j] = Accls[i][j];
                    Gyros[i-1][j] = Gyros[i][j];
                }
            }
        }

        if(applyToAccl){
            accl[x] = (int)( (aAll[x] - aMax[x] - aMin[x]) / MFMedianLen);
            accl[y] = (int)( (aAll[y] - aMax[y] - aMin[y]) / MFMedianLen);
            accl[z] = (int)( (aAll[z] - aMax[z] - aMin[z]) / MFMedianLen);
        }
        if(applyToGyro){
            gyro[x] = (int)( (gAll[x] - gMax[x] - gMin[x]) / MFMedianLen);
            gyro[y] = (int)( (gAll[y] - gMax[y] - gMin[y]) / MFMedianLen);
            gyro[z] = (int)( (gAll[z] - gMax[z] - gMin[z]) / MFMedianLen);
        }
        //中位值率波 (去頭去尾) 結束
        return true;
    }
}


//********************************************************//
//														  //
//			Decode information from mpu6050
//														  //
//********************************************************//
void waitUntilSerialStable(Serial* SP, char *incomingData, int dataLength){
    int pass = 0;

    while(SP->IsConnected())
    {
        SP->ReadData(incomingData,dataLength);

        if(pass < 50){
            //printf("%.*s\n", readResult, incomingData);
            pass++;
        }else{
            break;
        }
    }
}
//return 0 if buffer overflow or no data is read(readResult = -1), else return 1
int readSerialIntoBuffer(Serial* SP, char *incomingData, int &dataLength, int &readResult, int &bLen, int bSize, char *buffer){
    readResult = SP->ReadData(incomingData,dataLength);

    std::string test(incomingData);
    if(readResult != -1){
        char *newData = new char[readResult];
        memcpy(newData, &incomingData[0], readResult);
        //printf("newData = %.*s\n", readResult, newData);

        bLen = bLen + readResult;
        // avoid buffer overflow
        if(bLen <= bSize){
            //printf("Blen = %d\n", bLen);
            //printf("readResult = %d\n", readResult);
            //get new string into buffer
            strncat (buffer, newData, readResult);

            return 1;
        }else{
            printf("Buffer overflow.\n\n\n\n\n\n\n\n\n\n");
            strcpy(buffer,"");
            bLen = 0;
            return 0;
        }
    }else{
        return 0; // result = -1 no data read
    }
}

void writeAcclAndGyroAndGravity(const char *fname, int *accl, int *gyro, float *gravity, int isDownKeyPressed){
    std::ofstream myfile (fname, std::ios::app);
    myfile << accl[0] << "," << accl[1] << "," << accl[2] << ",";
    myfile << gyro[0] << "," << gyro[1] << "," << gyro[2] << ",";
    myfile << gravity[0] << "," << gravity[1] << "," << gravity[2] <<",";
    myfile << isDownKeyPressed << "\n";
    myfile.close();
}

void writeMpu6050RawToFile(const char *fname, int *accl, int *gyro, float *quaternion, int period, int isDownKeyPressed){
    std::ofstream myfile (fname, std::ios::app);
    myfile << accl[0] << "," << accl[1] << "," << accl[2] << ",";
    myfile << gyro[0] << "," << gyro[1] << "," << gyro[2] << ",";
    myfile << quaternion[0] << "," << quaternion[1] << "," << quaternion[2] <<"," << quaternion[3] << ",";
    myfile << period << ",";
    myfile << isDownKeyPressed << "\n";
    myfile.close();
}
