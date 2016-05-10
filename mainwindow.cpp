#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mpuReader = new MpuReader(this);
    ui->setupUi(this);

    ui->type_box->addItem("平球", 1);
    ui->type_box->addItem("小球", 2);
    ui->type_box->addItem("挑球", 3);
    ui->type_box->addItem("長球", 4);
    ui->type_box->addItem("殺球", 5);
    ui->type_box->addItem("切球", 6);
    ui->type_box->addItem("撲球", 7);
    //ui->person_text_edit->setPlainText(QString("wu"));

    connect(ui->readDataBtn, SIGNAL(clicked()),
            this, SLOT(toggleMPU6050Reading()));


    mpuReader->setPerson_string(ui->person_text_edit->toPlainText().toStdString());
    this->mpuReader->setRacket_file_count(ui->file_count_text_edit->toPlainText().toInt());
    mpuReader->setGlWidget(ui->widget);
    isReadingMPU6050 = false;

    ui->record_btn->setEnabled(false);
    ui->type_box->setEnabled(false);
    ui->person_text_edit->setEnabled(false);
    ui->file_count_text_edit->setEnabled(false);

    //mpuReader->setNextSymCount(nextSampleCount);
    //toggleMPU6050Reading();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleMPU6050Reading(){
    isReadingMPU6050 = !isReadingMPU6050;
    this->setProperty("isReadingMPU6050", isReadingMPU6050);
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
    if(isReadingMPU6050){
        //ui->readDataBtn->setText(tr("連接藍芽中..."));
        mpuReader->startReading();
        ui->readDataBtn->setText(tr("停止讀取"));
        ui->readDataBtn->setFixedWidth(100);

        ui->record_btn->setEnabled(true);
        ui->type_box->setEnabled(true);
        ui->person_text_edit->setEnabled(true);
        ui->file_count_text_edit->setEnabled(true);
    }else{
        mpuReader->stopReading();
        ui->readDataBtn->setText(tr("讀取MPU6050數據"));
        ui->readDataBtn->setFixedWidth(150);

        ui->record_btn->setEnabled(false);
        ui->type_box->setEnabled(false);
        ui->person_text_edit->setEnabled(false);
        ui->file_count_text_edit->setEnabled(false);
    }
}

void MainWindow::on_record_btn_clicked()
{
    if( mpuReader->getWriteRawToFile() ){
        mpuReader->setWriteRawToFile(false);
        ui->record_btn->setText("寫入檔案");
        ui->type_box->setEnabled(true);
        ui->person_text_edit->setEnabled(true);
        ui->file_count_text_edit->setEnabled(true);
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
