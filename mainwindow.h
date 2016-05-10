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
    void toggleMPU6050Reading();

    void on_record_btn_clicked();

    void on_type_box_currentIndexChanged(int index);

    void on_person_text_edit_textChanged();

    void on_file_count_text_edit_textChanged();

private:
    Ui::MainWindow *ui;
    bool isReadingMPU6050;
    MpuReader *mpuReader;
};

#endif // MAINWINDOW_H
