#ifndef RACKETDATAANALYZER
#define RACKETDATAANALYZER

#include <QThread>
#include <QComboBox>

const int BadDataTotalElement = 11;
struct BadData{
    int accl[3];
    int gyro[3];
    float quaternion[4];
    int ms;//period
};

class RacketDataAnalyzer : public QThread{
    Q_OBJECT
public:
    RacketDataAnalyzer(QObject *parent = 0, QComboBox *_fpathBox = 0);
    ~RacketDataAnalyzer();
    void stop();
signals:
    updateDataIndexRange(int,int);
    updateDataIndex(int);
    updateBadData(BadData *data);
public slots:
    void readNewFile(QString fname);
    void changeDataIndex(int);
protected:
    void run() Q_DECL_OVERRIDE;
private:
    QComboBox *fpathBox;
    const int bufSize = 100000;
    int dataAmt;
    int currentIdx;
    BadData *dataBuf;
    bool stopFlag;
};

#endif // RACKETDATAANALYZER

