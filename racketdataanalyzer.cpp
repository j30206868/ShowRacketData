#include "racketdataanalyzer.h"
#include "commonfunc.h"
#include <iostream>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include "windows.h"
#include "mpu6050reader.h"

inline void baddataStrToStruct(QString str, BadData *data){
    QStringList list = str.split(",");

    data->accl[0] = list.at(0).toInt();
    data->accl[1] = list.at(1).toInt();
    data->accl[2] = list.at(2).toInt();

    data->gyro[0] = list.at(3).toInt();
    data->gyro[1] = list.at(4).toInt();
    data->gyro[2] = list.at(5).toInt();

    data->quaternion[0] = list.at(6).toFloat();
    data->quaternion[1] = list.at(7).toFloat();
    data->quaternion[2] = list.at(8).toFloat();
    data->quaternion[3] = list.at(9).toFloat();

    data->ms      = list.at(10).toInt();
}
void readRacketRawDataFromFile(QString fileName, BadData *dataBuf, int &bufIdx){
    bufIdx = 0;

    QFile inputFile(fileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          baddataStrToStruct(line, &dataBuf[bufIdx++]);
       }
       inputFile.close();
    }
}


RacketDataAnalyzer::RacketDataAnalyzer(QObject *parent, QComboBox *_fpathBox)
{
    dataBuf = new BadData[bufSize];
    drawInfo = new DrawingInfo();
    fpathBox = _fpathBox;
    stopFlag = true;
    currentIdx = 0;
}

RacketDataAnalyzer::~RacketDataAnalyzer()
{
    stopFlag = true;
    delete[] dataBuf;
    wait();//使執行緒在Stop被改為true時 會等到run那邊的迴圈結束才把執行緒destory掉
           //避免不可預期之錯誤發生
}

void RacketDataAnalyzer::stop()
{
    stopFlag = true;
}

void RacketDataAnalyzer::readNewFile(QString fname)
{
    this->stop();
    std::cout << "Read File From: " << this->fpathBox->currentData().toString().toStdString() << std::endl;
    readRacketRawDataFromFile(this->fpathBox->currentData().toString(), this->dataBuf, this->dataAmt);
    std::cout << "Data Count: " << this->dataAmt << std::endl;
    emit updateDataIndexRange(0, this->dataAmt-1);
    emit updateDataIndex(0);
    if(stopFlag){//如果threa停了, 就重新啟動
        this->start();
    }
}

void RacketDataAnalyzer::changeDataIndex(int idx)
{
    this->currentIdx = idx;
    drawInfo->rawBaddata = &dataBuf[idx];
    if(currentIdx >= 50 && currentIdx <= 100){
        drawInfo->shouldRackSurfRed = true;
    }else{
        drawInfo->shouldRackSurfRed = false;
    }
    emit updateBadData(drawInfo);
}

static bool surfaceRed = false;
void RacketDataAnalyzer::run()
{
    stopFlag = false;

    emit updateDataIndexRange(1600, 1700);
    float gravity[3];
    int accl[3];
    float accl_in_ms[3];
    float velocity[3];
    velocity[0] = 0;
    velocity[1] = 0;
    velocity[2] = 0;
    Vector3f displace = Vector3f(0,0,0);
    while(currentIdx < 1700 && !stopFlag){

        QtoGravity(dataBuf[currentIdx].quaternion, gravity);
        accl[0] = dataBuf[currentIdx].accl[0];
        accl[1] = dataBuf[currentIdx].accl[1];
        accl[2] = dataBuf[currentIdx].accl[2];
        removeGravity(accl, gravity);
        //1 g = 9.80665 m/s2
        //convert accl from g to m/s2
        accl_in_ms[0] = (float)accl[0] / 2048;
        accl_in_ms[1] = (float)accl[1] / 2048;
        accl_in_ms[2] = (float)accl[2] / 2048;

        //velocity[0] = accl_in_ms[0] * 0.00980665 * dataBuf[currentIdx].ms;
        //velocity[1] = accl_in_ms[1] * 0.00980665 * dataBuf[currentIdx].ms;
        //velocity[2] = accl_in_ms[2] * 0.00980665 * dataBuf[currentIdx].ms;
        velocity[0] = accl_in_ms[0] * 0.00980665;
        velocity[1] = accl_in_ms[1] * 0.00980665;
        velocity[2] = accl_in_ms[2] * 0.00980665;

        SHORT leftKeyState  = GetAsyncKeyState( VK_LEFT );
        SHORT rightKeyState = GetAsyncKeyState( VK_RIGHT );
        if( ( 1 << 16 ) & leftKeyState ){
            displace.x -= velocity[0];
            displace.y -= velocity[1];
            displace.z -= velocity[2];

            drawInfo->displace = displace;

            std::cout << "velocity: " << velocity[0] << "," << velocity[1] << "," << velocity[2] << std::endl;
            std::cout << "displace: " << displace.x << "," << displace.y << "," << displace.z << std::endl;
            emit updateDataIndex(currentIdx-1);
        }
        if( ( 1 << 16 ) & rightKeyState ){
            displace.x += velocity[0];
            displace.y += velocity[1];
            displace.z += velocity[2];

            drawInfo->displace = displace;

            std::cout << "velocity: " << velocity[0] << "," << velocity[1] << "," << velocity[2] << std::endl;
            std::cout << "displace: " << displace.x << "," << displace.y << "," << displace.z << std::endl;
            emit updateDataIndex(currentIdx+1);
        }
        Sleep(10);
    }

    while(!stopFlag){
        SHORT leftKeyState  = GetAsyncKeyState( VK_LEFT );
        SHORT rightKeyState = GetAsyncKeyState( VK_RIGHT );

        bool keyPressed = false;
        if( ( 1 << 16 ) & leftKeyState ){
            emit updateDataIndex(currentIdx-1);
            keyPressed = true;
        }
        if( ( 1 << 16 ) & rightKeyState ){
            emit updateDataIndex(currentIdx+1);
            keyPressed = true;
        }
        if(keyPressed){

        }

        Sleep(10);
    }
}
