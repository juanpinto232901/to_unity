#include "to_unityt_global.h"
#include "to_unityt.h"
#include <QMessageBox>
#include <QDebug>
#include "CapturSUBThread.h"
#include "CapturSUB.h"


QVector<CapturSUBThread*> theSUBNodes;
/*
//TO_UNITYTSHARED_EXPORT
To_Unityt::To_Unityt()
	: iSUBThreadsNumber(1)
	, iNumProcessorsQTH(1)
	, xRot(0.0)
	, yRot(0.0)
	, zRot(0.0)
	, m_fDistEye(0.0)
	, initialized(false)                           /// The devices was initialized
	, allocated(false)                             /// The memory was allocated
	, plyloaded(false)                             /// The file PLY was loaded
{
	iNumProcessorsQTH = QThread::idealThreadCount();
	qDebug() << QString("iNumProcessorsQTH=%1").arg(iNumProcessorsQTH);
	//	setThreadsNumber(4);// iNumProcessorsQTH - 2);
	setSUBThreadsNumber(iNUMPCL);

	//QMessageBox msgBox;
	//msgBox.setText("The document has been modified.");
	//msgBox.setInformativeText("Do you want to save your changes?");
	//msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	//msgBox.setDefaultButton(QMessageBox::Save);
	//int ret = msgBox.exec();

}
//*/
//****************************************************************************

void init()
{
	iSUBThreadsNumber = 0;
	iNumProcessorsQTH = 1;
	xRot = 0;
	yRot = 0;
	zRot = 0;
	m_fDistEye = 0.0;
	initialized = false;                           /// The devices was initialized
	allocated = false;                             /// The memory was allocated
	plyloaded = false;                             /// The file PLY was loaded
	
	iNumProcessorsQTH = QThread::idealThreadCount();
	qDebug() << QString("initialization    iNumProcessorsQTH=%1").arg(iNumProcessorsQTH);
	iNUMPCL = 0;
	setSUBThreadsNumber(0);

}
//extern "C" TO_UNITYTSHARED_EXPORT 
void stopSUBThreads()
{
//	QMutexLocker locker(&doStopMutex);

	qDebug("stopSUBThreads   Stopping threads ......................... ");
	if (theSUBNodes.size() < 1)return;
	QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
	while (itGeom != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom);
		thNode->stop();
		++itGeom;
	}
	itGeom = theSUBNodes.begin();
	while (itGeom != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom);
//		thNode->terminate();                       not to use
		thNode->wait();
		++itGeom;
	}
}

//extern "C" TO_UNITYTSHARED_EXPORT
void unStopSUBThreads()
{
//	QMutexLocker locker(&doStopMutex);
	qDebug("unStopSUBThreads   unStopping threads ......................... ");
	QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
	while (itGeom != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom);
		thNode->unStop();
		++itGeom;
	}
}

//extern "C" TO_UNITYTSHARED_EXPORT
void startSUBThreads()
{
//	QMutexLocker locker(&doStopMutex);
	qDebug() << QString("startSUBThreads ............... ");
	int iIndex = 0;

	qDebug() << QString("startSUBThreads    theSUBNodes_size = %1").arg(theSUBNodes.size());
	iIndex = 0;
	QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
	while (itGeom != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom);
		thNode->start((QThread::Priority)QThread::LowPriority);

		qDebug() << QString("startSUBThreads  start  iIndex=%1 ").arg(iIndex);
		++itGeom;
		iIndex++;
	}
	/*
	   enum Priority {
			IdlePriority,

			LowestPriority,
			LowPriority,
			NormalPriority,
			HighPriority,
			HighestPriority,

			TimeCriticalPriority,

			InheritPriority
		};
	*/
}

//extern "C" TO_UNITYTSHARED_EXPORT
void setSUBThreads()
{
//	QMutexLocker locker(&doStopMutex);
	int iIndex = 0;
	QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
	while (itGeom != theSUBNodes.end())
	{
		qDebug() << QString("setSUBThreads   iIndex=%1 ").arg(iIndex);
		CapturSUBThread* thNode = (*itGeom);
		QString tx1 = QString("TH%1").arg(iIndex + 1);
//		thNode->theGLMessageTh = &theGLMessage;
		thNode->setName(tx1);
		mypcl[iIndex] = thNode->getPCL();
//		thNode->setID(iIndex);
		++itGeom;
		iIndex++;
	}


	qDebug() << QString("setSUBThreads  MAXQTHREADS= %1").arg(getSUBThreadsNumber());
	int i = 0;
	QVector<CapturSUBThread*>::iterator itGeom1 = theSUBNodes.begin();
	while (itGeom1 != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom1);
		//		qDebug() << QString("name=%1 worker=%2").arg(thNode->getName()).arg(thNode->getWorker()->getName());
		qDebug() << QString("name=%1 ").arg(thNode->getName());
		qDebug() << QString("uID=%1 url=%2 user=%3 ").arg(thNode->getID()).arg(thNode->getUrl()).arg(thNode->getUserName());
		qDebug() << QString("setSUBThreads  i=%1  mypcl_size=%2 mypcl_pointer=%3").arg(i).arg(_msize(mypcl[i])).arg((qlonglong)mypcl[i]);
		++itGeom1;
		i++;
	}
}

