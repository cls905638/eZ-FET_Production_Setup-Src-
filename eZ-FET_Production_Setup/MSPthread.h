#ifndef MSPTHREAD_H
#define MSPTHREAD_H

#include <QThread>
#include "MSP430_FET.h"

#define FET_FW_UPDATE   0
#define FW_PROGRAM      1

class MSPThread : public QThread
{
    Q_OBJECT
public:
    explicit MSPThread(QObject *parent = 0);

protected:
    void run();

signals:
    
public slots:
    
};

#endif // MSPTHREAD_H
