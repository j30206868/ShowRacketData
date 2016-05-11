#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mpu6050reader.h"
#include "racketdataanalyzer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void toggleMPU6050Reading();
    void mpu6050StartReading();
    void mpu6050StopReading();

    void changeCurrentBadData(DrawingInfo *data);

    void on_record_btn_clicked();

    void on_type_box_currentIndexChanged(int index);

    void on_person_text_edit_textChanged();

    void on_file_count_text_edit_textChanged();
    void on_read_file_btn_clicked();

private:
    Ui::MainWindow *ui;
    MpuReader *mpuReader;
    RacketDataAnalyzer *rdAnalyzer;
    DrawingInfo *drawInfo;
};

#endif // MAINWINDOW_H
