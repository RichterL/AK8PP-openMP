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
    if (state == Qt::Checked) {
        ui->threadCount->setReadOnly(false);
        ui->arraySize->setMaximum(999);
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
        result = runScalable(arraySize, threadCount, array);
    } else {
        result = runNonScalable(arraySize, array);        
    }

    QString text;
    for (int i = 0; i < arraySize; ++i) {        
        text.append(QString::number(result[i])).append(", ");
    }
    ui->textEdit->setText(text);
}

int* MainWindow::runScalable(int arraySize, int threadCount, int *input)
{
    if (arraySize < threadCount) {
        threadCount = arraySize;
    }
    // ceil in order to work with uneven arraySize/threadCount (i.e. n=7, p=2)
    int chunkSize = ceil((float) arraySize / threadCount);
    int y[threadCount];
    int *globalPrefixPtr = y;

    #pragma omp parallel num_threads(threadCount)
    {
        int threadId = omp_get_thread_num();
        int localPrefix[chunkSize];

        int i;
        for (i = 0; i <= chunkSize - 1; ++i) {
            int index = threadId * chunkSize + i;
            if (index >= arraySize) break; // end of the array may be undefined (uneven arraySize/threadCount)
            localPrefix[i] = input[index] + (i == 0 ? 0 : localPrefix[i-1]);
        }
        // store last value for global prefix count later
        globalPrefixPtr[threadId] = localPrefix[i-1];

        #pragma omp barrier

        #pragma omp single
        {
            globalPrefixPtr = runNonScalable(threadCount, globalPrefixPtr);
        }

        // implied barrier

        for (int i = 0; i < chunkSize; ++i) {
            int index = threadId * chunkSize + i;
            if (index >= arraySize) break; // end of the array may be undefined (uneven arraySize/threadCount)
            input[index] = localPrefix[i] + (threadId == 0 ? 0 : globalPrefixPtr[threadId - 1]);
        }
    }

    return input;
}

int* MainWindow::runNonScalable(int arraySize, int *input)
{   
    // allow nested parallelism
    omp_set_nested(1);
    #pragma omp parallel num_threads(arraySize)
    {
        // every thread creates its own copy of memory
        int x[arraySize];
        for (int i = 0; i < arraySize; ++i) {
            x[i] = input[i];
        }

        // only one thread will execute the outter loop
        #pragma omp single
        {
            // only ceil(log2(n)) iterations are needed
            for (int j = 0; j < ceil(log2(arraySize)); ++j) {
                qDebug() << "SINGLE - CPU " << omp_get_thread_num() << " j = " << j;

                // create a shared variable to lower costs
                int index = pow(2,j);

                // (n - 2^j) threads will run the nested block in parallel again
                #pragma omp parallel num_threads(arraySize - index)
                {
                    // declare loop as cooperative
                    #pragma omp for
                    for (int i = index; i < arraySize; ++i) {
                        x[i] = x[i] + input[i - index];
                        qDebug() << "PARALLEL - CPU " << omp_get_thread_num() << " j = " << j << ", i = " << i << ", x[i] = " << x[i];
                    }

                    // declare loop as cooperative
                    #pragma omp for
                    for (int i = index; i < arraySize; ++i) {
                        input[i] = x[i];
                    }
                }
            }
        }
    }

    return input;
}
