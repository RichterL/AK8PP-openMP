#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <omp.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->checkbox, SIGNAL(stateChanged(int)), this, SLOT(onCheckboxClicked(int)));
    connect(ui->button, SIGNAL(released()), this, SLOT(onButtonClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCheckboxClicked(int state)
{
    qDebug() << "checkbox clicked";
    if (state == Qt::Checked) {
        ui->threadCount->setReadOnly(false);
        ui->arraySize->setMaximum(99);
    } else {
        ui->threadCount->setReadOnly(true);
        ui->threadCount->setValue(32);
        ui->arraySize->setMaximum(32);
    }
}

void MainWindow::onButtonClicked()
{
    int arraySize = ui->arraySize->value();
    int array[arraySize];
    for (int i = 0; i < arraySize; ++i) {
        array[i] = i + 1;
    }
    int *result;
    if (ui->checkbox->isChecked()) {
        int threadCount = ui->threadCount->value();
        result = runScalable(arraySize, array, threadCount);
    } else {
        result = runNonScalable(arraySize, array);
    }

    QString text;
    for (int i = 0; i < arraySize; ++i) {
        text.append(QString::number(result[i])).append(", ");
    }
    ui->textEdit->setText(text);
}

int* MainWindow::runScalable(int n, int *m, int threadCount)
{
    int q = n / threadCount;
    int x[threadCount][q];
    int z[q];

    // prepare the array

    // divide array into q chunks

    // store chunks in x

    for (int i = 0; i < threadCount; ++i) {
        // local prefix sum
        int sum = 0;
        for (int j = 0; j < q; ++j) {
            sum += x[i][j];
        }
        z[i] = sum;
    }

    int s[threadCount];
    for (int i = 1; i < threadCount; ++i) {
        int s[i][q];
        for (int j = 0; j < q - 1; ++j) {
            s[i][j] = s[i][j] + z[i - 1];
        }
    }

    return m;
}

int* MainWindow::runNonScalable(int n, int *m)
{   
    // allow nested parallelism
    omp_set_nested(1);
    #pragma omp parallel num_threads(n)
    {
        // every thread creates its own copy of memory
        int y[n];
        for (int i = 0; i < n; ++i) {
            y[i] = m[i];
        }

        // only one thread will execute the outter loop
        #pragma omp single
        {
            // only ceil(log2(n)) iterations are needed
            for (int j = 0; j < ceil(log2(n)); ++j) {
                qDebug() << "SINGLE - CPU " << omp_get_thread_num() << " j = " << j;

                // create a shared variable to lower costs
                int index = pow(2,j);

                // (n - 2^j) threads will run the nested block in parallel again
                #pragma omp parallel num_threads(n - index)
                {
                    // declare loop as cooperative
                    #pragma omp for
                    for (int i = index; i < n; ++i) {
                        y[i] = y[i] + m[i - index];
                        qDebug() << "PARALLEL - CPU " << omp_get_thread_num() << " j = " << j << ", i = " << i << ", y[i] = " << y[i];
                    }

                    // declare loop as cooperative
                    #pragma omp for
                    for (int i = index; i < n; ++i) {
                        m[i] = y[i];
                    }
                }
            }
        }
    }

    return m;
}
