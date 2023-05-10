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
    int* runScalable(int arraySize, int threadCount, int *input);
    int* runNonScalable(int arraySize, int *input);

public slots:
    void onCheckboxClicked(int state);
    void onButtonClicked();

};
#endif // MAINWINDOW_H
