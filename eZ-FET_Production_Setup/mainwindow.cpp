//*****************************************************************************
//
// Copyright (C) 2012 - 2013 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the
//  distribution.
//
//  Neither the name of Texas Instruments Incorporated nor the names of
//  its contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//****************************************************************************

// Header section
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "MSP430_EEM.h"
#include "MSP430_FET.h"

// Global variables
long version, uifStatus = -1;

// Context
MainWindow *pW = NULL;


/******************************************************************************
 * Typedefs for Pointer to DLL API calls
 */
typedef LONG(WINAPI *DLL430VOID)              (void);
typedef LONG(WINAPI *DLL430CHARPLONGP)        (CHAR*, LONG*);
typedef LONG(WINAPI *DLL430CHARPLONGLONG)     (CHAR*, LONG, LONG);
typedef LONG(WINAPI *DLL430CHARPCHARPLONGLONGLONG) (CHAR*,CHAR*, LONG, LONG, LONG);
typedef LONG(WINAPI *DLL430CHARPLONG)		  (CHAR*, LONG);
typedef LONG(WINAPI *DLL430BYTEPPLONGP)       (BYTE*, LONG*);
typedef LONG(WINAPI *DLL430WORDWORDPWORDP)    (WORD, WORD*, WORD*);
typedef LONG(WINAPI *DLL430LONG)              (LONG);
typedef CHAR*(WINAPI *CDLL430LONG)            (LONG);
typedef LONG(WINAPI *DLL430LONGLONG)          (LONG, LONG);
typedef LONG(WINAPI *DLL430LONGLONGLPTSTRLONG)(LONG, LONG, LPTSTR, LONG);
typedef LONG(WINAPI *DLL430LONGLONGLONG)      (LONG, LONG, LONG);
typedef LONG(WINAPI *DLL430LONGCHARPLONG)     (LONG, CHAR*, LONG);
typedef LONG(WINAPI *DLL430LONGCHARPLONGLONG) (LONG, CHAR*, LONG, LONG);
typedef LONG(WINAPI *DLL430LONGCHARPPLONGP)   (LONG, CHAR**, LONG*);
typedef LONG(WINAPI *DLL430LONGLONGCHARP)     (LONG, LONG, CHAR*);
typedef LONG(WINAPI *DLL430LONGLONGP)         (LONG, LONG*);
typedef LONG(WINAPI *DLL430LONGP)             (LONG*);
typedef LONG(WINAPI *DLL430LONGPLONGP)        (LONG*, LONG*);
typedef LONG(WINAPI *DLL430LONGPLONGLONG)     (LONG*, LONG, LONG);
typedef LONG(WINAPI *DLL430LONGPLONGLONGP)    (LONG*, LONG, LONG*);
typedef LONG(WINAPI *DLL430LPTSTRLONGLONG)    (LPTSTR, LONG, LONG);
typedef LONG(WINAPI *DLL430LPTSTR)            (LPTSTR);
typedef LONG(WINAPI *DLL430FETFWUPDATE)       (LPTSTR, DLL430_FET_NOTIFY_FUNC, LONG);
typedef LONG(WINAPI *DLL430WORDPBPPARAMP)     (WORD*, BpParameter_t*);
typedef LONG(WINAPI *DLL430EEMINIT)           (MSP430_EVENTNOTIFY_FUNC, LONG, MessageID_t*);
typedef LONG(WINAPI *DLL430FETFWUPDATE)       (LPTSTR, DLL430_FET_NOTIFY_FUNC, LONG);

// Pointer to DLL API calls
DLL430CHARPLONGP pDLL_Initialize;
DLL430LONG pDLL_Close;
DLL430CHARPLONGLONG pDLL_Identify;
DLL430CHARPCHARPLONGLONGLONG pDLL_OpenDevice;
DLL430CHARPLONG pDLL_GetFoundDevice;
DLL430LONGLONG pDLL_Configure;
DLL430LONG pDLL_VCC;
DLL430LONGLONGLONG pDLL_Reset;
DLL430LONGLONGLONG pDLL_Erase;
DLL430LONGCHARPLONGLONG pDLL_Memory;
DLL430VOID pDLL_Error_Number;
CDLL430LONG pDLL_Error_String;
DLL430LONGLONG pDLL_Run;
DLL430LONGPLONGLONGP pDLL_State;
DLL430LPTSTRLONGLONG pDLL_ProgramFile;
DLL430LONGCHARPLONG pDLL_Device;
DLL430LONGPLONGLONG pDLL_Registers;
DLL430EEMINIT pDLL_EEM_Init;
DLL430WORDPBPPARAMP pDLL_EEM_SetBreakpoint;
DLL430FETFWUPDATE pDLL_FET_FwUpdate;

