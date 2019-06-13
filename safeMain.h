#ifndef SAFEMAIN_H
#define SAFEMAIN_H

#include <string>
//#include <QGLWidget>
#include <QtGui>
#include <QWidget>

#include "signals_unity_bridge.h"
#include "cwipc_codec/include/api.h"

//#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "TimerHQ.h"
#include "GLVertexBuffers.h"
#include "GLMessages.h"
//#include "framerate.h"
#include "CapturSUB.h"

#ifdef USE_CUDA
    #include <cuda_gl_interop.h>
    #include <cuda.h>
    #include <cudaGL.h>
    #include <cuda_runtime.h>
    #include <../../CUDA/inc/helper_cuda.h>
    #include <../../CUDA/inc/helper_math.h>
#endif

#define AVERAGE 100
#define BUFFERMEM 100 * 1024
#define NUMVERTICES 1
#define RADIANS_TO_DEGREES(radians) ((radians) * (180.0f / M_PI))
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0f * M_PI)
#define CONTROLLER_ZOOM_FACTOR -50.0f;
#define MAX_THREADS 256
#define MAX_X 1024 //8192
#define MAX_Y 1024 //8192
#define NUMPCL 3
#define NUMTHREADS 3


class SafeMain : public QGLWidget
{
    Q_OBJECT

public:
    SafeMain(QWidget *parent = 0);
    ~SafeMain();

    int safeMain();
	void safeMainV31();// int argc, char const* argv[]);

    void setSUBThreadsNumber(int iVal) { iSUBThreadsNumber = iVal; }
    int getSUBThreadsNumber() { return iSUBThreadsNumber; }
    void stopSUBThreads();
    void unStopSUBThreads();
    void setSUBThreads();
    void destroySUBThreads();
    void startSUBThreads();
    void setUseSUBThreads(bool value);

    void setCudaParam();
    void setCudaParamMulti();
    void setVerticesCUDA();
    void setVerticesCUDAMulti();
    void freeVerticesCUDA();
    void allocVerticesCUDA();

    void setCudaParamThreads();
    void setVerticesCUDAThreads(uint uThID);
    void freeVerticesCUDAThreads(uint uThID);
    void allocVerticesCUDAThreads(uint uThID);

    bool getUseCUDA() { return useCUDA; }
    void setUseCUDA(bool bVal) { useCUDA = bVal; }
//	CUgraphicsResource* getPositionCUDABuffer() { return &positionCUDABuffer; }
//	CUgraphicsResource* getPosColorCUDABuffer() { return &posColorCUDABuffer; }
//	CUgraphicsResource* getPositionCUDABufferThread(int iVal) { return &positionCUDABufferTh[iVal]; }
//	CUgraphicsResource* getPosColorCUDABufferThread(int iVal) { return &posColorCUDABufferTh[iVal]; }

    void setInitialized(bool bVal) { initialized = bVal; }
    void setAllocated(bool bVal) { allocated = bVal; }
    void setPLYloaded(bool bVal) { plyloaded = bVal; }
    bool getInitialized() { return initialized; }
    bool getAllocated() { return allocated; }
    bool getPLYloaded() { return plyloaded; }

#ifndef THREADSUB
	CapturSUB* mySUB01;
	CapturSUB* mySUB02;
	CapturSUB* mySUB03;
#endif


public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void animate();
	void setYElev(int elev);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
	void processGeomTh(float depX, float despY, float despZ);
	void processGeom(float depX, float despY, float despZ);
	void processGeomCUDA(float despX, float despY, float despZ);

    void initializeGL();
	void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent* ptEvent) override;
    void keyPressEvent(QKeyEvent *e) override;
	void paintEvent(QPaintEvent *event) override;

    /** Increments the wheel zoom factor */
    void incZoom() {
        m_fViewportZoomFactor = CONTROLLER_ZOOM_FACTOR;
        m_fDistEye += m_fViewportZoomFactor;
    }

    /** Decremets the wheel zoom factor */
    void decZoom() {
        m_fViewportZoomFactor = -CONTROLLER_ZOOM_FACTOR;
        m_fDistEye += m_fViewportZoomFactor;
    }

    void addIncMov();
	void setVars1(int iVal1, int iVal2, int iVal3);


