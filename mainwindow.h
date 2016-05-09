#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mpu6050reader.h"

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
    void on_move_btn_clicked();
    void toggleMPU6050Reading();

private:
    Ui::MainWindow *ui;
    bool isReadingMPU6050;
    MpuReader *mpuReader;
};

#endif // MAINWINDOW_H
