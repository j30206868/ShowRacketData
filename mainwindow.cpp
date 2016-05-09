#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->readDataBtn, SIGNAL(clicked()),
            this, SLOT(toggleMPU6050Reading()));

    mpuReader = new MpuReader(this);
    mpuReader->setGlWidget(ui->widget);
    isReadingMPU6050 = false;
    //mpuReader->setNextSymCount(nextSampleCount);
    toggleMPU6050Reading();
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
        mpuReader->startReading();
        ui->readDataBtn->setText(tr("停止讀取"));
        ui->readDataBtn->setFixedWidth(100);
    }else{
        mpuReader->stopReading();
        ui->readDataBtn->setText(tr("讀取MPU6050數據"));
        ui->readDataBtn->setFixedWidth(150);
    }
}

void MainWindow::on_move_btn_clicked()
{
    ui->widget->updateGL();
}
