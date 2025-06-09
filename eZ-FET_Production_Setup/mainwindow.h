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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>
#include "qextserialport.h"
#include "MSP430.h"
#include "MSP430_FET.h"
#include "MSP430_Debug.h"

#define CONFIG_FILE          "ez-flasher.cfg"
#define EZFET_DEVICE_NAME    "MSP430F5528"
#define DEVICE_VCC           3600
#define DEVICE_BUFFER_SIZE   68
#define BSL_ADDRESS          0x1000
#define BSL_LOCATIONS        2048
#define BP_ADDRESS           0xF000
#define BP_TIMEOUT           5  //3sec tick, 15s total


// -------------------------------------------------------
// eZ-FET Embedded Test framework
// 0-3: Test 1 [type, result, value)
// 4-7: Test 2 [type, result, value)
// ...
// n: Global pass / fail
#define TEST_ARRAY_LOCATION         0x2400
#define TEST_COUNT                  20
#define TEST_SIZE                   6
#define TEST_ARRAY_LENGTH           (TEST_COUNT*TEST_SIZE+1)
#define TEST_TYPE                   0
#define TEST_RESULT                 1
#define TEST_VALUE_3                2
#define TEST_VALUE_2                3
#define TEST_VALUE_1                4
#define TEST_VALUE_0                5
#define TEST_PASS                   0xAA
#define TEST_FAIL                   0x55

// List of test types
typedef enum testType
{
    // Test over signature
    TEST_OVER = 0,

    // Common tests
    TEST_LOTID,
    TEST_DIEXY,
    TEST_XT2,

    // Host MCU tests
    TEST_VBUS,
    TEST_VOUT,
    TEST_VJTAG,
    TEST_DCDC_MCU,
    TEST_FPGA,
    TEST_UART,
    TEST_STANDBY_CURRENT,

    // Target MCU tests
    TEST_VOLTAGE1,
    TEST_VOLTAGE2,
    TEST_VOLTAGE3,
    TEST_VOLTAGE4,
    TEST_XT,
    TEST_DISPLAY1,
    TEST_DISPLAY2,
    TEST_DISPLAY3,
    TEST_DISPLAY4,
    TEST_LED1,
    TEST_LED2,
    TEST_LED3,
    TEST_LED4,
    TEST_SWITCH1,
    TEST_SWITCH2,
    TEST_SWITCH3,
    TEST_SWITCH4,
    TEST_POWER1,
    TEST_POWER2,
    TEST_POWER3,
    TEST_POWER4,
    TEST_CAPTOUCH1,
    TEST_CAPTOUCH2,
    TEST_CAPTOUCH3,
    TEST_CAPTOUCH4,
    TEST_UART1,
    TEST_UART2,
    TEST_UART3,
    TEST_UART4
} testType_t;

#define TARGET_TEST_TIMEOUT 30

#define MSG_ERROR            0
#define MSG_INFO             1
#define MSG_LOG              2
#define MSG_START            3
#define MSG_OPERATOR         4

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

private slots:

    void loadLibrary();

    // GUI buttons
    void on_selectLogFileButton_clicked();
    void on_enableExpertModeBox_clicked(bool checked);

    void on_selectBslButton_clicked();
    void on_programBslButton_clicked();

    void on_selectHostTestFWButton_clicked();
    void on_programHostTestFWButton_clicked();

    void on_selectHostProdFWButton_clicked();
    void on_programHostProdFWButton_clicked();

    void enableControls();
    void disableControls();
    void on_startButton_clicked();

    // Support functions
    void manageAlert(QFile *f, QString msg, int type);
    void scanForUIF();
    void connectUIF();
    void closeUIF();

    // Host test & programming sequence
    void waitHostTestEndTimeout();


private:
    Ui::MainWindow *ui;
    QTimer *waitHostTestEndTimer, *scanForUIFTimer;
    QFile  *logFile;            // Pointer to logfile
    int bpTimeout;              // Test timeout counter
    STATUS_T status;            // MSP430.DLL status value
    char * ezName;              // Name of the newly programmed eZ-FET
    char* uifName;              // Sticky MSP-FET430UIF name, kept during programming session
    QByteArray uifNameBA, ezNameBA;
    bool testOver;              // TRUE = test is over
    bool testPass;              // TRUE = test has passed
    int testLen;                // Test length in bytes
    char testResult[TEST_ARRAY_LENGTH];
    QString timeStamp;          // Timestamp used for one session
};

#endif // MAINWINDOW_H
