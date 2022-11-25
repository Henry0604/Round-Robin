#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Simulador de Procesos");
    setFixedSize(300,200);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::generateProcesses()
{
    int estimatedTime,num1,num2,signOperator;
    //Generating random numbers
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution_time(6,16);
    std::uniform_int_distribution<int> distribution_numbers(-100,100);
    std::uniform_int_distribution<int> distribution_sign(0,5);
    auto randomNumber = bind(distribution_numbers, generator);
    auto randomSign = bind(distribution_sign,generator);
    auto randomEstimatedTime = bind(distribution_time, generator);

    for (int i = 0; i < totalProcess;){
        i++;
        Proceso p;
        p.setId(i);
        num1 = randomNumber();
        num2 = randomNumber();
        signOperator = randomSign();
        createOperation(p,num1,num2,signOperator);
        estimatedTime = randomEstimatedTime();
        p.setTiempoEstimado(estimatedTime);
        p.setQuantum(quantum);
        p.estado = States::Nuevo;
        newProcesses.push_back(p);
        newProcessesCount++;
    }
}

Proceso MainWindow::generarteProcess(int id)
{
    Proceso p;
    int estimatedTime,num1,num2,signOperator;
    //Generating random numbers
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution_time(6,15);
    std::uniform_int_distribution<int> distribution_numbers(-100,100);
    std::uniform_int_distribution<int> distribution_sign(0,5);
    auto randomNumber = bind(distribution_numbers, generator);
    auto randomSign = bind(distribution_sign,generator);
    auto randomEstimatedTime = bind(distribution_time, generator);

    p.setId(id);
    num1 = randomNumber();
    num2 = randomNumber();
    signOperator = randomSign();
    createOperation(p,num1,num2,signOperator);
    estimatedTime = randomEstimatedTime();
    p.setTiempoEstimado(estimatedTime);
    p.estado = States::Nuevo;
    p.setQuantum(quantum);

    return p;
}

void MainWindow::createOperation(Proceso &p,int num1,int num2,int signOperator)
{
    int result;
    std::string operation = "";
    char sign;
    //Generating random numbers with the system time
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution_sign(0,5);
    auto randomNumber = bind(distribution_sign,generator);

    switch (signOperator){
    case ADD:
        result = num1 + num2;
        sign = '+';
        break;
    case SUB:
        result = num1 - num2;
        sign = '-';
        break;
    case MULT:
        result = num1 * num2;
        sign = '*';
        break;
    case DIV:
        while (num2 == 0){
            num2 = randomNumber();
        }
        result = num1 / num2;
        sign = '/';
        break;
    case MOD:
        while (num2 == 0){
            num2 = randomNumber();
        }
        result = num1 % num2;
        sign = '%';
        break;
    default:
        sign = '+';
        result = num1 + num2;
        break;
    }
    operation = std::to_string(num1) + sign + std::to_string(num2);
    p.setOperacion(operation);
    p.setResultadoOperacion(result);
}

void MainWindow::loadProcessesMemory()
{
    //The maximum of processes in memory is 5
    for (int i = 0; i < MAX_PROCESSES_IN_MEMORY and i < totalProcess; i++){
        Proceso p;
        p = newProcesses[0];
        p.setTiempoLlegada(globalCounter);
        p.estado = States::Listo;
        newProcesses.erase(newProcesses.begin());
        readyProcesses.push_back(p);
    }
    totalProcess > MAX_PROCESSES_IN_MEMORY ?
                newProcessesCount = newProcessesCount - MAX_PROCESSES_IN_MEMORY:
                newProcessesCount = 0;
}