//extern "C" TO_UNITYTSHARED_EXPORT
void destroySUBThreads()
{
//	QMutexLocker locker(&doStopMutex);
	QVector<CapturSUBThread*>::iterator it = theSUBNodes.begin();
	while (it != theSUBNodes.end())
	{
		it = theSUBNodes.erase(it);
	}
	qDebug() << QString("VRtogetherWidget::destroySUBThreads size=%1 ==== ").arg(theSUBNodes.size());

}

//extern "C" TO_UNITYTSHARED_EXPORT
void setUseSUBThreads(bool value)
{

	//#define USE_CUDA_TREADS
	//#define USE_CUDA_NOTH
	//#define USE_CPU

//	doneCurrent();
	if (!value) {
		stopSUBThreads();
	}
	//	makeCurrent();
	if (value) {
		destroySUBThreads();
		setSUBThreads();
		startSUBThreads();
	}

}

//*****************************************************************************


//extern "C" TO_UNITYTSHARED_EXPORT
void removeUserbyID(int uID)
{
//	QMutexLocker locker(&doStopMutex);
	int ind = 0;
	QVector<CapturSUBThread*>::iterator it = theSUBNodes.begin();
	while (it != theSUBNodes.end())
	{
		if(uID == ind)it = theSUBNodes.erase(it);
		else ++it;
		ind++;
	}
	qDebug() << QString("removeUserbyID  size=%1 ======== uID=%2 ========= iSUBThreadsNumber=%3").arg(theSUBNodes.size()).arg(uID).arg(iSUBThreadsNumber);

	iSUBThreadsNumber--;
}

//extern "C" TO_UNITYTSHARED_EXPORT
void removeUserbyName(char* UserName)
{
//	QMutexLocker locker(&doStopMutex);
	int ind = 0;
	QVector<CapturSUBThread*>::iterator it = theSUBNodes.begin();
	while (it != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*it);
		if (strcmp(UserName, thNode->getUserName()) == 0)it = theSUBNodes.erase(it);
		else ++it;
		ind++;
	}
	qDebug() << QString("removeUserbyName  size=%1 =========  UserName=%2 ========= iSUBThreadsNumber=%3").arg(theSUBNodes.size()).arg(UserName).arg(iSUBThreadsNumber);

	iSUBThreadsNumber--;
}

//extern "C" TO_UNITYTSHARED_EXPORT
void addUser(int uID, char* UrlName, char* UserName)
{
//	QMutexLocker locker(&doStopMutex);
	qDebug() << QString("addUser  **********************  uID=%1  UrlName=%2  UserName=%3 ").arg(uID).arg(UrlName).arg(UserName);


	CapturSUBThread* thNode = 0;
	thNode = new CapturSUBThread;
	thNode->setID((int)uID);
	thNode->setUrl((char*)UrlName);
	thNode->setUserName((char*)UserName);

	theSUBNodes.append(thNode);


	iSUBThreadsNumber++;
	qDebug() << QString("addUser  out  uID=%1 url=%2 user=%5   iSUBThreadsNumber=%3  theSUBNodes_size=%4 ").arg(thNode->getID()).arg(thNode->getUrl()).arg(iSUBThreadsNumber).arg(theSUBNodes.size()).arg(thNode->getUserName());
}

//extern "C" TO_UNITYTSHARED_EXPORT
void setSUBThreadsNumber(int iVal) { iSUBThreadsNumber = iVal;  qDebug() << QString("setter   iSUBThreadsNumber=%1").arg(iSUBThreadsNumber); }
//extern "C" TO_UNITYTSHARED_EXPORT
int getSUBThreadsNumber() { qDebug() << QString("getter  iSUBThreadsNumber=%1").arg(iSUBThreadsNumber); return iSUBThreadsNumber; }


void printUsers()
{
//	QMutexLocker locker(&doStopMutex);
	qDebug() << QString("****************************************************************");
	qDebug() << QString("************************* Print Users ************************* ");
	qDebug() << QString("****************************************************************");
	int i = 0;
	QVector<CapturSUBThread*>::iterator itGeom1 = theSUBNodes.begin();
	while (itGeom1 != theSUBNodes.end())
	{
		CapturSUBThread* thNode = (*itGeom1);
		if (thNode) {
			//		qDebug() << QString("name=%1 worker=%2").arg(thNode->getName()).arg(thNode->getWorker()->getName());
			qDebug() << QString("name=%1 uID=%2 url=%3 ").arg(thNode->getName()).arg(thNode->getID()).arg(thNode->getUrl());
			qDebug() << QString("uID=%1 url=%2 user=%3 ").arg(thNode->getID()).arg(thNode->getUrl()).arg(thNode->getUserName());
			if (mypcl[i]) {
				qDebug() << QString("setSUBThreads  i=%1  mypcl_size=%2 mypcl_pointer=%3").arg(i).arg(_msize(mypcl[i])).arg((qlonglong)mypcl[i]);
			}
		}
		++itGeom1;
		i++;
	}
	qDebug() << QString("****************************************************************");
	qDebug() << QString("****************************************************************");
	qDebug() << QString("****************************************************************");

}