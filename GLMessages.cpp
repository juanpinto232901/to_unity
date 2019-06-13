#include "GLMessages.h"

GLMessages::GLMessages()
    : averageFPS1(0.0)
    , averageFPS2(0.0)
    , averagePaint1(0.0)
    , averagePaint2(0.0)
    , averageCPU1(0.0)
    , averageCPU2(0.0)
    , averageCUDA1(0.0)
    , averageCUDA2(0.0)
    , averageOpenCL1(0.0)
    , averageOpenCL2(0.0)
    , averageFlush1(0.0)
    , averageFlush2(0.0)
    , averageAnimate1(0.0)
    , averageAnimate2(0.0)
    , ticdraw(0)
	, ticdraw2(0)
	, iSizePCL1comp(0)
	, iSizePCL2comp(0)
	, iSizePCL3comp(0)
	, iReadedPCL1comp(0)
	, iReadedPCL2comp(0)
	, iReadedPCL3comp(0)
	, iUncomp1(0)
	, iUncomp2(0)
	, iUncomp3(0)
	, iAverageSizePCL1comp(0)
	, iAverageSizePCL2comp(0)
	, iAverageSizePCL3comp(0)
	, iAverageReadedPCL1comp(0)
	, iAverageReadedPCL2comp(0)
	, iAverageReadedPCL3comp(0)
	, iAverageUncomp1(0)
	, iAverageUncomp2(0)
	, iAverageUncomp3(0)

{
//    commonParams = &CommonStruct::getInstance();

    qDebug("GLMessage Constructor");
	iSizePCL1comp = 0;
	iSizePCL2comp = 0;
	iSizePCL3comp = 0;
	iReadedPCL1comp = 0;
	iReadedPCL2comp = 0;
	iReadedPCL3comp = 0;
	iUncomp1 = 0;
	iUncomp2 = 0;
	iUncomp3 = 0;

}

