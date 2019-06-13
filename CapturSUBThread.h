#ifndef CAPTURSUBTHREAD_H
#define CAPTURSUBTHREAD_H


#include "signals_unity_bridge.h"
#include "cwipc_codec/include/api.h"
#include <QtCore/QThread>
#include <QMutex>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include "TimerHQ.h"
#include "GLMessages.h"

#define DELAY 40
#define DELAYINC 5


class VRtogetherWidget;

class CapturSUBThread : public QThread
{

public:
	GLMessages* theGLMessageTh;


	CapturSUBThread();
	~CapturSUBThread();
	void run();

    void setName(QString tTxt){ txt1 = QString(tTxt); }
    QString getName() { return txt1; }
    void setID(uint uTmp){ uID = uTmp; }
    uint getID() { return uID; }
	void setUrl(char* sVal) { url = sVal; }
	char* getUrl() { return url; }
	void setUserName(char* sVal) { userName = sVal; }
	char* getUserName() { return userName; }

	void stop();
    void unStop();

	typedef sub_handle*(*sub_create)(const char* name);
	sub_create func_sub_create;
	//	sub_handle* func_sub_create;

	// Destroys a pipeline. This frees all the resources.
	typedef void(*sub_destroy)(sub_handle* h);
	sub_destroy func_sub_destroy;

	// Returns the number of compressed streams.
	typedef int(*sub_get_stream_count)(sub_handle* h);
	sub_get_stream_count func_sub_get_stream_count;

	// Plays a given URL.
	typedef bool(*sub_play)(sub_handle* h, const char* URL);
	sub_play func_sub_play;

	// Copy the next received compressed frame to a buffer.
	// Returns: the size of compressed data actually copied,
	// or zero, if no frame was available for this stream.
	// If 'dst' is null, the frame will not be dequeued, but its size will be returned.
	typedef size_t(*sub_grab_frame)(sub_handle* h, int streamIndex, uint8_t* dst, size_t dstLen, FrameInfo* info);
	sub_grab_frame func_sub_grab_frame;

	void setPCL(cwipc_point* vVal) { 
		QMutexLocker locker(&doStopMutex);
		mypcl = vVal; }
	cwipc_point* getPCL() { 
		QMutexLocker locker(&doStopMutex);
		return mypcl; }
	void setNumPoints(int iTmp) {
		QMutexLocker locker(&doStopMutex);
		iNumPoints = iTmp; }
	uint getNumPoints() {
		QMutexLocker locker(&doStopMutex);
		return iNumPoints; }
	FrameInfo* getInfo() {
		QMutexLocker locker(&doStopMutex);
		return timestampInfo;
	}



protected:
    QString txt1;
    uint uID;
	char* url;
	char* userName;

    volatile bool doStop;
    QMutex doStopMutex;
	QMutex doStopM01;
	FrameInfo* info;
	uint8_t* dst;
	cwipc_point* mypcl;
	int iNumPoints;
	Timer myTimer;
	FrameInfo* timestampInfo;

	int iNumReads;
	bool bInitialized;
	int iDelay;
	bool bDirInc;// true=incrementing  false=decreasing

	bool bStoped;
	int iReaded;
};

#endif // CAPTURSUBTHREAD_H