// Empty DLL callback function
void DLL_Callbacks(UINT MsgId,ULONG wParam, ULONG lParam, LONG clientHandle)
{
}

// Constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialize variables
    ui->setupUi(this);
    pW = this;

    // Run-time linking of MSP430.DLL
    loadLibrary();

    logFile = NULL;
    testPass = false;
    testOver = true;

    // Read configuration file
    QFile  *configFile = NULL;
    configFile = new QFile(CONFIG_FILE);
    if(!configFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->msgList->addItem("Could not open config file.");
        return;
    }
    else
    {
        int lineLength;
        char buf[1024];
        while(!configFile->atEnd())
        {
            lineLength = configFile->readLine(buf, sizeof(buf));
            if (lineLength != -1)
            {
                if(buf[0]!=32) //" "
                {
                    if(buf[0]!=35)//"#"
                    {
                        QString s = QString::fromLatin1(buf);
                        if(s.contains("log_file="))
                        {
                           ui->logFilePathBox->setText(s.remove("log_file=", Qt::CaseInsensitive).trimmed());
                        }
                        if(s.contains("bsl_file="))
                        {
                           ui->bslPathBox->setText(s.remove("bsl_file=", Qt::CaseInsensitive).trimmed());
                        }
                        if(s.contains("emulation_test_file="))
                        {
                           ui->hostTestCodePathBox->setText(s.remove("emulation_test_file=", Qt::CaseInsensitive).trimmed());
                        }
                        if(s.contains("emulation_production_file="))
                        {
                           ui->hostProdCodePathBox->setText(s.remove("emulation_production_file=", Qt::CaseInsensitive).trimmed());
                        }
                    }
                }
            }

        }
    }
    configFile->close();

    // Scan and add MSP-FET430UIF tool -- once
    scanForUIF();

    QObject::connect(ui->msgList->model(),SIGNAL(rowsInserted ( const QModelIndex &, int, int ) ),ui->msgList, SLOT(scrollToBottom ()));

    // Setup timers
    waitHostTestEndTimer = new QTimer(this);
}

