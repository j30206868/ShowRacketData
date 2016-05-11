#include "racketdataanalyzer.h"
#include "commonfunc.h"
#include <iostream>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include "windows.h"

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
    fpathBox = _fpathBox;
    stopFlag = true;
    currentIdx = 0;
}

RacketDataAnalyzer::~RacketDataAnalyzer()
{
    stopFlag = true;
    delete[] dataBuf;
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
    emit updateBadData(&dataBuf[idx]);
}

static bool surfaceRed = false;
void RacketDataAnalyzer::run()
{
    stopFlag = false;
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
