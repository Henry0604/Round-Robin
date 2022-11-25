#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <thread>
#include <chrono>
#include <random>
#include <functional>

#include <QTimer>
#include <QTime>
#include <QThread>
#include <QMessageBox>
#include <QKeyEvent>
#include <QtDebug>
#include <QEventLoop>

#include "proceso.h"

#define MAX_PROCESSES_IN_MEMORY 3

enum Pages{CAPTURE_DATA,SHOW_PROCESSES,SHOW_TIMES_PROCESSES};
enum Operations{ADD,SUB,MULT,DIV,MOD};
enum TableColumns{ID,OPERATION,RESULT,ESTIMATED,ARRIVED,FINISH,RETURN,REQUEST,WAITING,SERVICE,STATE};
enum ProgramStates{PAUSED,RUNNING};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void generateProcesses();
    Proceso generarteProcess(int id);
    void createOperation(Proceso &p,int num1,int num2,int signOperator);
    void loadProcessesMemory();
    void startProcesses();
    void showReadyProcesses();
    void runProcess();
    void showFinishedProcesses(bool finishedType, Proceso& p);
    void interruptProcesss(Proceso& p);
    void showBlockedProcesses();
    void incrementBlockedTimes();
    void calculateProcessTimes(Proceso& p);
    void createProcessesTable();
    void showProcessesBcp();
    void createNewProcess();
    void delay();

protected:

    void keyPressEvent(QKeyEvent *event);

private slots:

    void on_processNumberSP_valueChanged(int arg1);

    void on_processAcountPB_clicked();

    void on_startRunPB_clicked();

    void enable_initialButton();

    void on_quantumSP_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QEventLoop pause;

    int capturedProcess = 0;
    int totalProcess;
    int globalCounter = 0;
    int newProcessesCount = 0;
    int quantum;
    int tT;
    int tR;
    bool state = RUNNING;
    bool interrupted = false;
    bool error = false;

    std::vector<Proceso> newProcesses;
    std::vector<Proceso> readyProcesses;
    std::vector<Proceso> blockProcesses;
    std::vector<Proceso> finishedProcesses;
    std::vector<Proceso> processesTable;
};
#endif // MAINWINDOW_H