// Destructor
MainWindow::~MainWindow()
{
    //Close files and release pointers
    if (logFile)
    {
        logFile->close();
        delete logFile;
        logFile = NULL;
    }
    pDLL_Close(0);
    delete waitHostTestEndTimer;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
 {
    //Close files and release pointers
    if (logFile)
    {
        logFile->close();
        delete logFile;
        logFile = NULL;
    }
    waitHostTestEndTimer->stop();
    closeUIF();
}

// Dummy callback handler for EEM
void MY_MSP430_EVENTNOTIFY_FUNC(UINT MsgId, WPARAM wParam, LPARAM lParam, LONG clientHandle)
{
    if(clientHandle)
    {
    }
}


// -------------------------------------------------------------------------------
// GUI functions

// Enable GUI controls
void MainWindow::enableControls()
{
    ui->uifBox->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->logFilePathBox->setEnabled(true);
    ui->selectLogFileButton->setEnabled(true);
    ui->enableExpertModeBox->setEnabled(true);
    if(ui->enableExpertModeBox->isChecked())
    {
        // Emu BSL
        ui->bslPathBox->setEnabled(true);
        ui->selectBslButton->setEnabled(true);
        ui->programBslButton->setEnabled(true);

        // Emu test
        ui->hostTestCodePathBox->setEnabled(true);
        ui->selectHostTestFWButton->setEnabled(true);
        ui->programHostTestFWButton->setEnabled(true);

        // Emu production
        ui->hostProdCodePathBox->setEnabled(true);
        ui->selectHostProdFWButton->setEnabled(true);
        ui->programHostProdFWButton->setEnabled(true);
    }
}

// Disable GUI controls
void MainWindow::disableControls()
{
    ui->uifBox->setEnabled(false);
    ui->logFilePathBox->setEnabled(false);
    ui->selectLogFileButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->enableExpertModeBox->setEnabled(false);
    if(ui->enableExpertModeBox->isChecked())
    {
        // Emu BSL
        ui->bslPathBox->setEnabled(false);
        ui->selectBslButton->setEnabled(false);
        ui->programBslButton->setEnabled(false);

        // Emu test
        ui->hostTestCodePathBox->setEnabled(false);
        ui->selectHostTestFWButton->setEnabled(false);
        ui->programHostTestFWButton->setEnabled(false);

        // Emu production
        ui->hostProdCodePathBox->setEnabled(false);
        ui->selectHostProdFWButton->setEnabled(false);
        ui->programHostProdFWButton->setEnabled(false);
    }
}

// Prints messages in the GUI and the log file
void MainWindow::manageAlert(QFile *f, QString msg, int type)
{
    // Avoid useless DLL error string
    if (msg == "No error")
    {
        return;
    }

    QTextStream out(f);
    switch(type)
    {
        case MSG_START: timeStamp = QDateTime::currentDateTime().toString(Qt::SystemLocaleDate);
                        out << "-----------------------------------------------------------------------------------------------------------------------\n";
                        out << timeStamp << ": START\n";
                        break;
        case MSG_ERROR: out << timeStamp << ": ERROR - " << msg << "\n";
                        ui->msgList->addItem(msg);
                        break;
        case MSG_INFO:  ui->msgList->addItem(msg);
                        break;
        case MSG_OPERATOR:
                        ui->uartList->clear();
                        ui->uartList->addItem(msg);
                        break;
        case MSG_LOG:   ui->msgList->addItem(msg);
                        out << timeStamp << ": " << msg << "\n";
                        break;
        default:        break;
    }
    out.flush();
    QCoreApplication::processEvents();
}

// Enables/disables controls
void MainWindow::on_enableExpertModeBox_clicked(bool checked)
{
    if (checked)
    {
        ui->selectBslButton->setEnabled(true);
        ui->selectHostTestFWButton->setEnabled(true);
        ui->selectHostProdFWButton->setEnabled(true);
        ui->bslPathBox->setEnabled(true);
        ui->hostTestCodePathBox->setEnabled(true);
        ui->programBslButton->setEnabled(true);
        ui->programHostTestFWButton->setEnabled(true);
        ui->hostTestCodePathBox->setEnabled(true);
        ui->hostProdCodePathBox->setEnabled(true);
        ui->programHostProdFWButton->setEnabled(true);
    }
    else
    {
        ui->selectBslButton->setEnabled(false);
        ui->selectHostTestFWButton->setEnabled(false);
        ui->selectHostProdFWButton->setEnabled(false);
        ui->bslPathBox->setEnabled(false);
        ui->hostTestCodePathBox->setEnabled(false);
        ui->hostProdCodePathBox->setEnabled(false);
        ui->programBslButton->setEnabled(false);
        ui->programHostTestFWButton->setEnabled(false);
        ui->programHostProdFWButton->setEnabled(false);
    }
}

// Function that open a file select dialog for bsl, test code, production code and log file.
void MainWindow::on_selectBslButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Files (*.txt)"));
    if(fileName.count()>0)
        ui->bslPathBox->setText(fileName);
}

// Function that open a file select dialog for bsl, test code, production code and log file.
void MainWindow::on_selectHostTestFWButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Files (*.txt)"));
    if(fileName.count()>0)
        ui->hostTestCodePathBox->setText(fileName);
}

// Function that open a file select dialog for bsl, test code, production code and log file.
void MainWindow::on_selectHostProdFWButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Files (*.txt)"));
    if(fileName.count()>0)
        ui->hostProdCodePathBox->setText(fileName);
}

// Function that open a file select dialog for bsl, test code, production code and log file.
void MainWindow::on_selectLogFileButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),"",tr("Files (*.txt)"));
    if(fileName.count()>0)
        ui->logFilePathBox->setText(fileName);
}



// -------------------------------------------------------------------------------
// Tool functions

