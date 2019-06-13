#ifndef GLMESSAGES_H
#define GLMESSAGES_H

#include <QtWidgets/QWidget>

#include <QtDebug>
#include <QMutex>
//#include "../BezierDefines.h"
#include <QtCore/qtimer.h>
#include <QtCore/qdatetime.h>
//#include <QtGui/qpainter.h>
#include <QPainter>
#include "TimerHQ.h"
//#include "../DataStructs/CommonStruct.h"

#define AVERAGE 100
#define AVERAGE2 20

class GLMessages : public QWidget
{
public:
    GLMessages();

    void drawGLText(QPainter* painter, float fps, float fpscap, int iNumPolygons, int iNumCapFrame);
    void startFlush() { tFlush.start(); }
    void stopFlush() { tFlush.stop(); }
    void startPaint() { tPaint.start(); }
    void stopPaint() { tPaint.stop(); }
    void startDevices(){
        tCPU.start(); tCPU.stop();
        tCUDA.start(); tCUDA.stop();
        tOpenCL.start(); tOpenCL.stop();
    }
    void startAnimate() { tAnimate.start(); }
    void stopAnimate() { tAnimate.stop(); }
    void startCapture() { tCapture.start(); }
    void stopCapture() { tCapture.stop(); }
    void startAnimatesCaptures()
    {
        tCapture.start(); tCapture.stop();
        tAnimate.start(); tAnimate.stop();
    }
    void startCPU() { tCPU.start(); }
    void stopCPU() { tCPU.stop(); }
    void startCUDA() { tCUDA.start(); }
    void stopCUDA() { tCUDA.stop(); }
    void startOpenCL() { tOpenCL.start(); }
    void stopOpenCL() { tOpenCL.stop(); }
	void setColorTextGL(uint uVal) { colorTextAverage = uVal; }
	uint getColorTextGL() { return colorTextAverage; }
	double getTimeMicrosegCPU() { return tCPU.getElapsedTimeInMicroSec(); }
	double getTimeMilisegCPU() { return tCPU.getElapsedTimeInMilliSec(); }
	double getTimeSegCPU() { return tCPU.getElapsedTimeInSec(); }

	//void setMsg01(QString msg1);
	//void setMsg02(QString msg1);
	//void setMsg03(QString msg1);

	void setSizePCL1comp(int iVal) { iSizePCL1comp += iVal; }
	void setSizePCL2comp(int iVal) { iSizePCL2comp += iVal; }
	void setSizePCL3comp(int iVal) { iSizePCL3comp += iVal; }
	void setReadedPCL1comp(int iVal) { iReadedPCL1comp += iVal; }
	void setReadedPCL2comp(int iVal) { iReadedPCL2comp += iVal; }
	void setReadedPCL3comp(int iVal) { iReadedPCL3comp += iVal; }
	void setUncomp1(int iVal) { iUncomp1 += iVal; }
	void setUncomp2(int iVal) { iUncomp2 += iVal; }
	void setUncomp3(int iVal) { iUncomp3 += iVal; }
	void setAvatar1Name(QString sVal) { /*Avatar1Name = QString(sVal);*/ }
	void setAvatar2Name(QString sVal) { /*Avatar2Name = QString(sVal);*/ }
	void setAvatar3Name(QString sVal) { /*Avatar3Name = QString(sVal);*/ }


protected:
    float averageFPS1;
    float averageFPS2;
    Timer tCPU;
    float averageCPU1;
    float averageCPU2;
    Timer tCUDA;
    float averageCUDA1;
    float averageCUDA2;
    Timer tOpenCL;
    float averageOpenCL1;
    float averageOpenCL2;
    Timer tPaint;
    float averagePaint1;
    float averagePaint2;
    Timer tFlush;
    float averageFlush1;
    float averageFlush2;
    Timer tAnimate;
    float averageAnimate1;
    float averageAnimate2;
    Timer tCapture;
    float averageCapture1;
    float averageCapture2;

    uint ticdraw;
	uint ticdraw2;

	uint colorTextAverage;                      /// OpenGL Text color for messages

	int iSizePCL1comp;
	int iSizePCL2comp;
	int iSizePCL3comp;
	int iReadedPCL1comp;
	int iReadedPCL2comp;
	int iReadedPCL3comp;
	int iUncomp1;
	int iUncomp2;
	int iUncomp3;

	int iAverageSizePCL1comp;
	int iAverageSizePCL2comp;
	int iAverageSizePCL3comp;
	int iAverageReadedPCL1comp;
	int iAverageReadedPCL2comp;
	int iAverageReadedPCL3comp;
	int iAverageUncomp1;
	int iAverageUncomp2;
	int iAverageUncomp3;

	QString Avatar1Name;
	QString Avatar2Name;
	QString Avatar3Name;
};

#endif // GLMESSAGES_H