void MainWindow::startProcesses()
{
    ui->newProcessesLCD->display(newProcessesCount);
    loadProcessesMemory();
    while (readyProcesses.size()){
        showReadyProcesses();
        runProcess();
        if (readyProcesses.size() == 0 and blockProcesses.size() > 0){
            Proceso nullP(0,"NULL",0,false);
            nullP.setQuantum(0);
            readyProcesses.push_back(nullP);
        }
    }

    //Notifying that processes have finished
    QMessageBox msg;
    msg.setWindowTitle("Atencion!");
    msg.setIcon(QMessageBox::Warning);
    msg.setText("Han terminado de ejecutarse todos los procesos.");
    msg.exec();

    //In this point all the processes have finished
    ui->stackedWidget->setCurrentIndex(SHOW_TIMES_PROCESSES);
    showProcessesBcp();
}

void MainWindow::showReadyProcesses()
{
    int rows = 0;
    int i = 1;
    Proceso p;

    ui->readyProcessesTB->setRowCount(0);
    while (i < (int)readyProcesses.size()){
        p = readyProcesses[i];
        ui->readyProcessesTB->insertRow(ui->readyProcessesTB->rowCount());
        ui->readyProcessesTB->setItem(rows,0,new QTableWidgetItem(QString::number(p.getId())));
        ui->readyProcessesTB->setItem(rows,1,new QTableWidgetItem(QString::number(p.getTiempoEstimado())));
        ui->readyProcessesTB->setItem(rows,2,new QTableWidgetItem(QString::number(p.getTiempoTranscurrido())));
        rows++;
        i++;
    }
}

void MainWindow::runProcess()
{
    //Process flags
    readyProcesses[0].setFinalizacion(SUCCESFUL_FINISH);
    if (!readyProcesses[0].getEjecutado()){
        qDebug() << "Ejecutado ID = " << readyProcesses[0].getId();
        readyProcesses[0].setEjecutado(true);
        readyProcesses[0].setTiempoRespuesta(globalCounter - readyProcesses[0].getTiempoLlegada());
    }
    readyProcesses[0].estado = States::Ejecutandose;
    tT = readyProcesses[0].getTiempoTranscurrido();
    tR = readyProcesses[0].getTiempoEstimado() - tT;

    //Inicial states of the program in each process
    interrupted = error = false;

    //Printing current process running
    ui->processRuningTB->setItem(0,0,new QTableWidgetItem(QString::number(readyProcesses[0].getId())));
    ui->processRuningTB->setItem(1,0,new QTableWidgetItem(QString(readyProcesses[0].getOperacion().c_str())));
    ui->processRuningTB->setItem(2,0,new QTableWidgetItem(QString::number(readyProcesses[0].getTiempoEstimado())));
    ui->processRuningTB->setItem(3,0,new QTableWidgetItem(QString::number(readyProcesses[0].getTiempoTranscurrido())));
    ui->processRuningTB->setItem(4,0,new QTableWidgetItem(QString::number(tR)));
    ui->processRuningTB->setItem(5,0,new QTableWidgetItem(QString::number(readyProcesses[0].getQuantum())));

    if (readyProcesses[0].getId() != NULL_PROCESS){
        //Increment de tT and decrement the tR
        while (tT < readyProcesses[0].getTiempoEstimado() and tT != ACTION_CODE){
            tT++;
            tR--;
            readyProcesses[0].setTiempoTranscurrido(tT);
            readyProcesses[0].setQuantum(readyProcesses[0].getQuantum()-1);
            ui->processRuningTB->setItem(0,0,new QTableWidgetItem(QString::number(readyProcesses[0].getId())));
            ui->processRuningTB->setItem(1,0,new QTableWidgetItem(QString(readyProcesses[0].getOperacion().c_str())));
            ui->processRuningTB->setItem(2,0,new QTableWidgetItem(QString::number(readyProcesses[0].getTiempoEstimado())));
            ui->processRuningTB->setItem(3,0,new QTableWidgetItem(QString::number(readyProcesses[0].getTiempoTranscurrido())));
            ui->processRuningTB->setItem(4,0,new QTableWidgetItem(QString::number(tR)));
            ui->processRuningTB->setItem(5,0,new QTableWidgetItem(QString::number(readyProcesses[0].getQuantum())));

            if (readyProcesses[0].getQuantum() == 0){
                if (readyProcesses.size() > 1){
                    Proceso p = readyProcesses[0];
                    p.setQuantum(quantum);
                    p.estado = States::Listo;
                    readyProcesses.push_back(p);
                    readyProcesses.erase(readyProcesses.begin());
                    tT = readyProcesses[0].getTiempoTranscurrido();
                    tR = readyProcesses[0].getTiempoEstimado() - tT;
                    showReadyProcesses();
                }else{
                    readyProcesses[0].setQuantum(quantum);
                }
            }
            //Blocked Process
            if (!blockProcesses.empty()){
                incrementBlockedTimes();
            }
            showBlockedProcesses();
            globalCounter++;
            ui->globalCountLCD->display(globalCounter);
            ui->newProcessesLCD->display(newProcessesCount);
            delay();
        }
        //Checking which kind of action is next
        if (interrupted){
            interruptProcesss(readyProcesses[0]);
        }
        else if (error){
            showFinishedProcesses(ERROR_FINISH,readyProcesses[0]);
        }
        else{
            showFinishedProcesses(SUCCESFUL_FINISH,readyProcesses[0]);
        }
    }
    else{
        //Comparing if there is another process further than the null process
        while (readyProcesses.size() < 2) {
            incrementBlockedTimes();
            globalCounter++;
            showBlockedProcesses();
            ui->globalCountLCD->display(globalCounter);
            ui->newProcessesLCD->display(newProcessesCount);
            delay();
        }
    }
    //Process running finish and the ready processes is updated
    readyProcesses.erase(readyProcesses.begin());
    ui->stackedWidget->repaint();
    ui->processRuningTB->clearContents();
}