// Scan for connected MSP-FET430UIF
void MainWindow::scanForUIF()
{
    QString str = NULL;
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for (int i=0;i<ports.size();i++)
    {
        if (ports.at(i).friendName.contains("MSP-FET430UIF", Qt::CaseInsensitive))
        {
            str = ports.at(i).portName;
            uifNameBA = str.toLocal8Bit();
            uifName = uifNameBA.data();
            ui->uifBox->addItem("MSP-FET430UIF ("+str+")");
            break;
        }
    }
}

// Connects to the selected MSP-FET430UIF
void MainWindow::connectUIF()
{
    if(ui->uifBox->count()==0)
    {
        manageAlert(logFile,"No MSP-FET430UIF connected.",MSG_ERROR);
        return;
    }

    bpTimeout=0;

    // Open connection with UIF
    status = pDLL_Initialize(uifName,&version);
    if(status==STATUS_OK)
    {
        if (version==-3)
        {
            // V2 firmware --  break here
            QMessageBox msgBox;
            msgBox.setText("MSP-FET430UIF with old V2 firmware detected!");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setInformativeText("Please update the firmware to V3.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            status = STATUS_ERROR;
            return;
        }
        else if (version!=-1)
        {
            // DLL and FW versions match
            manageAlert(logFile,"Port: "+QString::fromLatin1(uifName)+" / DLL Version: "+QString::number(version),MSG_INFO);
        }
        else
        {
            //DLL and FW versions mismatch
            manageAlert(logFile,"DLL and MSP-FET430UIF firmware version mismatch.",MSG_ERROR);

            //Ask the user to update the FW
            QMessageBox msgBox;
            msgBox.setWindowTitle("MSP430 eZ-FET Test and Program Tool");
            msgBox.setText("Outdated MSP-FET430UIF firmware detected!");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setInformativeText("Do you want to update the MSP-FET430UIF firmware?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            switch (ret)
            {
               case QMessageBox::Cancel:
                    enableControls();
                    return;
               case QMessageBox::Ok:
                    //Start FW update
                    manageAlert(logFile,"Updating MSP-FET430UIF firmware, please wait...",MSG_INFO);
                    QCoreApplication::processEvents();
                    pDLL_FET_FwUpdate((LPTSTR)NULL, DLL_Callbacks, NULL);
                    QCoreApplication::processEvents();
                    if(status == STATUS_OK)
                    {
                        manageAlert(logFile,"MSP-FET430UIF firmware updated successfully. Restart test.",MSG_INFO);
                        status = STATUS_ERROR;  //block sequence after fw update
                        return;
                    }
                    else
                    {
                        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
                        return;
                    }

            }
        }
    }
    else
    {
        // Get Error code and log it
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
    }

    // Refresh the GUI
    QCoreApplication::processEvents();

    // Set device VCC and communication mode, the open connection
    status = pDLL_VCC(DEVICE_VCC);

    status = pDLL_Configure(INTERFACE_MODE, SPYBIWIREJTAG_IF);
    if (status != STATUS_OK)
    {
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
        closeUIF();
        ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
        enableControls();
        return;
    }
    status = pDLL_OpenDevice(EZFET_DEVICE_NAME,"",0x0,0x0,DEVICE_UNKNOWN);
    if (status != STATUS_OK)
    {
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
        closeUIF();
        ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
        enableControls();
        return;
    }

    //Get connected device info
    DEVICE_T device;
    status = pDLL_GetFoundDevice(device.buffer,sizeof(device.buffer));
    if (status == STATUS_OK)
    {
        QString deviceName = QString::fromLatin1((const char *)device.string);
        if((deviceName==EZFET_DEVICE_NAME))
        {
            manageAlert(logFile,"Device: "+deviceName+" / eZ-FET",MSG_INFO);
        }
        else
        {
            manageAlert(logFile,"Device mismatch. Detected device: "+deviceName+". Expected devices: "+EZFET_DEVICE_NAME,MSG_INFO);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }
    }
    else
    {
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
        return;
    }
}

// Close MSP-FET430UIF
void MainWindow::closeUIF()
{
    // Close FET debug channel
    pDLL_Close(true);
}


// -------------------------------------------------------------------------------
// Button functions

// Start testing and programming sequence
void MainWindow::on_startButton_clicked()
{
    // Open log file
    if(logFile==NULL)
    {
        logFile = new QFile(ui->logFilePathBox->text());
        if(!logFile->open(QIODevice::Append | QIODevice::Text))
        {
            ui->msgList->addItem("Could not open log file!");
            enableControls();
            return;
        }
    }
    ui->msgList->clear();
    ui->uartList->clear();
    manageAlert(logFile,"\n",MSG_START);

    // Emulate clicks on Expert buttons
    on_programBslButton_clicked();
    if (status != STATUS_OK)
    {
        manageAlert(logFile,"Test failed. Repeat once.",MSG_OPERATOR);
        return;
    }
    on_programHostTestFWButton_clicked();
    if (status != STATUS_OK)
    {
        manageAlert(logFile,"Test failed. Repeat once.",MSG_OPERATOR);
        return;
    }
    on_programHostProdFWButton_clicked();
    if (status != STATUS_OK)
    {
        manageAlert(logFile,"Test failed. Repeat once.",MSG_OPERATOR);
    }
    else
    {
        manageAlert(logFile,"Test passed. Next board.",MSG_OPERATOR);
    }
}

// Program emulation BSL
void MainWindow::on_programBslButton_clicked()
{
    // Open log file
    if(logFile==NULL)
    {
        logFile = new QFile(ui->logFilePathBox->text());
        if(!logFile->open(QIODevice::Append | QIODevice::Text))
        {
            ui->msgList->addItem("Could not open log file!");
            return;
        }
    }

    disableControls();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: yellow;");
    ui->uartList->clear();
    connectUIF();
    if(status != STATUS_ERROR)
    {
        manageAlert(logFile,"Programming BSL.",MSG_INFO);

        // Download BSL to the device from the default path
        status = pDLL_Configure(UNLOCK_BSL_MODE,1);
        if (status != STATUS_OK)
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Program & verify BSL
        status = pDLL_ProgramFile((LPTSTR)ui->bslPathBox->text().toLatin1().data(),ERASE_ALL,true);
        if (status == STATUS_OK)
        {
            manageAlert(logFile,"BSL programmed.",MSG_INFO);
        }
        else
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Lock BSL memory
        status = pDLL_Configure(UNLOCK_BSL_MODE,0);
        if (status != STATUS_OK)
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

    }
    else
    {
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
        closeUIF();
        ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
        enableControls();
        return;
    }
    closeUIF();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: green;");
    enableControls();
}

BpParameter_t	BpParameter;

// Download and execute emulation test code
void MainWindow::on_programHostTestFWButton_clicked()
{
    // Open log file
    if(logFile==NULL)
    {
        logFile = new QFile(ui->logFilePathBox->text());
        if(!logFile->open(QIODevice::Append | QIODevice::Text))
        {
            ui->msgList->addItem("Could not open log file!");
            return;
        }
    }

    disableControls();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: yellow;");
    ui->uartList->clear();
    connectUIF();
    if(status != STATUS_ERROR)
    {
        manageAlert(logFile,"Programming emulation test firmware.",MSG_INFO);

        // Download test software to the device from the desired path
        status = pDLL_ProgramFile((LPTSTR)ui->hostTestCodePathBox->text().toLatin1().data(),ERASE_ALL,true);
        if (status == STATUS_OK)
        {
            manageAlert(logFile,"Emulation test firmware programmed.",MSG_INFO);
        }
        else
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Set breakpoint to the end of the test program to detect when the tests have finished,
        // then run the program
        QCoreApplication::processEvents();

        MessageID_t MsgId;
        pDLL_EEM_Init(MY_MSP430_EVENTNOTIFY_FUNC, (LONG)this, &MsgId);

        WORD wBpHandle = 0;
        BpParameter.bpMode =		BP_CODE;
        BpParameter.lAddrVal =		BP_ADDRESS;
        BpParameter.bpType =		BP_MAB;
        BpParameter.lReg =			0;
        BpParameter.bpAccess =		BP_FETCH;
        BpParameter.bpAction =		BP_BRK;
        BpParameter.bpOperat =		BP_EQUAL;
        BpParameter.lMask =			0xFFFFF;
        BpParameter.lRangeEndAdVa = 0x00000;
        BpParameter.bpRangeAction = BP_INSIDE;
        BpParameter.lCondMdbVal =	0x00000;
        BpParameter.bpCondAccess =	BP_FETCH;
        BpParameter.lCondMask =		0xFFFFF;
        BpParameter.bpCondOperat =	BP_EQUAL;
        pDLL_EEM_SetBreakpoint(&wBpHandle, &BpParameter);

        status = pDLL_Run(RUN_TO_BREAKPOINT,false);
        if (status == STATUS_OK)
        {
            manageAlert(logFile,"Emulation test firmware running.",MSG_INFO);
        }
        else
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        //Start timer that checks every 1s if the device has finished tests
        //waitHostTestEndTimer = new QTimer(this);
        waitHostTestEndTimer->setInterval(3000);
        connect(waitHostTestEndTimer, SIGNAL(timeout()), this, SLOT(waitHostTestEndTimeout()));
        waitHostTestEndTimer->start();
    }
    else
    {
        manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
        closeUIF();
        ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
        enableControls();
        return;
    }
    // Wait for test code to terminate
    while(waitHostTestEndTimer->isActive())
    {
        QCoreApplication::processEvents();
    }
    if(waitHostTestEndTimer->property("status")!=STATUS_OK)
    {
        ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
        enableControls();
        closeUIF();
        return;
    }

    closeUIF();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: green;");
    enableControls();
}

// Download and execute emulation test code
void MainWindow::on_programHostProdFWButton_clicked()
{
    // Open log file
    if(logFile==NULL)
    {
        logFile = new QFile(ui->logFilePathBox->text());
        if(!logFile->open(QIODevice::Append | QIODevice::Text))
        {
            ui->msgList->addItem("Could not open log file!");
            return;
        }
    }

    disableControls();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: yellow;");
    ui->uartList->clear();

    connectUIF();
    if(status != STATUS_ERROR)
    {
        // Unlock Info A
        status = pDLL_Configure(LOCKED_FLASH_ACCESS,true);
        if (status != STATUS_OK)
        {
            manageAlert(logFile,"Could not unlock Info A before programming.",MSG_INFO);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Program production firmware
        manageAlert(logFile,"Programming emulation firmware.",MSG_INFO);

        status = pDLL_ProgramFile((LPTSTR)ui->hostProdCodePathBox->text().toLatin1().data(),ERASE_ALL,true);
        if (status == STATUS_OK)
        {
            manageAlert(logFile,"Emulation firmware programmed.",MSG_INFO);
        }
        else
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Lock Info A
        status = pDLL_Configure(LOCKED_FLASH_ACCESS,false);
        if (status != STATUS_OK)
        {
            manageAlert(logFile,"Could not lock Info A after programming.",MSG_INFO);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        // Let emulation run
        status = pDLL_Reset(RST_RESET, false, false);
        if(status == STATUS_ERROR)
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }

        pDLL_Run(RUN_TO_BREAKPOINT, true);
        if(status == STATUS_ERROR)
        {
            manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
            closeUIF();
            ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
            enableControls();
            return;
        }
    }

    closeUIF();
    ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: green;");
    enableControls();
}




// Reads Host MCU RAM to check test status
void MainWindow::waitHostTestEndTimeout()
{
    //Initialize buffer array
    char ram1[TEST_ARRAY_LENGTH];
    unsigned char ram[TEST_ARRAY_LENGTH];
    for(int i=0; i<TEST_ARRAY_LENGTH;i++)
    {
        ram1[i]=0;
    }

    // Halt the device and read RAM
    long state, cycles = 0;
    pDLL_State(&state,true,&cycles);

    LONG registers[16];
    QString string;
    status = pDLL_Registers(registers, MASKREG(PC), READ);
    string = "PC="+QString::number(registers[PC],16);
    manageAlert(logFile,string,MSG_INFO);

    status = pDLL_Memory(TEST_ARRAY_LOCATION,ram1,TEST_ARRAY_LENGTH,READ);
    waitHostTestEndTimer->setProperty("status",status);

    for(int i=0; i<TEST_ARRAY_LENGTH;i++)
    {
        ram[i]=ram1[i];
    }

    if(status==STATUS_OK)
    {
        // Check global pass/fail signature
        if (ram[0] != 0)
        {
            for(int i=1;i<TEST_ARRAY_LENGTH;i=i+6)
            {
                unsigned char type = (unsigned char)ram[i];
                unsigned char result = (unsigned char)ram[i+1];
                unsigned long value = ((unsigned long)ram[i+2]<<24) + ((unsigned long)ram[i+3]<<16) + ((unsigned long)ram[i+4]<<8) + ram[i+5];

                QString string;
                switch (type)
                {
                    case TEST_LOTID:    string = "eZ-FET Test: Lot ID="+QString::number(value)+" (PASS)";
                                        break;
                    case TEST_DIEXY:    string = "eZ-FET Test: Die X-Pos="+QString::number(value>>16)+", Die Y-Pos="+QString::number(value&0xFFFF)+" (PASS)";
                                        break;
                    case TEST_VBUS:     if (result == TEST_PASS)
                                            string = "eZ-FET Test: VBUS="+QString::number(value)+"mV (PASS)";
                                        else
                                            string = "eZ-FET Test: VBUS="+QString::number(value)+"mV (FAIL)";
                                        break;
                    case TEST_VOUT:     if (result == TEST_PASS)
                                            string = "eZ-FET Test: VOUT="+QString::number(value)+"mV (PASS)";
                                        else
                                            string = "eZ-FET Test: VOUT="+QString::number(value)+"mV (FAIL)";
                                        break;
                    case TEST_XT2:      if (result == TEST_PASS)
                                            string = "eZ-FET Test: XT2="+QString::number(value)+"Hz (PASS)";
                                        else
                                            string = "eZ-FET Test: XT2="+QString::number(value)+"Hz (FAIL)";
                                        break;
                    case TEST_DCDC_MCU:
                                    if (result == TEST_PASS)
                                        string = "eZ-FET Test: DCDC MCU="+QString::number(value)+ " (PASS)";
                                    else
                                        if (value==1)
                                            string = "eZ-FET Test: DCDC MCU Programming (FAIL)";
                                        else if (value==2)
                                            string = "eZ-FET Test: DCDC MCU I2C Communication (FAIL)";
                                        else
                                            string = "eZ-FET Test: DCDC MCU I2C version mismatch (FAIL)";
                                    break;
                    case TEST_UART:     if (result == TEST_PASS)
                                            string = "eZ-FET Test: UART Loopback (PASS)";
                                        else
                                            string = "eZ-FET Test: UART Loopback (FAIL)";
                                        break;
                    case TEST_STANDBY_CURRENT:  if (result == TEST_PASS)
                                                    string = "eZ-FET Test: Standby current "+QString::number(value)+" (PASS)";
                                                else
                                                    string = "eZ-FET Test: Standby current "+QString::number(value)+" (FAIL)";
                                                break;
                    case TEST_OVER: i = TEST_ARRAY_LENGTH;
                }
                // Print test result
                if (i<TEST_ARRAY_LENGTH)
                {
                    manageAlert(logFile,string,MSG_LOG);
                }
            }

            if ((unsigned char)ram[0] == TEST_PASS)
            {
                manageAlert(logFile,"All eZ-FET tests have passed.",MSG_LOG);
                waitHostTestEndTimer->setProperty("status",STATUS_OK);
                waitHostTestEndTimer->stop();
                status = STATUS_OK;
            }
            else
            {
                manageAlert(logFile,"Some eZ-FET tests have failed. Restart test.",MSG_LOG);
                waitHostTestEndTimer->setProperty("status",STATUS_ERROR);
                waitHostTestEndTimer->stop();
                status = STATUS_ERROR;
            }

            return;
        }
       //Device is still testing, continue execution
       else
       {
           QString s = ui->msgList->item(ui->msgList->count()-1)->text();
           ui->msgList->item(ui->msgList->count()-1)->setText(s+".");
           pDLL_Run(RUN_TO_BREAKPOINT,false);
           // Exit condition if code execution gets stuck, bpTimeout has to be setted to wait for FPGA programming
           if(bpTimeout==BP_TIMEOUT)
           {
               manageAlert(logFile,"Breakpoint timeout reached! Please restart test.",MSG_ERROR);
               waitHostTestEndTimer->setProperty("status",STATUS_ERROR);
               waitHostTestEndTimer->stop();
               closeUIF();
               ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
               enableControls();
               return;
           }
           else
           {
               bpTimeout++;
           }
       }
    }
    else
    {
       manageAlert(logFile,QString::fromLatin1(pDLL_Error_String(pDLL_Error_Number())),MSG_ERROR);
       // Exit condition after too many failed attempts
       if(bpTimeout==BP_TIMEOUT)
       {
           manageAlert(logFile,"eZ-FET test timeout. Restart test.",MSG_ERROR);
           waitHostTestEndTimer->setProperty("status",STATUS_ERROR);
           waitHostTestEndTimer->stop();
           closeUIF();
           ui->statusIcon->setStyleSheet("border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;background-color: red;");
           enableControls();
           return;
       }
       else
       {
           bpTimeout++;
       }
    }
}


// Load and map MSP430.DLL functions
void MainWindow::loadLibrary(void)
{
    HINSTANCE h430Dll;

    // Get a handle to the DLL module.
    h430Dll = LoadLibrary(TEXT("MSP430.dll"));

    // If the handle is valid, try to get the function address.
    if (h430Dll != NULL)
    {
        pDLL_Initialize = (DLL430CHARPLONGP)GetProcAddress(h430Dll,"MSP430_Initialize");
        pDLL_Close = (DLL430LONG)GetProcAddress(h430Dll,"MSP430_Close");
        pDLL_Identify = (DLL430CHARPLONGLONG)GetProcAddress(h430Dll,"MSP430_Identify");
        pDLL_OpenDevice = (DLL430CHARPCHARPLONGLONGLONG)GetProcAddress(h430Dll,"MSP430_OpenDevice");
        pDLL_GetFoundDevice = (DLL430CHARPLONG)GetProcAddress(h430Dll,"MSP430_GetFoundDevice");
        pDLL_Configure = (DLL430LONGLONG)GetProcAddress(h430Dll,"MSP430_Configure");
        pDLL_VCC = (DLL430LONG)GetProcAddress(h430Dll,"MSP430_VCC");
        pDLL_Reset = (DLL430LONGLONGLONG)GetProcAddress(h430Dll,"MSP430_Reset");
        pDLL_Erase = (DLL430LONGLONGLONG)GetProcAddress(h430Dll,"MSP430_Erase");
        pDLL_Memory = (DLL430LONGCHARPLONGLONG)GetProcAddress(h430Dll,"MSP430_Memory");
        pDLL_Error_Number = (DLL430VOID)GetProcAddress(h430Dll,"MSP430_Error_Number");
        pDLL_Error_String = (CDLL430LONG)GetProcAddress(h430Dll,"MSP430_Error_String");
        pDLL_ProgramFile = (DLL430LPTSTRLONGLONG)GetProcAddress(h430Dll,"MSP430_ProgramFile");
        pDLL_Registers = (DLL430LONGPLONGLONG)GetProcAddress(h430Dll,"MSP430_Registers");
        pDLL_State = (DLL430LONGPLONGLONGP)GetProcAddress(h430Dll,"MSP430_State");
        pDLL_EEM_Init = (DLL430EEMINIT)GetProcAddress(h430Dll,"MSP430_EEM_Init");
        pDLL_EEM_SetBreakpoint = (DLL430WORDPBPPARAMP)GetProcAddress(h430Dll,"MSP430_EEM_SetBreakpoint");
        pDLL_Run = (DLL430LONGLONG)GetProcAddress(h430Dll,"MSP430_Run");
        pDLL_FET_FwUpdate = (DLL430FETFWUPDATE)GetProcAddress(h430Dll,"MSP430_FET_FwUpdate");
    }
    else
    {
        // Error loading DLL
        ui->msgList->addItem("Could not open MSP430.DLL.");

        // Init function pointers -- will throw access error later on
        pDLL_Close = 0;
        pDLL_Identify = 0;
        pDLL_OpenDevice = 0;
        pDLL_GetFoundDevice = 0;
        pDLL_Configure = 0;
        pDLL_VCC = 0;
        pDLL_Reset = 0;
        pDLL_Erase = 0;
        pDLL_Memory = 0;
        pDLL_Error_Number = 0;
        pDLL_Error_String = 0;
        pDLL_ProgramFile = 0;
        pDLL_Registers = 0;
        pDLL_State = 0;
        pDLL_EEM_Init = 0;
        pDLL_EEM_SetBreakpoint = 0;
        pDLL_FET_FwUpdate = 0;
    }
}
