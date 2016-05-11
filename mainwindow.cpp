#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sstream"
#include <iostream>

#include <QDir>

inline void initUiWidgetValueAndState(Ui::MainWindow *ui){
    //選擇記錄球種的Combox
    ui->type_box->addItem("平球", 1);
    ui->type_box->addItem("小球", 2);
    ui->type_box->addItem("挑球", 3);
    ui->type_box->addItem("長球", 4);
    ui->type_box->addItem("殺球", 5);
    ui->type_box->addItem("切球", 6);
    ui->type_box->addItem("撲球", 7);
    //記錄受測人員代號的TextEditor
    ui->person_text_edit->setPlainText(QString("wu"));
    //一開始尚未連線, 寫入檔案選項相關gui disable
    ui->record_btn->setEnabled(false);
    ui->type_box->setEnabled(false);
    ui->person_text_edit->setEnabled(false);
    ui->file_count_text_edit->setEnabled(false);
}
void addSelectBoxItemsByFileName(QComboBox *combox, QString fname){
    QString sampleDirPath = fname;
    QDir sampleDir(sampleDirPath);

    foreach(QString fileName, sampleDir.entryList() )
    {
        if(fileName.length() > 4)
        {//"檔名長度最少5個字元(含副檔名)"
            combox->addItem(fileName, fname+fileName);
        }
    }
}

inline void toggleOffLineFunctionEnable(Ui::MainWindow *ui, RacketDataAnalyzer *rdAnalyzer, bool state){

    ui->read_file_btn->setEnabled(state);
    if(!state){
        ui->read_file_name_box->setEnabled(state);
        ui->read_file_name_box->clear();
        ui->racket_data_idx_slider->setEnabled(state);
        ui->racket_idx_label->setEnabled(state);
        rdAnalyzer->stop();
    }
}
inline void toggleRealTimeGUIEnable(Ui::MainWindow *ui, bool state){
    ui->record_btn->setEnabled(state);
    ui->type_box->setEnabled(state);
    ui->person_text_edit->setEnabled(state);
    ui->file_count_text_edit->setEnabled(state);
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mpuReader  = new MpuReader(this);
    ui->setupUi(this);
    rdAnalyzer = new RacketDataAnalyzer(this, ui->read_file_name_box);

    connect(ui->read_file_name_box, SIGNAL(currentTextChanged(QString)),
            rdAnalyzer, SLOT(readNewFile(QString)));

    connect(ui->readDataBtn, SIGNAL(clicked()),
            this, SLOT(toggleMPU6050Reading()));
    connect(mpuReader, SIGNAL(connectionCreated()),
            this, SLOT(mpu6050StartReading()));

    connect(rdAnalyzer, SIGNAL(updateBadData(DrawingInfo *)),
            this, SLOT(changeCurrentBadData(DrawingInfo *)));

    connect(this->rdAnalyzer, SIGNAL(updateDataIndexRange(int,int)),
            ui->racket_data_idx_slider, SLOT(setRange(int,int)));
    connect(this->rdAnalyzer, SIGNAL(updateDataIndex(int)),
            ui->racket_data_idx_slider, SLOT(setValue(int)));

    connect(ui->racket_data_idx_slider, SIGNAL(valueChanged(int)),
            this->rdAnalyzer, SLOT(changeDataIndex(int)));
    connect(ui->racket_data_idx_slider, SIGNAL(valueChanged(int)),
            ui->racket_idx_label, SLOT(setNum(int)));

    //GUI內容初始化
    initUiWidgetValueAndState(ui);

    //mpureader值初始化(讀取GUI的設定)
    mpuReader->setPerson_string(ui->person_text_edit->toPlainText().toStdString());
    this->mpuReader->setRacket_file_count(ui->file_count_text_edit->toPlainText().toInt());
    mpuReader->setGlWidget(ui->widget);
    //toggleMPU6050Reading();

    //rdAnalyzer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleMPU6050Reading(){
    if( !mpuReader->isReading() ){
        ui->readDataBtn->setText(tr("連接藍芽中..."));
        toggleOffLineFunctionEnable(ui, rdAnalyzer, false);
        mpuReader->startReading();
    }else{
        mpuReader->stopReading();
        this->mpu6050StopReading();
        toggleOffLineFunctionEnable(ui, rdAnalyzer, true);
    }
}

void MainWindow::mpu6050StartReading()
{//真的取得到資料才會被mpuReader的signal觸發
    ui->readDataBtn->setText(tr("停止讀取"));
    ui->readDataBtn->setFixedWidth(100);
    //enable file related GUI
    toggleRealTimeGUIEnable(ui, true);
}

void MainWindow::mpu6050StopReading()
{
    ui->readDataBtn->setText(tr("讀取MPU6050數據"));
    ui->readDataBtn->setFixedWidth(150);
    //disable file related GUI
    toggleRealTimeGUIEnable(ui, false);
}

void MainWindow::changeCurrentBadData(DrawingInfo *dInfo)
{
    drawInfo = dInfo;
    QtoGravity(drawInfo->rawBaddata->quaternion, drawInfo->gravity);
    ui->widget->setNewZ(drawInfo->gravity[0], drawInfo->gravity[1], drawInfo->gravity[2]);
    ui->widget->setShouldSurfaceColorRed(drawInfo->shouldRackSurfRed);
    ui->widget->setDisplace(drawInfo->displace);
}

void MainWindow::on_record_btn_clicked()
{
    if( mpuReader->getWriteRawToFile() ){
        mpuReader->setWriteRawToFile(false);
        ui->record_btn->setText("寫入檔案");
        ui->type_box->setEnabled(true);
        ui->person_text_edit->setEnabled(true);
        ui->file_count_text_edit->setEnabled(true);

        ui->file_count_text_edit->setPlainText( QString::number(mpuReader->getRacket_file_count()) );
    }else{
        mpuReader->setWriteRawToFile(true);
        ui->record_btn->setText("寫入中...");
        ui->type_box->setEnabled(false);
        ui->person_text_edit->setEnabled(false);
        ui->file_count_text_edit->setEnabled(false);
    }
}

void MainWindow::on_type_box_currentIndexChanged(int index)
{
    QString tmp = ui->type_box->currentData().toString();
    this->mpuReader->setType_string( tmp.toStdString() );

    this->mpuReader->setRacket_file_count(1);
    ui->file_count_text_edit->setPlainText( QString::number(mpuReader->getRacket_file_count()) );
}

void MainWindow::on_person_text_edit_textChanged()
{
    this->mpuReader->setPerson_string(ui->person_text_edit->toPlainText().toStdString());
    //std::cout << "placeholderText: "<< ui->person_text_edit->toPlainText().toStdString() << std::endl;
}

void MainWindow::on_file_count_text_edit_textChanged()
{
    this->mpuReader->setRacket_file_count(ui->file_count_text_edit->toPlainText().toInt());
}

void MainWindow::on_read_file_btn_clicked()
{
    //重新讀取能讀的檔案有哪些
    ui->read_file_btn->setEnabled(false);
    addSelectBoxItemsByFileName(ui->read_file_name_box, QString("Racket/"));
    //rdAnalyzer->readNewFile();
    ui->read_file_name_box->setEnabled(true);
    ui->racket_data_idx_slider->setEnabled(true);
    ui->racket_idx_label->setEnabled(true);
}