void MainWindow::showFinishedProcesses(bool finishedType,Proceso& p)
{
    if (p.getId() != NULL_PROCESS){
        //Setting finished time to a each process that finish
        p.setTiempoFinalizacion(globalCounter);
        p.setFinalizacion(finishedType);
        p.estado = States::Finalizado;
        calculateProcessTimes(p);
        finishedProcesses.push_back(p);
        //Printing the table
        int rowsFinished(ui->finishedTB->rowCount());
        ui->finishedTB->insertRow(rowsFinished);
        ui->finishedTB->setItem(rowsFinished,0,new QTableWidgetItem(QString::number(p.getId())));
        ui->finishedTB->setItem(rowsFinished,1,new QTableWidgetItem(QString(p.getOperacion().c_str())));

        if (finishedType == ERROR_FINISH){
            ui->finishedTB->setItem(rowsFinished,2,new QTableWidgetItem(QString("ERROR")));
        }
        else{
            ui->finishedTB->setItem(rowsFinished,2,new QTableWidgetItem(QString::number(p.getResultadoOperacion())));
        }
        //Adding a new process when someone finish
        if (newProcesses.size()){
            Proceso p;
            p = newProcesses[0];
            newProcesses.erase(newProcesses.begin());
            //Setting arrive time for each new process in ready processes
            p.setTiempoLlegada(globalCounter);
            p.estado = States::Listo;
            readyProcesses.push_back(p);
            newProcessesCount--;
        }
    }
}

void MainWindow::interruptProcesss(Proceso& p)
{
    //Pasing the current process into the block queue
    p.estado = States::Bloqueado;
    p.setQuantum(quantum);
    blockProcesses.push_back(p);
    showBlockedProcesses();
}

void MainWindow::showBlockedProcesses()
{
    Proceso p;
    int i = 0,row = 0;

    //Showing the current values of the blocked processes
    ui->blockTB->setRowCount(0);
    while (i < (int)blockProcesses.size()){
        p = blockProcesses[i];
        ui->blockTB->insertRow(ui->blockTB->rowCount());
        ui->blockTB->setItem(row,0,new QTableWidgetItem(QString::number(p.getId())));
        ui->blockTB->setItem(row,1,new QTableWidgetItem(QString::number(p.getTiempoBloqueado())));
        i++;
        row++;
    }
}