void GLMessages::drawGLText(QPainter *painter, float fps, float fpscap, int iNumPolygons, int iNumCapFrame)
{
	QMutex doStopMutex01;

    if (fps > 0.0f) {
        uint colorTextAverage = getColorTextGL();

		QString sNum = QString("%1").arg(iNumPolygons, 9, 10, QLatin1Char(' '));
		QString sRnum = QString(sNum.right(3));
		QString sMnum = QString(sNum.right(6).left(3));
		QString sLnum = QString(sNum.left(3));
		QString oneStr01 = QString("%3.%2.%1   %4").arg(sRnum).arg(sMnum).arg(sLnum).arg(QString::number(iNumPolygons));
		QString str = oneStr01 + QLatin1String(" points, ");

		iSizePCL1comp = 0;
		iSizePCL2comp = 0;
		iSizePCL3comp = 0;

//		qDebug() << str;
        averageFPS1 += 1.0 / (tPaint.getElapsedTimeInMicroSec() / 1000000.0);
        averagePaint1 += tPaint.getElapsedTimeInMicroSec();
        averageCPU1 += tCPU.getElapsedTimeInMicroSec();
        averageCUDA1 += tCUDA.getElapsedTimeInMicroSec();
        averageOpenCL1 += tOpenCL.getElapsedTimeInMicroSec();
        //averageFlush1 += tFlush.getElapsedTimeInMicroSec();
        //averageAnimate1 += tAnimate.getElapsedTimeInMicroSec();
        //averageCapture1 += tCapture.getElapsedTimeInMicroSec();
		averagePaint2 = averagePaint1;
//		qDebug() << QString("GLMessages::drawGLText    iSizePCL1comp=%1 iAverageSizePCL1comp=%2 ticdraw2=%3 20 ").arg(iSizePCL1comp).arg(iAverageSizePCL1comp).arg(ticdraw2);


		if (ticdraw == AVERAGE) {
			averageFPS2 = averageFPS1 / (float)AVERAGE;
			averagePaint2 = averagePaint1 / (float)AVERAGE;
			averageCPU2 = averageCPU1 / (float)AVERAGE;
			averageCUDA2 = averageCUDA1 / (float)AVERAGE;
			averageOpenCL2 = averageOpenCL1 / (float)AVERAGE;
			//averageFlush2 = averageFlush1 / (float)AVERAGE;
			//averageAnimate2 = averageAnimate1 / (float)AVERAGE;
			//averageCapture2 = averageCapture1 / (float)AVERAGE;
		}
		if (ticdraw2 >= AVERAGE2) {
			iAverageSizePCL1comp = iSizePCL1comp / AVERAGE2;
//			qDebug() << QString("GLMessages::drawGLText    iAverageSizePCL1comp=%1 ").arg(iAverageSizePCL1comp);
			iAverageSizePCL2comp = iSizePCL2comp / AVERAGE2;
			iAverageSizePCL3comp = iSizePCL3comp / AVERAGE2;
			iAverageReadedPCL1comp = iReadedPCL1comp / AVERAGE2;
			iAverageReadedPCL2comp = iReadedPCL2comp / AVERAGE2;
			iAverageReadedPCL3comp = iReadedPCL3comp / AVERAGE2;
			iAverageUncomp1 = iUncomp1 / AVERAGE2;
			iAverageUncomp2 = iUncomp2 / AVERAGE2;
			iAverageUncomp3 = iUncomp3 / AVERAGE2;
//			qDebug() << QString("GLMessages::drawGLText  ***  iSizePCL1comp=%1 iAverageSizePCL1comp=%2 ticdraw2=%3 20 ").arg(iSizePCL1comp).arg(iAverageSizePCL1comp).arg(ticdraw2);
		}
//        str += " " + QString::number(fpscap) + QLatin1String("  fps capture frame");
        str += "\n" + QString::number(fps) + QLatin1String("  fps frame rate");
        str += "\n" + QString::number(averageFPS2) + QLatin1String("  fps paint");
        str += "\n" + QString::number(averagePaint2) + QLatin1String(" us tPaint");
        str += "\n" + QString::number(averageCPU2) + QLatin1String(" us tCPU");
        str += "\n" + QString::number(averageCUDA2) + QLatin1String(" us tCUDA");
        str += "\n" + QString::number(averageOpenCL2) + QLatin1String(" us VBO process");
		str += "\n";
		doStopMutex01.lock();
		str += "\n" + QString::number(iAverageSizePCL1comp) + QLatin1String("   ") + QString::number(iAverageReadedPCL1comp) + QLatin1String("   ") + QString::number(iAverageUncomp1) + QString("   redandblack");
		str += "\n" + QString::number(iAverageSizePCL2comp) + QLatin1String("   ") + QString::number(iAverageReadedPCL2comp) + QLatin1String("   ") + QString::number(iAverageUncomp2) + QString("   longdress");
		str += "\n" + QString::number(iAverageSizePCL3comp) + QLatin1String("   ") + QString::number(iAverageReadedPCL3comp) + QLatin1String("   ") + QString::number(iAverageUncomp3) + QString("   loot");
		doStopMutex01.unlock();

		//str += "\n" + QString::number(averageFlush2) + QLatin1String(" us tFlush");
        //str += "\n" + QString::number(averageAnimate2) + QLatin1String(" us tAnimate");
        //str += "\n" + QString::number(averageCapture2) + QLatin1String(" us tCapture");
        if(colorTextAverage==0)painter->setPen(Qt::white);
        if(colorTextAverage==1)painter->setPen(Qt::black);
        if(colorTextAverage==2)painter->setPen(Qt::darkGray);
        if(colorTextAverage==3)painter->setPen(Qt::gray);
        if(colorTextAverage==4)painter->setPen(Qt::lightGray);
        if(colorTextAverage==5)painter->setPen(Qt::red);
        if(colorTextAverage==6)painter->setPen(Qt::green);
        if(colorTextAverage==7)painter->setPen(Qt::blue);
        if(colorTextAverage==8)painter->setPen(Qt::cyan);
        if(colorTextAverage==9)painter->setPen(Qt::magenta);
        if(colorTextAverage==10)painter->setPen(Qt::yellow);
        if(colorTextAverage==11)painter->setPen(Qt::darkRed);
        if(colorTextAverage==12)painter->setPen(Qt::darkGreen);
        if(colorTextAverage==13)painter->setPen(Qt::darkBlue);
        if(colorTextAverage==14)painter->setPen(Qt::darkCyan);
        if(colorTextAverage==15)painter->setPen(Qt::darkMagenta);
        if(colorTextAverage==16)painter->setPen(Qt::darkYellow);
//        if(iNumCapFrame > 1)painter->setPen(Qt::red);
		QRect rect1 = QRect(0, 0, 400, 300);
        painter->drawText(rect1, str);
//        white,
//        black,
//        darkGray,
//        gray,
//        lightGray,
//        red,
//        green,
//        blue,
//        cyan,
//        magenta,
//        yellow,
//        darkRed,
//        darkGreen,
//        darkBlue,
//        darkCyan,
//        darkMagenta,
//        darkYellow,

    }
//    if(iNumCapFrame > 0)qDebug() << QString("iNumCapFrame=%1 iNumFrame=%2").arg(iNumCapFrame).arg(iNumFrame);
    ticdraw++;
//	qDebug() << QString("ticdraw=%1  ticdraw2=%2 ").arg(ticdraw).arg(ticdraw2);
	if (ticdraw > AVERAGE) {
		ticdraw = 1;
		averageFPS1 = 0.0;
		averagePaint1 = 0.0;
		averageCPU1 = 0.0;
		averageCUDA1 = 0.0;
		averageOpenCL1 = 0.0;
		//averageFlush1 = 0.0;
		//averageAnimate1 = 0.0;
		//averageCapture1 = 0.0;
	}
	if (ticdraw2 >= AVERAGE2) {
		ticdraw2 = 0;
		iSizePCL1comp = 0;
		iSizePCL2comp = 0;
		iSizePCL3comp = 0;
		iReadedPCL1comp = 0;
		iReadedPCL2comp = 0;
		iReadedPCL3comp = 0;
		iUncomp1 = 0;
		iUncomp2 = 0;
		iUncomp3 = 0;
    }
	ticdraw2++;

}
