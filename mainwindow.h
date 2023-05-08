#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int* runScalable(int n, int *array, int threadCount);
    int* runNonScalable(int n, int *array);

public slots:
    void onCheckboxClicked(int state);
    void onButtonClicked();

};
#endif // MAINWINDOW_H