void MainWindow::incrementBlockedTimes()
{
    //Increment the blocked time of each process
    for (auto it = blockProcesses.begin(); it != blockProcesses.end();){
        (*it).setTiempoBloqueado((*it).getTiempoBloqueado() + 1);

        if ((*it).getTiempoBloqueado() == BLOCKED_TIME){
            Proceso p;
            p = *it;
            p.setTiempoBloqueado(0);
            p.estado = States::Listo;
            readyProcesses.push_back(p);
            blockProcesses.erase(it);
            showReadyProcesses();
        }
        else{
            it++;
        }
    }
}

void MainWindow::calculateProcessTimes(Proceso& p)
{
    if (p.estado == States::Finalizado){
        p.setTiempoRetorno(p.getTiempoFinalizacion()-p.getTiempoLlegada());
        p.setTiempoServicio(p.getTiempoTranscurrido());
        p.setTiempoEspera(p.getTiempoRetorno()-p.getTiempoServicio());
    }
    else if (p.estado != States::Nuevo){
        p.setTiempoServicio(p.getTiempoTranscurrido());
        p.setTiempoEspera(globalCounter-p.getTiempoTranscurrido());
    }
}

void MainWindow::createProcessesTable()
{
    int i;
    if (!processesTable.empty()){
        processesTable.clear();
    }
    for (i = 0; i < totalProcess; i++){
        if (i < (int)readyProcesses.size()){
            processesTable.push_back(readyProcesses[i]);
        }
        if (i < (int)blockProcesses.size()){
            processesTable.push_back(blockProcesses[i]);
        }
        if (i < (int)newProcesses.size()){
            processesTable.push_back(newProcesses[i]);
        }
        if (i < (int)finishedProcesses.size()){
            processesTable.push_back(finishedProcesses[i]);
        }
    }
    //Sorting the elements of the processes table
    std::sort(processesTable.begin(), processesTable.end());
}

void MainWindow::showProcessesBcp()
{
    int row;
    Proceso p;
    QString stateProcess,result,request,arrived,finish,retrn,waiting,service;
    //Creating the processes table
    createProcessesTable();
    //Showing the processes table
    ui->timesProcessesTB->setRowCount(0);
    for (int i = 0; i < (int)processesTable.size(); i++){
        p = processesTable[i];
        calculateProcessTimes(p);
        row = ui->timesProcessesTB->rowCount();
        ui->timesProcessesTB->insertRow(row);
        //ID, Operation, Estimated time ...
        ui->timesProcessesTB->setItem(row,ID,new QTableWidgetItem(QString::number(p.getId())));
        ui->timesProcessesTB->setItem(row,OPERATION,new QTableWidgetItem(QString(p.getOperacion().c_str())));
        ui->timesProcessesTB->setItem(row,ESTIMATED,new QTableWidgetItem(QString::number(p.getTiempoEstimado())));
        //Selecting the values to show
        switch (p.estado){
        case States::Finalizado:
        case States::Listo:
        case States::Bloqueado:
        case States::Ejecutandose:
            arrived = QString::number(p.getTiempoLlegada());
            service = QString::number(p.getTiempoServicio());
            request = QString::number(p.getTiempoRespuesta());
            waiting = QString::number(p.getTiempoEspera());
            switch (p.estado) {
            case States::Finalizado:
                finish = QString::number(p.getTiempoFinalizacion());
                retrn = QString::number(p.getTiempoRetorno());
                if (p.getFinalizacion()){
                    result = QString::number(p.getResultadoOperacion());
                }else{
                    result = "ERROR";
                }
                break;
            default:
                result = finish = retrn = "NULL";
            }
            break;
        default:
            result = finish = retrn = arrived = service = request = waiting = "NULL";
            break;
        }
        //Showing the process state
        switch (p.estado){
        case States::Listo:         stateProcess = "LISTO";         break;
        case States::Nuevo:         stateProcess = "NUEVO";         break;
        case States::Ejecutandose:  stateProcess = "EJECUCION";     break;
        case States::Bloqueado:     stateProcess = "BLOQUEADO";     break;
        case States::Finalizado:    stateProcess = "FINALIZADO";    break;
        default: ;
        }
        //Showing values
        ui->timesProcessesTB->setItem(row,RESULT,new QTableWidgetItem(result));
        ui->timesProcessesTB->setItem(row,ARRIVED,new QTableWidgetItem(arrived));
        ui->timesProcessesTB->setItem(row,FINISH,new QTableWidgetItem(finish));
        ui->timesProcessesTB->setItem(row,RETURN,new QTableWidgetItem(retrn));
        ui->timesProcessesTB->setItem(row,REQUEST,new QTableWidgetItem(request));
        ui->timesProcessesTB->setItem(row,WAITING,new QTableWidgetItem(waiting));
        ui->timesProcessesTB->setItem(row,SERVICE,new QTableWidgetItem(service));
        ui->timesProcessesTB->setItem(row,STATE,new QTableWidgetItem(stateProcess));
    }
}