private:
    void normalizeAngle(int *angle);
    void setBuffers();

	float mix(const float &a, const float &b, const float &interp) {
		static constexpr float one = ((float)1);
		return (a * (one - interp)) + (b * interp);
	}
    void cudaInfo();



    int iSUBThreadsNumber;
    int iNumProcessorsQTH;

    int xRot;
    int yRot;
    int zRot;
	int yElev;
    float m_fDistEye;
    bool initialized;                           /// The devices was initialized
    bool allocated;                             /// The memory was allocated
    bool plyloaded;                             /// The file PLY was loaded
    uint drawed;
    float m_fViewportZoomFactor;
    GLVertexBuffers *ViewerVertexBuffer1;
    GLVertexBuffers *ViewerVertexBufferTh[MAX_THREADS];
    bool useCUDA;
    int iBufferID;
    int iBufferIDTh[MAX_THREADS];
    int iMaxMem;
    int filePos;

    int num_elems;
    int iNumBlocks;
    int iFileMaxSize;
    int m_iNumVertices;
    int m_iNumVerticesPerThread;
    int iThreadsNumber;                         /// Number of threads
    int num_elemsTh[MAX_THREADS];

    float moveUp;
    QPoint lastPos;
    float incMov;
    int iAvatar;
	bool bSeeColor;
	bool bOnlyOne;

	FrameInfo* timestampInfo[NUMPCL];
	cwipc_point* mypcl[NUMPCL];
	int iNumPoints[NUMPCL];
	int iNumPointsPrev[NUMPCL][10];
	int iPosPrev;

	QMutex doStopM02;

	GLMessages theGLMessage;
	QPainter painter;

	int iTotalPoints;




#ifdef USE_CUDA
    CUdeviceptr pDevPtr[8];
    size_t  num_bytes[8];
    CUresult gmr;
    //float incMov;
    CUdeviceptr seedmemCU;
    int* seedmemCUbytes;
    ulong ulPosStart;
    ulong ulColStart;

	CUdeviceptr Avatar1memCU;
	int* Avatar1memCUbytes;

    int block_size;
    CUresult error;
    CUdevice dev;
    int deviceCount;
    CUdevice	cuDevice;
    CUcontext   cuContext;
    CUcontext   cuContextMulti;
    CUcontext   cuContextThread;

    CUmodule    cuModule;
    CUfunction  calculateNurbsUVcu;
    CUfunction  calculateNurbs1cu;
    CUfunction  vrtogCU;
    CUfunction  vrtogCU01;
    CUfunction  vrtogCU02;
    CUfunction  vrtogCU03;
    CUfunction  vrtogCU04;
    CUgraphicsResource positionCUDABuffer;
    CUgraphicsResource posColorCUDABuffer;
    CUgraphicsResource indicesCUDABuffer;
    CUgraphicsResource positionCUDABufferTh[MAX_THREADS];
    CUgraphicsResource posColorCUDABufferTh[MAX_THREADS];
    CUgraphicsResource indicesCUDABufferTh[MAX_THREADS];

    //CUgraphicsResource texCoordCUDABuffer;
    //CUgraphicsResource linesColorCUDABuffer;
    //CUgraphicsResource contournCUDABuffer;
    //CUgraphicsResource knotsColorCUDABuffer;

    //CUgraphicsResource imageInCUDABuffer;
    //CUgraphicsResource imageOutCUDABuffer;


    CUdeviceptr debugmemCU0;
    int* debugmemCUbytes0;
    CUdeviceptr debugmemCU1;
    int* debugmemCUbytes1;
    CUdeviceptr debugmemCU2;
    int* debugmemCUbytes2;

    CUdeviceptr debugmemFloatCU0;
    float* debugmemFloatCUbytes0;
    CUdeviceptr debugmemFloatCU1;
    float* debugmemFloatCUbytes1;
    CUdeviceptr debugmemFloatCU2;
    float* debugmemFloatCUbytes2;

    CUdeviceptr debugmemCharCU0;
    char* debugmemCharCUbytes0;
    CUdeviceptr debugmemCharCU1;
    char* debugmemCharCUbytes1;
    CUdeviceptr debugmemCharCU2;
    char* debugmemCharCUbytes2;

    CUdeviceptr nothingmemCU;
    int* nothingmemCUbytes;


    CUgraphicsResource imageOneIn1cu;
    CUgraphicsResource imageOneOut1cu;
    CUtexref TexRefIn;
    CUtexref TexRefOut;
    CUdeviceptr textureIn;
    CUdeviceptr textureOut;
    CUarray srcArray;
    CUarray dstArray;
    CUDA_MEMCPY2D copyParam;
    CUDA_MEMCPY2D getParam;
    CUDA_MEMCPY2D getToHost;
    CUDA_ARRAY_DESCRIPTOR desc;
    CUdeviceptr bufferEffects1cu;
    CUdeviceptr bufferEffects2cu;
    uint sizeCU10;
    uint sizeCU11;
    uint sizeCU12;
    uint sizeCU20;
    uint sizeCU21;
    uint sizeCU22;
    uint sizeCU30;
    uint sizeCU31;
    uint sizeCU32;
#endif


};

#endif // SAFEMAIN_H