void MainWindow::createNewProcess()
{
    Proceso p;

    p = generarteProcess(totalProcess + 1);

    if ((int)(readyProcesses.size() + blockProcesses.size()) < MAX_PROCESSES_IN_MEMORY){
        p.estado = States::Listo;
        p.setTiempoLlegada(globalCounter);
        readyProcesses.push_back(p);
        showReadyProcesses();
    }
    else{
        p.estado = States::Nuevo;
        newProcesses.push_back(p);
        newProcessesCount++;
    }
    totalProcess++;
}

void MainWindow::delay()
{
    QTime delayTime = QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < delayTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}

//SLOTS

void MainWindow::on_processNumberSP_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    enable_initialButton();
}

void MainWindow::on_quantumSP_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    enable_initialButton();
}

void MainWindow::on_processAcountPB_clicked()
{
    ui->processNumberSP->setEnabled(false);
    totalProcess = ui->processNumberSP->value();
    quantum = ui->quantumSP->value();
    generateProcesses();
    ui->stackedWidget->setCurrentIndex(SHOW_PROCESSES);
    setFixedSize(900,580);
}

void MainWindow::on_startRunPB_clicked()
{
    startProcesses();
    ui->startRunPB->setEnabled(false);
}

void MainWindow::enable_initialButton()
{
    if (ui->processNumberSP->value() > 0 and ui->quantumSP->value() > 0){
        ui->processAcountPB->setEnabled(true);
    }
    else{
        ui->processAcountPB->setEnabled(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_E and state == RUNNING){
        qDebug() << "Interrupcion";
        interrupted = true;
        tT = ACTION_CODE;
    }
    else if (event->key() == Qt::Key_W and state == RUNNING){
        qDebug() << "Error";
        error = true;
        tT = ACTION_CODE;
    }
    else if (event->key() == Qt::Key_P and state == RUNNING){
        qDebug() << "Pause";
        state = PAUSED;
        pause.exec();
    }
    else if (event->key() == Qt::Key_C and state == PAUSED){
        qDebug() << "Continue";
        state = RUNNING;
        if (ui->stackedWidget->currentIndex() == SHOW_TIMES_PROCESSES){
            ui->stackedWidget->setCurrentIndex(SHOW_PROCESSES);
        }
        pause.quit();
    }
    else if (event->key() == Qt::Key_B and state == RUNNING){
        qDebug() << "BCP";
        state = PAUSED;
        ui->stackedWidget->setCurrentIndex(SHOW_TIMES_PROCESSES);
        showProcessesBcp();
        this->setFocus();
        pause.exec();
        this->setFocus();
    }
    else if (event->key() == Qt::Key_N and state == RUNNING){
        qDebug() << "New process";
        createNewProcess();
    }
    else{
        qDebug() << "Nothing";
    }
}
