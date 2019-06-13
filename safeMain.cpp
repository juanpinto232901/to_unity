#include <iostream>
#include <fstream>

#include <Windows.h>
#include <cstdio>
#include <string>
#include <exception>
#include <functional>
#include <stdexcept>
#include "signals_unity_bridge.h"
#include "TimerHQ.h"
#include <sdl\include\SDL.h>
#include "cwipc_codec\include\api.h"
#include "safeMain.h"
#include <QDebug>
#include <QMouseEvent>
#include <qtimer.h>

#include "CapturSUBThread.h"
#include "CapturSUB.h"


#undef main

using namespace std;

#define IMPORT(name) ((decltype(name)*)SDL_LoadFunction(lib, # name))

QVector<CapturSUBThread*> theSUBNodes;

SafeMain::SafeMain(QWidget *parent)
    : useCUDA(false)
    , iSUBThreadsNumber(1)
    , iNumProcessorsQTH(1)

    , xRot(0.0)
    , yRot(0.0)
    , zRot(0.0)
    , m_fDistEye(0.0)
    , initialized(false)                           /// The devices was initialized
    , allocated(false)                             /// The memory was allocated
    , plyloaded(false)                             /// The file PLY was loaded
    , drawed(0)
    , m_fViewportZoomFactor(0.0)
    , ViewerVertexBuffer1(0)
    , iBufferID(0)
    #ifdef USE_CUDA
        , block_size(16)
        , positionCUDABuffer(0)
        , posColorCUDABuffer(0)
        , indicesCUDABuffer(0)
    #endif
    , iMaxMem(0)
    , filePos(0)
    , num_elems(0)
    , iNumBlocks(0)
    , iFileMaxSize(0)
    , m_iNumVertices(0)
    , m_iNumVerticesPerThread(0)
    , iThreadsNumber(0)                         /// Number of threads
    , moveUp(0.0)
    , incMov(0.0)
    , iAvatar(0xffffffff)
	, bSeeColor(true)
	, yElev(0)
	, bOnlyOne(false)
	, iPosPrev(0)
#ifndef THREADSUB
	, mySUB01(0)
	, mySUB02(0)
	, mySUB03(0)
#endif
{
    qDebug() << QString("SafeMain  constructor .......... ");
	setAutoFillBackground(false);

	setFocus();

    for(int i=0; i<MAX_THREADS; i++)
    {
        ViewerVertexBufferTh[i] = 0;
        iBufferIDTh[i] = 0;
        num_elemsTh[i] = 0;
    }

#ifndef THREADSUB
	m_fDistEye = -160;
	yElev = 100;
#else
//	m_fDistEye = 150;
	m_fDistEye = 700.0;
//	m_fDistEye = 3150.0;
	yElev = -200;
#endif

	iAvatar = 0xffffffff;
	bOnlyOne = false;
	iPosPrev = 0;


    iNumProcessorsQTH = QThread::idealThreadCount();
    qDebug() << QString("iNumProcessorsQTH=%1").arg(iNumProcessorsQTH);
//	setThreadsNumber(4);// iNumProcessorsQTH - 2);
    setSUBThreadsNumber(NUMPCL);
	iThreadsNumber = NUMPCL;

    cudaInfo();

//    safeMainV31();
#ifdef THREADSUB
    setUseSUBThreads(true);
#else
	mySUB01 = new CapturSUB;
	mySUB02 = new CapturSUB;
	mySUB03 = new CapturSUB;
#endif

//#ifdef CONGL
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(20);
//#endif
}

SafeMain::~SafeMain()
{
#ifndef THREADSUB
	mySUB01->read_end(0);
	mySUB02->read_end(1);
	mySUB03->read_end(2);
#endif
}

void* safeImport(void* lib, const char* name)
{
	auto r = SDL_LoadFunction(lib, name);

	if (!r)
		throw runtime_error("Symbol not found: " + string(name));

	return r;
}

//#define IMPORT(name) ((decltype(name)*)safeImport(lib, # name))

void SafeMain::safeMainV31()//int argc, char const* argv[])
{
	//if (argc != 3)
	//	throw runtime_error("Usage: app.exe <signals-unity-bridge.dll> [media url]");

	//const string libraryPath = argv[1];
	//const string mediaUrl = argv[2];
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
		throw runtime_error(string("Unable to initialize SDL: ") + SDL_GetError());

	const string libraryPath = "signals-unity-bridge.dll";
	const string mediaUrl = "http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd";

	qDebug() << QString("libraryPath=%1  mediaUrl=%2").arg(libraryPath.c_str()).arg(mediaUrl.c_str());


	auto lib = SDL_LoadObject(libraryPath.c_str());

	if (!lib)
		throw runtime_error("Can't load '" + libraryPath + "': " + SDL_GetError());

	auto func_sub_create = IMPORT(sub_create);
	auto func_sub_play = IMPORT(sub_play);
	auto func_sub_destroy = IMPORT(sub_destroy);
	auto func_sub_grab_frame = IMPORT(sub_grab_frame);
	auto func_sub_get_stream_count = IMPORT(sub_get_stream_count);

	auto handle = func_sub_create("MyMediaPipeline");

	func_sub_play(handle, mediaUrl.c_str());

	if (func_sub_get_stream_count(handle) == 0)
		throw runtime_error("No streams found");

	vector<uint8_t> buffer;

	for (int i = 0; i < 100000; ++i)
	{
		FrameInfo info{};
		buffer.resize(1024 * 1024 * 10);
		auto size = func_sub_grab_frame(handle, 0, buffer.data(), buffer.size(), &info);
		buffer.resize(size);

		if (size == 0)
			SDL_Delay(100);
		else
		{
			printf("Frame: % 5d bytes, t=%.3f [", (int)size, info.timestamp / 1000.0);

			for (int k = 0; k < (int)buffer.size(); ++k)
			{
				if (k == 8)
				{
					printf(" ...");
					break;
				}

				printf(" %.2X", buffer[k]);
			}

			printf(" ]\n");
		}
	}

	func_sub_destroy(handle);
	SDL_UnloadObject(lib);
}



int SafeMain::safeMain()
{
//  if(argc != 2 && argc != 3)
//    throw runtime_error("Usage: app.exe <signals-unity-bridge.dll> [media url]");

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    throw runtime_error(string("Unable to initialize SDL: ") + SDL_GetError());

  const string libraryPath = "signals-unity-bridge.dll";
  const string mediaUrl = "http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd";

  qDebug() << QString("libraryPath=%1  mediaUrl=%2").arg( libraryPath.c_str() ).arg(mediaUrl.c_str());


  auto lib = SDL_LoadObject("D:/i2cat_vrtogether/v30_stable_qt5/getSUB/w64/signals-unity-bridge.dll");// libraryPath.c_str());
  if (!lib)fprintf(stdout, "Error=%s", SDL_GetError());
  auto func_sub_create = IMPORT(sub_create);
  auto func_sub_play = IMPORT(sub_play);
  auto func_sub_destroy = IMPORT(sub_destroy);
  auto func_sub_grab_frame = IMPORT(sub_grab_frame);

  Timer myTimer;
  FrameInfo* info = 0;
  info = (FrameInfo*)malloc(sizeof(FrameInfo));
  int streamIndex = 0;
  uint8_t* dst = 0;

  auto handle = func_sub_create("MyMediaPipeline");
//  struct sub_handle hhh = func_create("MyMediaPipeline");

  func_sub_play(handle, mediaUrl.c_str());

  size_t iSize = 0;//(size_t)func_sub_grab_frame;

  size_t  iSizeB = 1048576 * 1;
  dst = (uint8_t*)malloc(iSizeB);

  cwipc_point* mypcl;
  mypcl = (cwipc_point*)malloc(iSizeB * 2 * sizeof(cwipc_point));

  //qDebug() << QString("dst=%1 mypcl=%2  cwipc_point=%3 ").arg(_msize(dst)).arg(_msize(mypcl)).arg(sizeof(cwipc_point));

  for (int n = 0; n < 200; n++)
  {
	  myTimer.start();
	  //fprintf(stdout, "\n\nn= %d\n", (int)n);
	  Sleep(30);


	  iSize = 1048576;
	  streamIndex = 0;
	  iSize = (size_t)func_sub_grab_frame(handle, streamIndex, 0, 0, info);
	  if (iSize > 0)
	  {
			//fprintf(stdout, "iSize= %d\n", (int)iSize);
			//fprintf(stdout, "dst_size= %d  handle=%d iSize=%d info=%d \n", (unsigned int)_msize(dst), (unsigned int)handle, (unsigned int)iSize, (unsigned int)_msize(info));
			iSize = (size_t)func_sub_grab_frame(handle, streamIndex, dst, iSize, info);
			//for (int i = 0; i < 20; i++) {
   //             qDebug() << QString( "i=%1 buff=%2  %3   ").arg( i).arg( dst[i]).arg( QString::number(dst[i], 16));
			//}
			//
			// Uncompress
			//
			cwipc_decoder *decoder = cwipc_new_decoder();
			if (decoder == NULL) {
                qDebug() << QString("%1: Could not create decoder \n").arg(libraryPath.c_str());
				return 1;
			}
			decoder->feed((void*)dst, iSize);
//			free((uint8_t *)dst); // After feed() we can free the input buffer

			bool ok = decoder->available(true);
			if (!ok) {
                qDebug() << QString("%1: Decoder did not create pointcloud  \n").arg(libraryPath.c_str());
				return 1;
			}
			cwipc *pc = decoder->get();
			if (pc == NULL) {
                qDebug() << QString("%1: Decoder did not return cwipc \n").arg(libraryPath.c_str());
				return 1;
			}

			decoder->free(); // We don't need the encoder anymore
            //qDebug() << "Decoded successfully, " << pc->get_uncompressed_size(CWIPC_POINT_VERSION) << " bytes (uncompressed)";

			int isizepcl = pc->get_uncompressed_size(CWIPC_POINT_VERSION);
			pc->copy_uncompressed(mypcl, isizepcl);

			//
			// Save pointcloud file
			//
			//if (cwipc_write(argv[2], pc, NULL) < 0) {
			//	std::cerr << argv[0] << ": Error writing PLY file " << argv[2] << std::endl;
			//	return 1;
			//}

			pc->free(); // We no longer need to pointcloud

			//for (int i = 0; i < 50; i++) {
   //             qDebug() << QString("i=%1 pcl=%2 %3 %4  rgb=%5 %6 %7 ").arg( i).arg( mypcl[i].x).arg( mypcl[i].y).arg( mypcl[i].z).arg( mypcl[i].r).arg( mypcl[i].g).arg( mypcl[i].b);
			//}

	  }

	  myTimer.stop();
	  double dTime = myTimer.getElapsedTimeInMilliSec();
	  fprintf(stdout, "............................................... time=%f ... ", dTime);
  }

  free(dst);
  dst = 0;
  free(mypcl);
  mypcl = 0;
  func_sub_destroy(handle);
  SDL_UnloadObject(lib);

  SDL_Quit();
  //*/
    return 1;
}

//****************************************************************************

void SafeMain::stopSUBThreads()
{
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
        thNode->wait();
        ++itGeom;
    }
}

void SafeMain::unStopSUBThreads()
{
    qDebug("unStopSUBThreads   unStopping threads ......................... ");
    QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
    while (itGeom != theSUBNodes.end())
    {
        CapturSUBThread* thNode = (*itGeom);
        thNode->unStop();
        ++itGeom;
    }
}

void SafeMain::startSUBThreads()
{
    qDebug() << QString("startSUBThreads ............... ");
    int iIndex = 0;

    QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
    while (itGeom != theSUBNodes.end())
    {
        //uint sz3 = sz2;
        //if (szrest >= (uint)iIndex)sz3++;

        qDebug() << QString("VRtogetherWidget::startSUBThreads   iIndex=%1 ").arg(iIndex);
        CapturSUBThread* thNode = (*itGeom);
        //thNode->setLimit(sz3);
        //thNode->setCount(0);
        //thNode->setMethod(0);
        thNode->setID(iIndex);
        ++itGeom;
        iIndex++;
    }
    iIndex = 0;
    itGeom = theSUBNodes.begin();
    while (itGeom != theSUBNodes.end())
    {
        CapturSUBThread* thNode = (*itGeom);
        thNode->start((QThread::Priority)QThread::LowPriority);
        qDebug() << QString("startSUBThreads  iIndex=%1 ").arg(iIndex);
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

void SafeMain::setSUBThreads()
{
//	makeCurrent();
    qDebug() << QString("VRtogetherWidget::setSUBThreads ........................ num=%1 ").arg(getSUBThreadsNumber());
    for (int i = 0; i < getSUBThreadsNumber(); i++) {
        CapturSUBThread* thNode = new CapturSUBThread();
        QString tx1 = QString("TH%1").arg(i + 1);
		thNode->theGLMessageTh = &theGLMessage;
		thNode->setName(tx1);
        thNode->setID(i);
		mypcl[i] = thNode->getPCL();
        theSUBNodes.append(thNode);
    }

    qDebug() << QString("setSUBThreads  MAXQTHREADS= %1").arg(getSUBThreadsNumber());
	int i = 0;
    QVector<CapturSUBThread*>::iterator itGeom = theSUBNodes.begin();
    while (itGeom != theSUBNodes.end())
    {
        CapturSUBThread* thNode = (*itGeom);
//		qDebug() << QString("name=%1 worker=%2").arg(thNode->getName()).arg(thNode->getWorker()->getName());
        qDebug() << QString("name=%1 ").arg(thNode->getName());
		qDebug() << QString("SafeMain::setSUBThreads  i=%1  mypcl_size=%2 mypcl_pointer=%3").arg(i).arg(_msize(mypcl[i])).arg((qlonglong)mypcl[i]);
		++itGeom;
		i++;
    }
}

void SafeMain::destroySUBThreads()
{
    QVector<CapturSUBThread*>::iterator it = theSUBNodes.begin();
    while (it != theSUBNodes.end())
    {
        it = theSUBNodes.erase(it);
    }
    qDebug() << QString("VRtogetherWidget::destroySUBThreads size=%1 ==== ").arg(theSUBNodes.size());

}

void SafeMain::setUseSUBThreads(bool value)
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

void SafeMain::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}


void SafeMain::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void SafeMain::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void SafeMain::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void SafeMain::setYElev(int elev)
{
	yElev += elev;
	updateGL();
}



void SafeMain::animate()
{
    //    qDebug() << QString("VRtogetherWidget::animate ------ initialized=%1  allocated=%2  plyloaded=%3 ").arg(initialized).arg(allocated).arg(plyloaded);
    if(!getInitialized() || !getAllocated())return;
//    qDebug() << QString("animate %1").arg(drawed);
    //theGLMessage.startAnimate();
    paintGL();
    addIncMov();
//    gear1Rot += 2 * 16;
    drawed++;
    //frameRate.newFrame();
    update();
}

void SafeMain::initializeGL()
{
/*
    static const GLfloat lightPos[4] = { 5.0f, 5.0f, 10.0f, 1.0f };
    static const GLfloat reflectance1[4] = { 0.8f, 0.1f, 0.0f, 1.0f };
    static const GLfloat reflectance2[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
    static const GLfloat reflectance3[4] = { 0.2f, 0.2f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    gear1 = makeGear(reflectance1, 1.0, 4.0, 1.0, 0.7, 20);
    gear2 = makeGear(reflectance2, 0.5, 2.0, 2.0, 0.7, 10);
    gear3 = makeGear(reflectance3, 1.3, 2.0, 0.5, 0.7, 10);

    glEnable(GL_NORMALIZE);
    glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
//*/
    // Declare RealSense pipeline, encapsulating the actual device and sensors
    //rs2::pipeline pipe;
    // Start streaming with default recommended configuration
//	pipe.start();

	setFocus();


    qDebug() << QString("SafeMain::initializeGL _____________________________________________________ ");
    glClearColor(1.0, 0.0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef USE_CUDA_TREADS
    for(int i=0; i<iThreadsNumber; i++)
    {
        ViewerVertexBufferTh[i] = new GLVertexBuffers;
        ViewerVertexBufferTh[i]->setParams(i);// create vertexBuffer, colorBuffer and indexBuffer.
    }
#else
    ViewerVertexBuffer1 = new GLVertexBuffers;
    ViewerVertexBuffer1->setParams(iBufferID);// create vertexBuffer, colorBuffer and indexBuffer.
#endif



    setBuffers();// create the buffers posns, colors and indices.
#ifdef USE_CUDA_TREADS
    int iPosBlock = 0;//iIndex * BUFFERMEM;
    int iMaxMem2 = (iMaxMem / iThreadsNumber) * 1;
    for(int i=0; i<iThreadsNumber; i++)
    {
        IndexType *indices = ViewerVertexBufferTh[i]->getIndices(iPosBlock);
    //    qDebug() << QString("........................ pointer to indices=%1 ").arg((uint)indices);
        Q_ASSERT(indices);

        qDebug() << QString("Filling indices..........");
        int iIndex = 0;
        for (int i1 = 0; i1 < MAX_Y / iThreadsNumber; i1++) {
            for (int j1 = 0; j1 < MAX_X; j1++) {
                indices[iIndex] = IndexType(iIndex);
                iIndex++;
            }
        }
        num_elemsTh[i] = iIndex;
        qDebug() << QString("................................ th=%1 num_elemsTh=%2 ").arg(i).arg(num_elemsTh[i]);

        ViewerVertexBufferTh[i]->allocVertexBuffer(iMaxMem2);
        ViewerVertexBufferTh[i]->allocColorBuffer(iMaxMem2);
        ViewerVertexBufferTh[i]->allocIndexBuffer(iMaxMem2);
    }

    //    m_iNumVertices = iIndex;
    //    m_iNumVerticesPerThread = m_iNumVertices / iThreadsNumber;
//*/

#else
    IndexType *indices = ViewerVertexBuffer1->getIndices();
//    qDebug() << QString("........................ pointer to indices=%1 ").arg((uint)indices);
    Q_ASSERT(indices);

    ViewerVertexBuffer1->allocVertexBuffer(iMaxMem);
    ViewerVertexBuffer1->allocColorBuffer(iMaxMem);
	//ViewerVertexBuffer1->allocAllBuffer(iMaxMem);
    ViewerVertexBuffer1->allocIndexBuffer(iMaxMem);
    qDebug() << QString("Filling indices..........  iMaxMem=%1    indices_size=%2 ").arg(iMaxMem).arg(_msize(indices));

    int iIndex = 0;
    for (int i = 0; i < MAX_Y; i++) {
        for (int j = 0; j < MAX_X; j++) {
            indices[iIndex] = IndexType(iIndex);
            iIndex++;
        }
    }
	qDebug() << QString("Idices filled................................ ");
    num_elems = iIndex;
    m_iNumVertices = iIndex;
    m_iNumVerticesPerThread = m_iNumVertices / iThreadsNumber;
    qDebug() << QString("................................ num_elems=%1 ").arg(num_elems);

#ifndef THREADSUB
	mySUB01->read_ini(0);
	mySUB02->read_ini(1);
	mySUB03->read_ini(2);
#endif

#endif
//#define USE_CUDA_TREADS
//#define USE_CUDA_NOTH
//#define USE_CPU

#ifdef USE_CUDA
    #ifdef USE_CUDA_NOTH
        setCudaParam();
        allocVerticesCUDA();
    #endif
    #ifdef USE_CUDA_TREADS
        setCudaParamThreads();
        for(int i=0; i<iThreadsNumber; i++)
        {
            allocVerticesCUDAThreads(i);
        }
    #endif
#ifdef USE_CUDA_TREADS_ONE
        setCudaParamMulti();
        allocVerticesCUDA();
#endif

#endif

//    qDebug() << QString("Filling indices..........");
//    int iIndex = 0;
//    for (int i = 0; i < MAX_Y; i++) {
//        for (int j = 0; j < MAX_X; j++) {
//            indices[iIndex] = IndexType(iIndex);
//            iIndex++;
//        }
//    }
//    num_elems = iIndex;
//    m_iNumVertices = iIndex;
//    m_iNumVerticesPerThread = m_iNumVertices / iThreadsNumber;
//    qDebug() << QString("................................ num_elems=%1 ").arg(num_elems);
////*/


#ifdef USE_CUDA_TREADS
    setUseThreads(true);
#endif
//    setUseSUBThreads(true);
//	oneAvatar();

    initialized = true;                           /// The devices was initialized
    allocated = true;                             /// The memory was allocated
    plyloaded = true;                             /// The file PLY was loaded

}

void SafeMain::processGeomCUDA(float despX, float despY, float despZ)
{
    QVector3D* vVertex = ViewerVertexBuffer1->getPosns();
    QVector4Du* vColor = ViewerVertexBuffer1->getColors();
    IndexType* vIndices = ViewerVertexBuffer1->getIndices();
    bool bDraw = false;
    int iTest = 0;
    int iNumAvat = 6;
    FrameInfo* timestampInfo[NUMPCL];

    for (int i = 0; i < getSUBThreadsNumber(); i++) {
        CapturSUBThread* thNode = theSUBNodes.at(i);

        bDraw = false;
        iTest = iAvatar & 1;
        if (iTest && i == 0)bDraw = true;
        iTest = iAvatar & 2;
        if (iTest && i == 1)bDraw = true;
        iTest = iAvatar & 4;
        if (iTest && i == 2)bDraw = true;
        iTest = iAvatar & 8;
        if (iTest && i == 3)bDraw = true;
        iTest = iAvatar & 16;
        if (iTest && i == 4)bDraw = true;
        iTest = iAvatar & 32;
        if (iTest && i == 5)bDraw = true;
        iTest = iAvatar & 64;
        if (iTest && i == 6)bDraw = true;
        iTest = iAvatar & 128;
        if (iTest && i == 7)bDraw = true;
        iTest = iAvatar & 256;
        if (iTest && i == 8)bDraw = true;
        iTest = iAvatar & 512;
        if (iTest && i == 9)bDraw = true;
        iTest = iAvatar & 1024;
        if (iTest && i == 10)bDraw = true;
        iTest = iAvatar & 2048;
        if (iTest && i == 11)bDraw = true;

//		qDebug() << QString("SafeMain::processGeom   th=%2 bDraw=%3 ").arg(i).arg(bDraw);
        //				qDebug() << QString("iAvatar=%1 th=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
        if (bDraw) {
            //ViewerVertexBuffer1->clearVertexBufferMem(64536, 1);
            //ViewerVertexBuffer1->clearColorBufferMem(64536, 1);
            int iposmat = 0;
            mypcl[i] = thNode->getPCL();
            timestampInfo[i] = thNode->getInfo();
            iNumPoints[i] = thNode->getNumPoints();
            num_elems = iNumPoints[i];
            iNumPointsPrev[i][iPosPrev] = iNumPoints[i];
//			qDebug() << QString("th=%1  iNumPoints=%2   iAvatar=%3 ").arg(i).arg(iNumPoints[i]).arg(QString::number( iAvatar, 16));
            if (iNumPoints[i] > 0 && mypcl[i]) {
                int iNumAvatar = 5;
                if (bOnlyOne)bDraw = false;
                if (iAvatar == 1 && bOnlyOne) { bDraw = true; iNumAvatar = 1; }
                if (iAvatar == 2 && bOnlyOne) { bDraw = true; iNumAvatar = 1; }
                if (iAvatar == 4 && bOnlyOne) { bDraw = true; iNumAvatar = 1; }
                //if (bOnlyOne) iNumAvatar = 1;
            //qDebug() << QString("SafeMain::processGeomTh   th=%1  iNumPoints=%2   iNumAvatar=%3 ").arg(i).arg(iNumPoints[i]).arg(iNumAvatar);
                for (int j1 = 0; j1 < iNumAvatar; j1++) {
                    if (bDraw) {

                        iposmat = (i * 5) + j1;
                        iTotalPoints += iNumPoints[i];
                        //qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
                        //for (int n = 0; n < 30; n++) {
                        //	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
                        //}
                        theGLMessage.startOpenCL();

                        for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
                            float vx = mypcl[i][i1].x;
                            float vy = mypcl[i][i1].y;
                            float vz = mypcl[i][i1].z;
                            if (!bOnlyOne) {

                                vx += despX;
                                vy += despY;
                                vz += despZ;
                                if (iposmat == 0) {
                                    vx -= 1.5;
                                    vy -= 1.5;
                                }
                                if (iposmat == 1) {
                                    vx -= 1.5;
                                    vy += 1.5;
                                }
                                if (iposmat == 2) {
                                    vx += 1.5;
                                    vy -= 1.5;
                                }
                                if (iposmat == 3) {
                                    vx += 1.5;
                                    vy += 1.5;
                                }
                                if (iposmat == 4) {
                                    vx -= 3.0;
                                    vy -= 1.5;
                                }
                                if (iposmat == 5) {
                                    vx += 3.0;
                                    vy -= 1.5;
                                }
                                if (iposmat == 6) {
                                    vx -= 3.0;
                                    vy += 1.5;
                                }
                                if (iposmat == 7) {
                                    vx += 3.0;
                                    vy += 1.5;
                                }
                                if (iposmat == 8) {
                                    vx -= 1.5;
                                    vy -= 4.0;
                                }
                                if (iposmat == 9) {
                                    vx += 1.5;
                                    vy -= 4.0;
                                }
                                if (iposmat == 10) {
                                    vx -= 1.5;
                                    vy += 4.0;
                                }
                                if (iposmat == 11) {
                                    vx += 1.5;
                                    vy += 4.0;
                                }

                                if (iposmat == 12) {
                                    vx -= 3.0;
                                    vy -= 4.0;
                                }
                                if (iposmat == 13) {
                                    vx += 3.0;
                                    vy -= 4.0;
                                }
                                if (iposmat == 14) {
                                    vx -= 3.0;
                                    vy += 4.0;
                                }
                                if (iposmat == 15) {
                                    vx += 3.0;
                                    vy += 4.0;
                                }
                            }

                            //*/
                            vVertex[i1].setX(vx);
                            vVertex[i1].setY(vy);
                            vVertex[i1].setZ(vz);
                            //				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

                            uint8_t cr = (mypcl[i][i1].r) * 0.5;
                            uint8_t cg = (mypcl[i][i1].g) * 0.5;
                            uint8_t cb = (mypcl[i][i1].b) * 0.5;
                            uint8_t ca = 255;
                            vColor[i1].setX(cr);
                            vColor[i1].setY(cg);
                            vColor[i1].setZ(cb);
                            //				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
                        }
                        theGLMessage.stopOpenCL();
                        //			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

                        glPointSize(3.0);
#ifdef USE_CPU
                        ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

                        ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

                        ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

                        ViewerVertexBuffer1->bindVertexBuffer();
                        ViewerVertexBuffer1->bindColorBuffer();
                        if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
                        else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
#endif
                        int uio = 0;
                    }
                }
                if (i >= 2 && bDraw) {
                    iposmat = (i * 5) + 5;
                    iTotalPoints += iNumPoints[i];
                    //qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
                    //for (int n = 0; n < 30; n++) {
                    //	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
                    //}
                    theGLMessage.startOpenCL();

                    for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
                        float vx = mypcl[i][i1].x;
                        float vy = mypcl[i][i1].y;
                        float vz = mypcl[i][i1].z;
                        vx += despX;
                        vy += despY;
                        vz += despZ;
                        if (iposmat == 0) {
                            vx -= 1.5;
                            vy -= 1.5;
                        }
                        if (iposmat == 1) {
                            vx -= 1.5;
                            vy += 1.5;
                        }
                        if (iposmat == 2) {
                            vx += 1.5;
                            vy -= 1.5;
                        }
                        if (iposmat == 3) {
                            vx += 1.5;
                            vy += 1.5;
                        }
                        if (iposmat == 4) {
                            vx -= 3.0;
                            vy -= 1.5;
                        }
                        if (iposmat == 5) {
                            vx += 3.0;
                            vy -= 1.5;
                        }
                        if (iposmat == 6) {
                            vx -= 3.0;
                            vy += 1.5;
                        }
                        if (iposmat == 7) {
                            vx += 3.0;
                            vy += 1.5;
                        }
                        if (iposmat == 8) {
                            vx -= 1.5;
                            vy -= 4.0;
                        }
                        if (iposmat == 9) {
                            vx += 1.5;
                            vy -= 4.0;
                        }
                        if (iposmat == 10) {
                            vx -= 1.5;
                            vy += 4.0;
                        }
                        if (iposmat == 11) {
                            vx += 1.5;
                            vy += 4.0;
                        }

                        if (iposmat == 12) {
                            vx -= 3.0;
                            vy -= 4.0;
                        }
                        if (iposmat == 13) {
                            vx += 3.0;
                            vy -= 4.0;
                        }
                        if (iposmat == 14) {
                            vx -= 3.0;
                            vy += 4.0;
                        }
                        if (iposmat == 15) {
                            vx += 3.0;
                            vy += 4.0;
                        }

                        //*/
                        vVertex[i1].setX(vx);
                        vVertex[i1].setY(vy);
                        vVertex[i1].setZ(vz);
                        //				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

                        uint8_t cr = (mypcl[i][i1].r) * 0.5;
                        uint8_t cg = (mypcl[i][i1].g) * 0.5;
                        uint8_t cb = (mypcl[i][i1].b) * 0.5;
                        uint8_t ca = 255;
                        vColor[i1].setX(cr);
                        vColor[i1].setY(cg);
                        vColor[i1].setZ(cb);
                        //				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
                    }
                    theGLMessage.stopOpenCL();
                    //			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

                    glPointSize(3.0);
#ifdef USE_CPU
                    ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

                    ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

                    ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

                    ViewerVertexBuffer1->bindVertexBuffer();
                    ViewerVertexBuffer1->bindColorBuffer();
                    if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
                    else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
#endif
                }
            }

            if (iPosPrev >= 10)iPosPrev = -1;
            iPosPrev++;

        }
        bDraw = false;
    }

}

void SafeMain::processGeomTh(float despX, float despY, float despZ)
{
	QVector3D* vVertex = ViewerVertexBuffer1->getPosns();
	QVector4Du* vColor = ViewerVertexBuffer1->getColors();
	IndexType* vIndices = ViewerVertexBuffer1->getIndices();
	bool bDraw = false;
	int iTest = 0;
	int iNumAvat = 6;
	FrameInfo* timestampInfo[NUMPCL];

	for (int i = 0; i < getSUBThreadsNumber(); i++) {
		CapturSUBThread* thNode = theSUBNodes.at(i);

		bDraw = false;
		iTest = iAvatar & 1;
		if (iTest && i == 0)bDraw = true;
		iTest = iAvatar & 2;
		if (iTest && i == 1)bDraw = true;
		iTest = iAvatar & 4;
		if (iTest && i == 2)bDraw = true;
		iTest = iAvatar & 8;
		if (iTest && i == 3)bDraw = true;
		iTest = iAvatar & 16;
		if (iTest && i == 4)bDraw = true;
		iTest = iAvatar & 32;
		if (iTest && i == 5)bDraw = true;
		iTest = iAvatar & 64;
		if (iTest && i == 6)bDraw = true;
		iTest = iAvatar & 128;
		if (iTest && i == 7)bDraw = true;
		iTest = iAvatar & 256;
		if (iTest && i == 8)bDraw = true;
		iTest = iAvatar & 512;
		if (iTest && i == 9)bDraw = true;
		iTest = iAvatar & 1024;
		if (iTest && i == 10)bDraw = true;
		iTest = iAvatar & 2048;
		if (iTest && i == 11)bDraw = true;

//		qDebug() << QString("SafeMain::processGeom   th=%2 bDraw=%3 ").arg(i).arg(bDraw);
		//				qDebug() << QString("iAvatar=%1 th=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
		if (bDraw) {
			//ViewerVertexBuffer1->clearVertexBufferMem(64536, 1);
			//ViewerVertexBuffer1->clearColorBufferMem(64536, 1);
			int iposmat = 0;
			mypcl[i] = thNode->getPCL();
			timestampInfo[i] = thNode->getInfo();
			iNumPoints[i] = thNode->getNumPoints();
			num_elems = iNumPoints[i];
			iNumPointsPrev[i][iPosPrev] = iNumPoints[i];
//			qDebug() << QString("th=%1  iNumPoints=%2   iAvatar=%3 ").arg(i).arg(iNumPoints[i]).arg(QString::number( iAvatar, 16));
			if (iNumPoints[i] > 0 && mypcl[i]) {
				int iNumAvatar = 5;
				if (bOnlyOne)bDraw = false;
				if (iAvatar == 1 && bOnlyOne) { bDraw = true; iNumAvatar = 1; };
				if (iAvatar == 2 && bOnlyOne) { bDraw = true; iNumAvatar = 1; }
				if (iAvatar == 4 && bOnlyOne) { bDraw = true; iNumAvatar = 1; }
				//if (bOnlyOne) iNumAvatar = 1;
			//qDebug() << QString("SafeMain::processGeomTh   th=%1  iNumPoints=%2   iNumAvatar=%3 ").arg(i).arg(iNumPoints[i]).arg(iNumAvatar);
				for (int j1 = 0; j1 < iNumAvatar; j1++) {
					if (bDraw) {

						iposmat = (i * 5) + j1;
						iTotalPoints += iNumPoints[i];
						//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
						//for (int n = 0; n < 30; n++) {
						//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
						//}
						theGLMessage.startOpenCL();

						for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
							float vx = mypcl[i][i1].x;
							float vy = mypcl[i][i1].y;
							float vz = mypcl[i][i1].z;
							if (!bOnlyOne) {

								vx += despX;
								vy += despY;
								vz += despZ;
								if (iposmat == 0) {
									vx -= 1.5;
									vy -= 1.5;
								}
								if (iposmat == 1) {
									vx -= 1.5;
									vy += 1.5;
								}
								if (iposmat == 2) {
									vx += 1.5;
									vy -= 1.5;
								}
								if (iposmat == 3) {
									vx += 1.5;
									vy += 1.5;
								}
								if (iposmat == 4) {
									vx -= 3.0;
									vy -= 1.5;
								}
								if (iposmat == 5) {
									vx += 3.0;
									vy -= 1.5;
								}
								if (iposmat == 6) {
									vx -= 3.0;
									vy += 1.5;
								}
								if (iposmat == 7) {
									vx += 3.0;
									vy += 1.5;
								}
								if (iposmat == 8) {
									vx -= 1.5;
									vy -= 4.0;
								}
								if (iposmat == 9) {
									vx += 1.5;
									vy -= 4.0;
								}
								if (iposmat == 10) {
									vx -= 1.5;
									vy += 4.0;
								}
								if (iposmat == 11) {
									vx += 1.5;
									vy += 4.0;
								}

								if (iposmat == 12) {
									vx -= 3.0;
									vy -= 4.0;
								}
								if (iposmat == 13) {
									vx += 3.0;
									vy -= 4.0;
								}
								if (iposmat == 14) {
									vx -= 3.0;
									vy += 4.0;
								}
								if (iposmat == 15) {
									vx += 3.0;
									vy += 4.0;
								}
							}

							//*/
							vVertex[i1].setX(vx);
							vVertex[i1].setY(vy);
							vVertex[i1].setZ(vz);
							//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

							uint8_t cr = (mypcl[i][i1].r) * 0.5;
							uint8_t cg = (mypcl[i][i1].g) * 0.5;
							uint8_t cb = (mypcl[i][i1].b) * 0.5;
							uint8_t ca = 255;
							vColor[i1].setX(cr);
							vColor[i1].setY(cg);
							vColor[i1].setZ(cb);
							//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
						}
						theGLMessage.stopOpenCL();
						//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

						glPointSize(3.0);
#ifdef USE_CPU
						ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

						ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

						ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

						ViewerVertexBuffer1->bindVertexBuffer();
						ViewerVertexBuffer1->bindColorBuffer();
						if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
						else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
#endif
						int uio = 0;
					}
				}
				if (i >= 2 && bDraw) {
					iposmat = (i * 5) + 5;
					iTotalPoints += iNumPoints[i];
					//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
					//for (int n = 0; n < 30; n++) {
					//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
					//}
					theGLMessage.startOpenCL();

					for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
						float vx = mypcl[i][i1].x;
						float vy = mypcl[i][i1].y;
						float vz = mypcl[i][i1].z;
						vx += despX;
						vy += despY;
						vz += despZ;
						if (iposmat == 0) {
							vx -= 1.5;
							vy -= 1.5;
						}
						if (iposmat == 1) {
							vx -= 1.5;
							vy += 1.5;
						}
						if (iposmat == 2) {
							vx += 1.5;
							vy -= 1.5;
						}
						if (iposmat == 3) {
							vx += 1.5;
							vy += 1.5;
						}
						if (iposmat == 4) {
							vx -= 3.0;
							vy -= 1.5;
						}
						if (iposmat == 5) {
							vx += 3.0;
							vy -= 1.5;
						}
						if (iposmat == 6) {
							vx -= 3.0;
							vy += 1.5;
						}
						if (iposmat == 7) {
							vx += 3.0;
							vy += 1.5;
						}
						if (iposmat == 8) {
							vx -= 1.5;
							vy -= 4.0;
						}
						if (iposmat == 9) {
							vx += 1.5;
							vy -= 4.0;
						}
						if (iposmat == 10) {
							vx -= 1.5;
							vy += 4.0;
						}
						if (iposmat == 11) {
							vx += 1.5;
							vy += 4.0;
						}

						if (iposmat == 12) {
							vx -= 3.0;
							vy -= 4.0;
						}
						if (iposmat == 13) {
							vx += 3.0;
							vy -= 4.0;
						}
						if (iposmat == 14) {
							vx -= 3.0;
							vy += 4.0;
						}
						if (iposmat == 15) {
							vx += 3.0;
							vy += 4.0;
						}

						//*/
						vVertex[i1].setX(vx);
						vVertex[i1].setY(vy);
						vVertex[i1].setZ(vz);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

						uint8_t cr = (mypcl[i][i1].r) * 0.5;
						uint8_t cg = (mypcl[i][i1].g) * 0.5;
						uint8_t cb = (mypcl[i][i1].b) * 0.5;
						uint8_t ca = 255;
						vColor[i1].setX(cr);
						vColor[i1].setY(cg);
						vColor[i1].setZ(cb);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
					}
					theGLMessage.stopOpenCL();
					//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

					glPointSize(3.0);
#ifdef USE_CPU
					ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

					ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

					ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

					ViewerVertexBuffer1->bindVertexBuffer();
					ViewerVertexBuffer1->bindColorBuffer();
					if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
					else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
#endif
				}
			}

			if (iPosPrev >= 10)iPosPrev = -1;
			iPosPrev++;

		}
		bDraw = false;
	}

}

void SafeMain::processGeom(float despX, float despY, float despZ)
{
#ifndef THREADSUB

	QVector3D* vVertex = ViewerVertexBuffer1->getPosns();
	QVector4Du* vColor = ViewerVertexBuffer1->getColors();
	IndexType* vIndices = ViewerVertexBuffer1->getIndices();
	bool bDraw = false;
	int iTest = 0;
	int iNumAvat = 6;
	FrameInfo* timestampInfo[NUMPCL];

	int iposmat = 0; int i = 0;
	mypcl[i] = mySUB01->getPCL();
	timestampInfo[i] = mySUB01->getInfo();
	iNumPoints[i] = mySUB01->getNumPoints();
	num_elems = iNumPoints[i];
	if (iNumPoints[i] > 0) {
		theGLMessage.startOpenCL();

		for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
			float vx = mypcl[i][i1].x;
			float vy = mypcl[i][i1].y;
			float vz = mypcl[i][i1].z;
			if (!bOnlyOne) {

				vx += despX;
				vy += despY;
				vz += despZ;
				if (iposmat == 0) {
					vx -= 1.5;
					vy -= 1.5;
				}
				if (iposmat == 1) {
					vx -= 1.5;
					vy += 1.5;
				}
				if (iposmat == 2) {
					vx += 1.5;
					vy -= 1.5;
				}
				if (iposmat == 3) {
					vx += 1.5;
					vy += 1.5;
				}
				if (iposmat == 4) {
					vx -= 3.0;
					vy -= 1.5;
				}
				if (iposmat == 5) {
					vx += 3.0;
					vy -= 1.5;
				}
				if (iposmat == 6) {
					vx -= 3.0;
					vy += 1.5;
				}
				if (iposmat == 7) {
					vx += 3.0;
					vy += 1.5;
				}
				if (iposmat == 8) {
					vx -= 1.5;
					vy -= 4.0;
				}
				if (iposmat == 9) {
					vx += 1.5;
					vy -= 4.0;
				}
				if (iposmat == 10) {
					vx -= 1.5;
					vy += 4.0;
				}
				if (iposmat == 11) {
					vx += 1.5;
					vy += 4.0;
				}

				if (iposmat == 12) {
					vx -= 3.0;
					vy -= 4.0;
				}
				if (iposmat == 13) {
					vx += 3.0;
					vy -= 4.0;
				}
				if (iposmat == 14) {
					vx -= 3.0;
					vy += 4.0;
				}
				if (iposmat == 15) {
					vx += 3.0;
					vy += 4.0;
				}
			}

			//*/
			vVertex[i1].setX(vx);
			vVertex[i1].setY(vy);
			vVertex[i1].setZ(vz);
			//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

			uint8_t cr = (mypcl[i][i1].r) * 0.5;
			uint8_t cg = (mypcl[i][i1].g) * 0.5;
			uint8_t cb = (mypcl[i][i1].b) * 0.5;
			uint8_t ca = 255;
			vColor[i1].setX(cr);
			vColor[i1].setY(cg);
			vColor[i1].setZ(cb);
			//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
		}
		theGLMessage.stopOpenCL();
		//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

		glPointSize(3.0);

		ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

		ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

		ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

		ViewerVertexBuffer1->bindVertexBuffer();
		ViewerVertexBuffer1->bindColorBuffer();
		if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
		else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);

	}
#endif
}


void SafeMain::paintGL()
{
    if(!getInitialized() || !getAllocated())return;
/*
    void (TVMReceiver::* ptfptr)(uint, DMesh) = &TVMReceiver::micasa;
    fnOnReceivedMesh(TVMReceiver::* tvmptr)(uint, DMesh) = &TVMReceiver::michoza;
    fnOnConnectionError(TVMReceiver::* errorptr)(uint) = &TVMReceiver::mierror;

    myTVMReceiver->myMesh.nVertices = 0;
    myTVMReceiver->myMesh.nTextures = 0;
    myTVMReceiver->myMesh.nTriangles = 0;
//*/
    // Wait for the next set of frames from the camera
    //auto frames = pipe.wait_for_frames();

    //auto depth = frames.get_depth_frame();


#ifdef THREADSUB
	QMutex doStopMutex;
#endif
	Timer time01;
	iTotalPoints = 0;

	theGLMessage.startDevices();
	theGLMessage.startCPU();


    glClearColor(1.0, 0.5, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);

	//qDebug() << QString("SafeMain::paintGL   m_fDistEye=%1  yElev=%2 ").arg(m_fDistEye).arg(yElev);
    int iW = (width() * 0.5) + m_fDistEye;
    int iH = (height() * 0.5) + m_fDistEye;

//    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-iW, iW , -iH, iH, -10000, 10000);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(0.0, yElev, 0);

//	qDebug() << QString("m_fDistEye=%1 yElev=%2 ").arg(m_fDistEye).arg(yElev);

    //glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    //glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    //glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    glRotated(xRot, 1.0, 0.0, 0.0);
    glRotated(yRot, 0.0, 1.0, 0.0);
    glRotated(zRot, 0.0, 0.0, 1.0);
	float fScale = 200.0;
	glScalef(fScale, fScale, fScale);

	yRot += 2.0;
//	qDebug() << QString("xRot=%1 yRot=%2 zRot=%3 ").arg(xRot).arg(yRot).arg(zRot);
/*
    drawGear(gear1, -3.0, -2.0, 0.0, gear1Rot / 16.0);
    drawGear(gear2, +3.1, -2.0, 0.0, -2.0 * (gear1Rot / 16.0) - 9.0);

    glRotated(+90.0, 1.0, 0.0, 0.0);
    drawGear(gear3, -3.1, -1.8, -2.2, +2.0 * (gear1Rot / 16.0) - 2.0);
//*/
    float fPointThick = 3.0f;
    glPointSize(fPointThick);

	//float brt = 1.5;
	//float con = 2.0;
	//float sat = 1.5;



#ifdef USE_CPU
    //GenerateCloud();//                               leer nuve de puntos de SUB
    Q_ASSERT(ViewerVertexBuffer1);

	//doStopM02.lock();
	//for (int i = 0; i < 20; i++) {
	//	qDebug() << QString("safeMain     i=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    uID=0 ").arg(i).arg(mypcl[0][i].x).arg(mypcl[0][i].y).arg(mypcl[0][i].z).arg(mypcl[0][i].r).arg(mypcl[0][i].g).arg(mypcl[0][i].b);
	//	qDebug() << QString("safeMain     i=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    uID=1 ").arg(i).arg(mypcl[1][i].x).arg(mypcl[1][i].y).arg(mypcl[1][i].z).arg(mypcl[1][i].r).arg(mypcl[1][i].g).arg(mypcl[1][i].b);
	//	qDebug() << QString("safeMain     i=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    uID=2 ").arg(i).arg(mypcl[2][i].x).arg(mypcl[2][i].y).arg(mypcl[2][i].z).arg(mypcl[2][i].r).arg(mypcl[2][i].g).arg(mypcl[2][i].b);
	//	qDebug() << QString("safeMain     i=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    uID=3 ").arg(i).arg(mypcl[3][i].x).arg(mypcl[3][i].y).arg(mypcl[3][i].z).arg(mypcl[3][i].r).arg(mypcl[3][i].g).arg(mypcl[3][i].b);
	//}
	//doStopM02.unlock();
    filePos = 0;
    //QVector3D* vVertex = ViewerVertexBuffer1->getPosns();
    //QVector4Du* vColor = ViewerVertexBuffer1->getColors();
    //IndexType* vIndices = ViewerVertexBuffer1->getIndices();
//	float* vVertexFloat = ViewerVertexBuffer1->getPosnsFloat();

    //ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, filePos, false);

    //ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, filePos, false);

//	qDebug() << QString("paintGL ---------------------------------- ");
#ifdef THREADSUB
		doStopMutex.lock();
#endif

//	glColor3f(1.0, 0.0, 0.0);
	theGLMessage.startPaint();
	//bool bDraw = false;
	//int iTest = 0;
	//int iNumAvat = 6;
	//FrameInfo* timestampInfo[NUMPCL];
#ifdef	THREADSUB
	//processGeom(0.0, 12.0, 0.0);
	//processGeom(-10.0, 0.0, 0.0);

	processGeomTh(0.0, 0.0, 0.0);
//	processGeom(0.0, 0.0, 0.0);
//void CapturSUB::processGeom(GLVertexBuffers *ViewerVertexBuffer1, GLMessages* theGLMessage, bool bSeeColor, float despX, float despY, float despZ)


	//processGeom(10.0, 0.0, 0.0);
	//processGeom(0.0, -12.0, 0.0);

	//processGeom(10.0, 12.0, 0.0);
	//processGeom(-10.0, 12.0, 0.0);
	//processGeom(10.0, -12.0, 0.0);
	//processGeom(-10.0, -12.0, 0.0);
#else
	float despX1 = 1.5;
	mySUB01->read(0);
	mySUB01->processGeom(ViewerVertexBuffer1, &theGLMessage, bSeeColor, -1.2 + despX1, 0, 0);
	//mySUB02->read(1);
	//mySUB02->processGeom(ViewerVertexBuffer1, &theGLMessage, bSeeColor, 0 + despX1, 0, 0);
	//mySUB03->read(2);
	//mySUB03->processGeom(ViewerVertexBuffer1, &theGLMessage, bSeeColor, 1.2 + despX1, 0, 0);
#endif
	/*
	for (int i = 0; i < getSUBThreadsNumber(); i++) {
		CapturSUBThread* thNode = theSUBNodes.at(i);

		bDraw = false;
		iTest = iAvatar & 1;
		if (iTest && i == 0)bDraw = true;
		iTest = iAvatar & 2;
		if (iTest && i == 1)bDraw = true;
		iTest = iAvatar & 4;
		if (iTest && i == 2)bDraw = true;
		iTest = iAvatar & 8;
		if (iTest && i == 3)bDraw = true;
		iTest = iAvatar & 16;
		if (iTest && i == 4)bDraw = true;
		iTest = iAvatar & 32;
		if (iTest && i == 5)bDraw = true;
		iTest = iAvatar & 64;
		if (iTest && i == 6)bDraw = true;
		iTest = iAvatar & 128;
		if (iTest && i == 7)bDraw = true;
		iTest = iAvatar & 256;
		if (iTest && i == 8)bDraw = true;
		iTest = iAvatar & 512;
		if (iTest && i == 9)bDraw = true;
		iTest = iAvatar & 1024;
		if (iTest && i == 10)bDraw = true;
		iTest = iAvatar & 2048;
		if (iTest && i == 11)bDraw = true;

//		qDebug() << QString("iAvatar=%1 i=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
		if (bDraw) {
			//ViewerVertexBuffer1->clearVertexBufferMem(64536, 1);
			//ViewerVertexBuffer1->clearColorBufferMem(64536, 1);
			int iposmat = 0;
			mypcl[i] = thNode->getPCL();
			timestampInfo[i] = thNode->getInfo();
			iNumPoints[i] = thNode->getNumPoints();
			num_elems = iNumPoints[i];
			if (iNumPoints[i] > 0) {
				for (int j1 = 0; j1 < 5; j1++) {
					iposmat = (i * 5) + j1;
					iTotalPoints += iNumPoints[i];
					//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
					//for (int n = 0; n < 30; n++) {
					//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
					//}
					theGLMessage.startOpenCL();

					for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
						float vx = mypcl[i][i1].x;
						float vy = mypcl[i][i1].y;
						float vz = mypcl[i][i1].z;
						if (iposmat == 0) {
							vx -= 1.5;
							vy -= 1.5;
						}
						if (iposmat == 1) {
							vx -= 1.5;
							vy += 1.5;
						}
						if (iposmat == 2) {
							vx += 1.5;
							vy -= 1.5;
						}
						if (iposmat == 3) {
							vx += 1.5;
							vy += 1.5;
						}
						if (iposmat == 4) {
							vx -= 3.0;
							vy -= 1.5;
						}
						if (iposmat == 5) {
							vx += 3.0;
							vy -= 1.5;
						}
						if (iposmat == 6) {
							vx -= 3.0;
							vy += 1.5;
						}
						if (iposmat == 7) {
							vx += 3.0;
							vy += 1.5;
						}
						if (iposmat == 8) {
							vx -= 1.5;
							vy -= 4.0;
						}
						if (iposmat == 9) {
							vx += 1.5;
							vy -= 4.0;
						}
						if (iposmat == 10) {
							vx -= 1.5;
							vy += 4.0;
						}
						if (iposmat == 11) {
							vx += 1.5;
							vy += 4.0;
						}

						if (iposmat == 12) {
							vx -= 3.0;
							vy -= 4.0;
						}
						if (iposmat == 13) {
							vx += 3.0;
							vy -= 4.0;
						}
						if (iposmat == 14) {
							vx -= 3.0;
							vy += 4.0;
						}
						if (iposmat == 15) {
							vx += 3.0;
							vy += 4.0;
						}


						vVertex[i1].setX(vx);
						vVertex[i1].setY(vy);
						vVertex[i1].setZ(vz);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

						uint8_t cr = (mypcl[i][i1].r) * 0.5;
						uint8_t cg = (mypcl[i][i1].g) * 0.5;
						uint8_t cb = (mypcl[i][i1].b) * 0.5;
						uint8_t ca = 255;
						vColor[i1].setX(cr);
						vColor[i1].setY(cg);
						vColor[i1].setZ(cb);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
					}
					theGLMessage.stopOpenCL();
					//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

					glPointSize(3.0);

					ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

					ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

					ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

					ViewerVertexBuffer1->bindVertexBuffer();
					ViewerVertexBuffer1->bindColorBuffer();
					if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
					else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
					int uio = 0;
				}
				if (i >= 2) {
					iposmat = (i * 5) + 5;
					iTotalPoints += iNumPoints[i];
					//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
					//for (int n = 0; n < 30; n++) {
					//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
					//}
					theGLMessage.startOpenCL();

					for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
						float vx = mypcl[i][i1].x;
						float vy = mypcl[i][i1].y;
						float vz = mypcl[i][i1].z;
						if (iposmat == 0) {
							vx -= 1.5;
							vy -= 1.5;
						}
						if (iposmat == 1) {
							vx -= 1.5;
							vy += 1.5;
						}
						if (iposmat == 2) {
							vx += 1.5;
							vy -= 1.5;
						}
						if (iposmat == 3) {
							vx += 1.5;
							vy += 1.5;
						}
						if (iposmat == 4) {
							vx -= 3.0;
							vy -= 1.5;
						}
						if (iposmat == 5) {
							vx += 3.0;
							vy -= 1.5;
						}
						if (iposmat == 6) {
							vx -= 3.0;
							vy += 1.5;
						}
						if (iposmat == 7) {
							vx += 3.0;
							vy += 1.5;
						}
						if (iposmat == 8) {
							vx -= 1.5;
							vy -= 4.0;
						}
						if (iposmat == 9) {
							vx += 1.5;
							vy -= 4.0;
						}
						if (iposmat == 10) {
							vx -= 1.5;
							vy += 4.0;
						}
						if (iposmat == 11) {
							vx += 1.5;
							vy += 4.0;
						}

						if (iposmat == 12) {
							vx -= 3.0;
							vy -= 4.0;
						}
						if (iposmat == 13) {
							vx += 3.0;
							vy -= 4.0;
						}
						if (iposmat == 14) {
							vx -= 3.0;
							vy += 4.0;
						}
						if (iposmat == 15) {
							vx += 3.0;
							vy += 4.0;
						}

						vVertex[i1].setX(vx);
						vVertex[i1].setY(vy);
						vVertex[i1].setZ(vz);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

						uint8_t cr = (mypcl[i][i1].r) * 0.5;
						uint8_t cg = (mypcl[i][i1].g) * 0.5;
						uint8_t cb = (mypcl[i][i1].b) * 0.5;
						uint8_t ca = 255;
						vColor[i1].setX(cr);
						vColor[i1].setY(cg);
						vColor[i1].setZ(cb);
						//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
					}
					theGLMessage.stopOpenCL();
					//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

					glPointSize(3.0);

					ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

					ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, false);

					ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);

					ViewerVertexBuffer1->bindVertexBuffer();
					ViewerVertexBuffer1->bindColorBuffer();
					if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
					else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
				}
			}



		}
		bDraw = false;
	}
//*/
	theGLMessage.stopPaint();

/*
	int i = 0;
	CapturSUBThread* thNode = theSUBNodes.at(i);
	int iTest = 0;

	bDraw = false;
	iTest = iAvatar & 1;
	if (iTest && i == 0)bDraw = true;
//	qDebug() << QString("iAvatar=%1 i=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
	if (bDraw) {
		mypcl[i] = thNode->getPCL();
		FrameInfo* timestampInfo[NUMPCL];
		timestampInfo[i] = thNode->getInfo();
		iNumPoints[i] = thNode->getNumPoints();
		num_elems = iNumPoints[i];
		if (iNumPoints[i] > 0) {
			iTotalPoints += iNumPoints[i];
			//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
			//for (int n = 0; n < 30; n++) {
			//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
			//}
			time01.start();

			for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
				float vx = mypcl[i][i1].x;
				float vy = mypcl[i][i1].y;
				float vz = mypcl[i][i1].z;
				if (i == 0) {
					vx -= 1.5;
					vy -= 1.5;
				}
				if (i == 1) {
					vx -= 1.5;
					vy += 1.5;
				}
				if (i == 2) {
					vx += 1.5;
					vy -= 1.5;
				}
				if (i == 3) {
					vx += 1.5;
					vy += 1.5;
				}
				vVertex[i1].setX(vx);
				vVertex[i1].setY(vy);
				vVertex[i1].setZ(vz);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }
//*/
/*

template <class T, class T2>
T mix(const T &a, const T &b, const T2 &interp) {
  static constexpr T2 one = ((T2)1);
  return (a * (one - interp)) + (b * interp);
}
			OPENCL
__constant float3 LumCoeff = (float3)(0.2125, 0.7154, 0.0721);
__constant float3 AvgLumin = (float3)(0.5, 0.5, 0.5);


					float3 brtColor = pixel1.xyz * brt;
					float3 intensity = (float3)(dot(brtColor, LumCoeff));
					float3 satColor = mix(intensity, brtColor, sat);
					pixel1 = (float4)(mix(AvgLumin, satColor, con), 1.0);
					pixel1 = clamp(pixel1, 0.0, 1.0);

			CUDA
					float3 brtColor = pixel1 * brt;
					float3 intensity = make_float3(dot(brtColor, LumCoeff));
					float3 satColor = lerp(intensity, brtColor, sat);
					pixel1 = lerp(AvgLumin, satColor, con);



#define      DOTPROD(a,b)     ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])


Procedure Truncate(value)
   If value < 0 Then value = 0
   If value > 255 Then value = 255
   Return value
EndProcedure

factor = (259 * (contrast + 255)) / (255 * (259 - contrast))
colour = GetPixelColour(x, y)
newRed   = Truncate(factor * (Red(colour)   - 128) + 128)
newGreen = Truncate(factor * (Green(colour) - 128) + 128)
newBlue  = Truncate(factor * (Blue(colour)  - 128) + 128)
PutPixelColour(x, y) = RGB(newRed, newGreen, newBlue)



*/
/*				//uint8_t cr = (255 - mypcl[i][i1].r) * 0.299;
				//uint8_t cg = (255 - mypcl[i][i1].g) * 0.587;
				//uint8_t cb = (255 - mypcl[i][i1].b) * 0.114;
				float crB = mypcl[i][i1].r * brt;
				float cgB = mypcl[i][i1].g * brt;
				float cbB = mypcl[i][i1].b * brt;

				float intR = crB * 0.215;
				float intG = cgB * 0.7154;
				float intB = cbB * 0.0721;
//				float3 satColor = lerp(intensity, brtColor, sat);

				float satR = mix(intR, crB, sat);
				float satG = mix(intG, cgB, sat);
				float satB = mix(intB, cbB, sat);

//				pixel1 = (float4)(mix(AvgLumin, satColor, con), 1.0);
				float outR = mix(0.5, satR, con);
				float outG = mix(0.5, satG, con);
				float outB = mix(0.5, satB, con);
				uint8_t cr = (uint8_t)outR;
				uint8_t cg = (uint8_t)outG;
				uint8_t cb = (uint8_t)outB;
				uint8_t ca = 255;
				if (i1 < 12) {
					qDebug() << QString("outR=%1 outG=%2 outB=%3   cr=%4 cg=%5 cb=%6 ").arg(outR).arg(outG).arg(outB).arg(cr).arg(cb).arg(cb);
				}
//*/
/*
				uint8_t cr = (mypcl[i][i1].r) * (0.299 * 1.5);
				uint8_t cg = (mypcl[i][i1].g) * (0.3 * 1.5);// 0.587;
				uint8_t cb = (mypcl[i][i1].b) * (0.3 * 1.5);// 0.114;
				uint8_t ca = 255;
				//vColor[i1].setX(cb);
				//vColor[i1].setY(cg);
				//vColor[i1].setZ(cr);
	//			cr = 0; cg = 255; cb = 0; ca = 255;
				vColor[i1].setX(cr);
				vColor[i1].setY(cg);
				vColor[i1].setZ(cb);
				vColor[i1].setW(ca);
				if (i1 < 12) { qDebug() << QString("paintGL     i1=%0 color=%1 %2 %3 %4  timestamp=%5  vColor=%6 %7 %8 %9 ")
					.arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()).arg(vColor[i1].w()).arg(timestampInfo[i]->timestamp)
					.arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()).arg(vColor[i1].w()); }
			}
			time01.stop();

			//			qDebug() << QString("time=%1  ").arg(time01.getElapsedTimeInMicroSec());

						//		ViewerVertexBuffer1->writeAllBuffer((uchar*)mypcl[i], iNumPoints[i], true);
			glPointSize(4.0);

			ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, false);

			ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, true);

			ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, false);
//			ViewerVertexBuffer1->printSizes();

			ViewerVertexBuffer1->bindVertexBuffer();
			ViewerVertexBuffer1->bindColorBuffer();
			if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
			else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
			int uio = 0;
		}
	}
//*/
/*
	i++;
	thNode = theSUBNodes.at(i);

	bDraw = false;
	iTest = iAvatar & 2;
	if (iTest && i == 1)bDraw = true;
	//	qDebug() << QString("iAvatar=%1 i=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
	if (bDraw) {
		mypcl[i] = thNode->getPCL();
		iNumPoints[i] = thNode->getNumPoints();
		num_elems = iNumPoints[i];
		if (iNumPoints[i] > 0) {
			iTotalPoints += iNumPoints[i];
			//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
			//for (int n = 0; n < 30; n++) {
			//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
			//}
			time01.start();

			for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
				float vx = mypcl[i][i1].x;
				float vy = mypcl[i][i1].y;
				float vz = mypcl[i][i1].z;
				if (i == 0) {
					vx -= 1.5;
					vy -= 1.5;
				}
				if (i == 1) {
					vx -= 1.5;
					vy += 1.5;
				}
				if (i == 2) {
					vx += 1.5;
					vy -= 1.5;
				}
				if (i == 3) {
					vx += 1.5;
					vy += 1.5;
				}
				vVertex[i1].setX(vx);
				vVertex[i1].setY(vy);
				vVertex[i1].setZ(vz);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

				uint8_t cr = mypcl[i][i1].r;
				uint8_t cg = mypcl[i][i1].g;
				uint8_t cb = mypcl[i][i1].b;
				//vColor[i1].setX(cb);
				//vColor[i1].setY(cg);
				//vColor[i1].setZ(cr);
				vColor[i1].setX(cr);
				vColor[i1].setY(cg);
				vColor[i1].setZ(cb);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
			}
			time01.stop();
			//			qDebug() << QString("time=%1  ").arg(time01.getElapsedTimeInMicroSec());

						//		ViewerVertexBuffer1->writeAllBuffer((uchar*)mypcl[i], iNumPoints[i], true);
			glPointSize(3.0);

			ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, filePos, false);

			ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, filePos, false);

			ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, filePos, false);

			ViewerVertexBuffer1->bindVertexBuffer();
			ViewerVertexBuffer1->bindColorBuffer();
			if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
			else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
			int uio = 0;
		}
	}

	i++;
	thNode = theSUBNodes.at(i);

	bDraw = false;
	iTest = iAvatar & 4;
	if (iTest && i == 2)bDraw = true;
	//	qDebug() << QString("iAvatar=%1 i=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
	if (bDraw) {
		mypcl[i] = thNode->getPCL();
		iNumPoints[i] = thNode->getNumPoints();
		num_elems = iNumPoints[i];
		if (iNumPoints[i] > 0) {
			iTotalPoints += iNumPoints[i];
			//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
			//for (int n = 0; n < 30; n++) {
			//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
			//}
			time01.start();

			for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
				float vx = mypcl[i][i1].x;
				float vy = mypcl[i][i1].y;
				float vz = mypcl[i][i1].z;
				if (i == 0) {
					vx -= 1.5;
					vy -= 1.5;
				}
				if (i == 1) {
					vx -= 1.5;
					vy += 1.5;
				}
				if (i == 2) {
					vx += 1.5;
					vy -= 1.5;
				}
				if (i == 3) {
					vx += 1.5;
					vy += 1.5;
				}
				vVertex[i1].setX(vx);
				vVertex[i1].setY(vy);
				vVertex[i1].setZ(vz);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

				uint8_t cr = mypcl[i][i1].r;
				uint8_t cg = mypcl[i][i1].g;
				uint8_t cb = mypcl[i][i1].b;
				//vColor[i1].setX(cb);
				//vColor[i1].setY(cg);
				//vColor[i1].setZ(cr);
				vColor[i1].setX(cr);
				vColor[i1].setY(cg);
				vColor[i1].setZ(cb);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
			}
			time01.stop();
			//			qDebug() << QString("time=%1  ").arg(time01.getElapsedTimeInMicroSec());

						//		ViewerVertexBuffer1->writeAllBuffer((uchar*)mypcl[i], iNumPoints[i], true);
			glPointSize(3.0);

			ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, filePos, false);

			ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, filePos, false);

			ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, filePos, false);

			ViewerVertexBuffer1->bindVertexBuffer();
			ViewerVertexBuffer1->bindColorBuffer();
			if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
			else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
			int uio = 0;
		}
	}

	i++;
	thNode = theSUBNodes.at(i);

	bDraw = false;
	iTest = iAvatar & 8;
	if (iTest && i == 3)bDraw = true;
	//	qDebug() << QString("iAvatar=%1 i=%2 bDraw=%3 ").arg(QString::number(iAvatar, 16)).arg(i).arg(bDraw);
	if (bDraw) {
		mypcl[i] = thNode->getPCL();
		iNumPoints[i] = thNode->getNumPoints();
		num_elems = iNumPoints[i];
		if (iNumPoints[i] > 0) {
			iTotalPoints += iNumPoints[i];
			//qDebug() << QString("........................ paintGL   i=%1 iNumPoint=%2  mypcl=%3 size=%4  cwipc_point=%5 filePos=%6 ").arg(i).arg(iNumPoints[i]).arg((qlonglong)mypcl[i]).arg(_msize(mypcl[i])).arg(sizeof(cwipc_point)).arg(filePos);
			//for (int n = 0; n < 30; n++) {
			//	qDebug() << QString("paintGL     n=%1 pcl=%2 %3 %4  rgb=%5 %6 %7    ").arg(n).arg(mypcl[i][n].x).arg(mypcl[i][n].y).arg(mypcl[i][n].z).arg(mypcl[i][n].r).arg(mypcl[i][n].g).arg(mypcl[i][n].b);
			//}
			time01.start();

			for (int i1 = 0; i1 < iNumPoints[i]; i1++) {
				float vx = mypcl[i][i1].x;
				float vy = mypcl[i][i1].y;
				float vz = mypcl[i][i1].z;
				if (i == 0) {
					vx -= 1.5;
					vy -= 1.5;
				}
				if (i == 1) {
					vx -= 1.5;
					vy += 1.5;
				}
				if (i == 2) {
					vx += 1.5;
					vy -= 1.5;
				}
				if (i == 3) {
					vx += 1.5;
					vy += 1.5;
				}
				vVertex[i1].setX(vx);
				vVertex[i1].setY(vy);
				vVertex[i1].setZ(vz);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 vertex=%1 %2 %3 ").arg(i1).arg(vVertex[i1].x()).arg(vVertex[i1].y()).arg(vVertex[i1].z()); }

				uint8_t cr = mypcl[i][i1].r;
				uint8_t cg = mypcl[i][i1].g;
				uint8_t cb = mypcl[i][i1].b;
				//vColor[i1].setX(cb);
				//vColor[i1].setY(cg);
				//vColor[i1].setZ(cr);
				vColor[i1].setX(cr);
				vColor[i1].setY(cg);
				vColor[i1].setZ(cb);
				//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
			}
			time01.stop();
			//			qDebug() << QString("time=%1  ").arg(time01.getElapsedTimeInMicroSec());

						//		ViewerVertexBuffer1->writeAllBuffer((uchar*)mypcl[i], iNumPoints[i], true);
			glPointSize(3.0);

			ViewerVertexBuffer1->writeVertexBuffer(vVertex, num_elems, filePos, false);

			ViewerVertexBuffer1->writeColorBuffer(vColor, num_elems, filePos, false);

			ViewerVertexBuffer1->writeIndexBuffer(vIndices, num_elems, filePos, false);

			ViewerVertexBuffer1->bindVertexBuffer();
			ViewerVertexBuffer1->bindColorBuffer();
			if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
			else ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, false);
			int uio = 0;
		}
	}
//*/

	glDisable(GL_POLYGON_OFFSET_FILL);

	theGLMessage.stopCPU();
//	float time02 = theGLMessage.getTimeMicrosegCPU();



#ifdef THREADSUB
	doStopMutex.unlock();
#endif
    //ViewerVertexBuffer1->bindVertexBuffer();
    //ViewerVertexBuffer1->bindColorBuffer();
    //ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false);

	//cwipc_point* mypcl[NUMPCL];
	//int iNumPoints[NUMPCL];

	//ViewerVertexBuffer1->bindIndexBufferAll(iNumPoints[0], (char*)mypcl[0], false, false, false);
#endif

#ifdef USE_CUDA_NOTH
    num_elems = MAX_X * MAX_Y;
    qDebug() << QString("num_elems=%1 ").arg(num_elems);
    //    qDebug() << QString("VRtogetherWidget::paintGL        vertexID=%1 colorID=%2 indicesID=%3 ").arg(vertexBuffer->bufferId()).arg(colorBuffer->bufferId()).arg(indicesBuffer->bufferId());

        setVerticesCUDA();
        ViewerVertexBuffer1->bindVertexBuffer();
        ViewerVertexBuffer1->bindColorBuffer();
        ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false);
#endif
#ifdef USE_CUDA_TREADS_ONE
        //unsigned int m = blockIdx.x * blockDim.x + threadIdx.x;
        //unsigned int n = blockIdx.y * blockDim.y + threadIdx.y;

        //if (m < MAX_X && n < MAX_Y2)
        //{
        //	uint iIndex = (n * MAX_X) + m;
        //	uint iIndex1 = iPosStart + (iIndex * 3);
        //	uint iIndex2 = iColStart + (iIndex * 4);
		qDebug() << QString("USE_CUDA_TREADS_ONE  ");
        setVerticesCUDAMulti();
        ViewerVertexBuffer1->bindVertexBuffer();
        ViewerVertexBuffer1->bindColorBuffer();
        ViewerVertexBuffer1->bindIndexBuffer(num_elems, false, false, true);
#endif

//    qDebug() << QString("VRtogetherWidget::paintGL   num_elems=%1  m_fDistEye=%2 ").arg(num_elems).arg(m_fDistEye);
//*/
    //glPopMatrix();

/*
        (myTVMReceiver->*ptfptr)(drawed, myTVMReceiver->myMesh);
        myTVMReceiver->p_RegisterOnReceivedMeshCallBack(clientID, (myTVMReceiver->*tvmptr)(clientID, myTVMReceiver->myMesh));
        qDebug() << QString("nVertices=%1 nTextures=%2 nTriangles=%3 ").arg(myTVMReceiver->myMesh.nVertices).arg(myTVMReceiver->myMesh.nTextures).arg(myTVMReceiver->myMesh.nTriangles);
        myTVMReceiver->p_RegisterOnConnectionErrorCallBack(clientID, (myTVMReceiver->*errorptr)(clientID) );
//*/
}

void SafeMain::resizeGL(int width, int height)
{
    qDebug() << QString("SafeMain::resizeGL  w=%1 h=%2 ...................................... ").arg(width).arg(height);
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    int iW = (width * 0.5) + m_fDistEye;
    int iH = (height * 0.5) + m_fDistEye;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    glFrustum(-1.0, +1.0, -1.0, 1.0, 0.1, 10000.0);
//	glOrtho(0, iW , 0, iH, -5000, 5000);
    glOrtho(-iW, iW, -iH, iH, -5000, 5000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    glTranslated(0.0, 0.0, -40.0);
}

void SafeMain::addIncMov()
{
    incMov += moveUp;
    if (incMov >= 800)moveUp = -20;
    if (incMov <= -800)moveUp = +20;
}

//**************************************************************************

void SafeMain::mousePressEvent(QMouseEvent *event)
{
	setFocus();

    lastPos = event->pos();
	updateGL();

}

void SafeMain::mouseMoveEvent(QMouseEvent *event)
{
	setFocus();
	int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    dx *= 0.25; dy *= 0.25;
    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + dy);// +8 * dy);
        setYRotation(yRot + dx);// +8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        //setXRotation(xRot + dy);// +8 * dy);
        //setZRotation(zRot + dx);// +8 * dx);
		setYElev(-dy);
    }
    lastPos = event->pos();
	updateGL();
}


void SafeMain::wheelEvent(QWheelEvent* ptEvent)
{
	setFocus();
	QMutex doStopMutex;
    doStopMutex.lock();
    setFocus();
    int iNumDegrees = ptEvent->delta() / 8;
    int iDelta = (float)(iNumDegrees / 15);
    if (iDelta > 0)
    {
        incZoom();
    }
    else
    {
        decZoom();
    }
    doStopMutex.unlock();
    updateGL();

}

void SafeMain::keyPressEvent(QKeyEvent *e)
{
	setFocus();
	int iMod = e->modifiers();

    if (e->key() == Qt::Key_Escape) {
        close();
    }
    if (e->key() == Qt::Key_Space) {
    }
    if (iMod == Qt::NoModifier) {
		if (e->key() == Qt::Key_Plus) {
			/** Increments the wheel zoom factor */
				m_fViewportZoomFactor = CONTROLLER_ZOOM_FACTOR;
				m_fDistEye += m_fViewportZoomFactor * 0.1f;
		}

		if (e->key() == Qt::Key_Minus) {
			/** Decremets the wheel zoom factor */
				m_fViewportZoomFactor = -CONTROLLER_ZOOM_FACTOR;
				m_fDistEye += m_fViewportZoomFactor * 0.1f;
		}
		if (e->key() == Qt::Key_O) {
			if (bSeeColor)bSeeColor = false;
			else bSeeColor = true;
			update();
		}

        if (e->key() == Qt::Key_0) {
			qDebug() << QString("Todos los Avatares ");
			glClearColor(1.0, 0.5, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (iAvatar == 0)
				iAvatar = 0xffffffff;
			else
				iAvatar = 0;
			bOnlyOne = false;
			//m_fDistEye = 150;
			m_fDistEye = 700.0;
			//	m_fDistEye = 3150.0;
			yElev = -200;

			update();
        }
        if (e->key() == Qt::Key_1) {
			qDebug() << QString("Avatar 1 ");
            glClearColor(1.0, 0.5, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);

			int iTest = iAvatar & 1;
			qDebug() << QString("iAvatar=%1   iTest=%2 ").arg(QString::number(iAvatar, 16)).arg(iTest);
			if (iTest) {
				qDebug() << QString("Avatar 1 in1  %1").arg(QString::number(iAvatar, 16));
				iAvatar = iAvatar & 0xfffffffffffffffe;
			} else {
				qDebug() << QString("Avatar 1 in2 %1").arg(QString::number(iAvatar, 16));
				iAvatar = iAvatar | 1;
			}
			qDebug() << QString("Avatar 1 out %1").arg(QString::number(iAvatar, 16));
			update();
		}
        if (e->key() == Qt::Key_2) {
            glClearColor(1.0, 0.5, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);



			int iTest = iAvatar & 2;
			if (iTest)
				iAvatar = iAvatar & 0xfffffffd;
			else
				iAvatar = iAvatar | 2;

		}
        if (e->key() == Qt::Key_3) {
            glClearColor(1.0, 0.5, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);


			int iTest = iAvatar & 4;
			if (iTest)
				iAvatar = iAvatar & 0xfffffffb;
			else
				iAvatar = iAvatar | 4;
		}
        if (e->key() == Qt::Key_4) {
            glClearColor(1.0, 0.5, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			int iTest = iAvatar & 8;
			if (iTest)
				iAvatar = iAvatar & 0xfffffff7;
			else
				iAvatar = iAvatar | 8;
		}

		if (e->key() == Qt::Key_A) {
			glClearColor(1.0, 0.5, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);


			int iTest = iAvatar & 1;
			if (iTest)
				iAvatar = iAvatar & 0xfffffffe;
			else
				iAvatar = iAvatar | 1;

			iAvatar = 1;
			bOnlyOne = true;
			m_fDistEye = -150;
			yElev = -200;
		}
		if (e->key() == Qt::Key_B) {
			glClearColor(1.0, 0.5, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);


			int iTest = iAvatar & 2;
			if (iTest)
				iAvatar = iAvatar & 0xfffffffd;
			else
				iAvatar = iAvatar | 2;

			iAvatar = 2;
			bOnlyOne = true;
			m_fDistEye = -150;
			yElev = -200;
		}
		if (e->key() == Qt::Key_C) {
			glClearColor(1.0, 0.5, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			iNumBlocks = 1;
			filePos = 0;
			QVector3D* posns = ViewerVertexBuffer1->getPosns();
			QVector4Du* colors = ViewerVertexBuffer1->getColors();

			// clear the Vertex Buffer
			ViewerVertexBuffer1->clearVertexBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeVertexBuffer(posns, iMaxMem, false);

			// Clear the Color Buffer
			ViewerVertexBuffer1->clearColorBufferMem(iMaxMem, iNumBlocks);
			ViewerVertexBuffer1->writeColorBuffer(colors, iMaxMem, false);


			int iTest = iAvatar & 4;
			if (iTest)
				iAvatar = iAvatar & 0xfffffffb;
			else
				iAvatar = iAvatar | 4;

			iAvatar = 4;
			bOnlyOne = true;
			m_fDistEye = -140;
			yElev = -200;
		}
	}
	if (iMod == Qt::ShiftModifier) {
		if (e->key() == Qt::Key_Up) {
			yElev += 10;
			update();
		}
		if (e->key() == Qt::Key_Down) {
			yElev -= 10;
			update();
		}
	}
    if (iMod == Qt::ControlModifier) {
        if (e->key() == Qt::Key_1) {
            xRot = 225.0;
            yRot = 0.0;
            zRot = 30.0;
            update();
        }
        if (e->key() == Qt::Key_2) {
            xRot = 270.0;
            yRot = 0.0;
            zRot = 0.0;
            update();
        }
        if (e->key() == Qt::Key_3) {
            xRot = 315.0;
            yRot = 0.0;
            zRot = 30.0;
            update();
        }
        if (e->key() == Qt::Key_4) {
            xRot = 180.0;
            yRot = 0.0;
            zRot = 0.0;
            update();
        }
        if (e->key() == Qt::Key_5) {
            xRot = 0.0;
            yRot = 90.0;
            zRot = 0.0;
            update();
        }
        if (e->key() == Qt::Key_6) {
            xRot = 0;
            yRot = 0.0;
            zRot = 0.0;
            update();
        }
        if (e->key() == Qt::Key_7) {
            xRot = 135.0;
            yRot = 0.0;
            zRot = 30.0;
            update();
        }
        if (e->key() == Qt::Key_8) {
            xRot = 270.0;
            yRot = 0.0;
            zRot = 0.0;
            update();
        }
        if (e->key() == Qt::Key_9) {
            xRot = 45.0;
            yRot = 0.0;
            zRot = 30.0;
            update();
        }
    }

    QWidget::keyPressEvent(e);
}

//*/

void SafeMain::setBuffers()
{
    iNumBlocks = NUMVERTICES;
    iMaxMem = 1024 * 1024 * 2;

    iFileMaxSize = 50000;//68000000;//5745000;
    if(iFileMaxSize > (BUFFERMEM * 1) && iFileMaxSize < (BUFFERMEM * 2))iMaxMem = BUFFERMEM * 2;
    if(iFileMaxSize > (BUFFERMEM * 2) && iFileMaxSize < (BUFFERMEM * 4))iMaxMem = BUFFERMEM * 4;
    if(iFileMaxSize > (BUFFERMEM * 4) && iFileMaxSize < (BUFFERMEM * 6))iMaxMem = BUFFERMEM * 6;
//    if(iFileMaxSize > (BUFFERMEM * 6) && iFileMaxSize < (BUFFERMEM * 8))iMaxMem = BUFFERMEM * 8;
//    if(iFileMaxSize > (BUFFERMEM * 8) && iFileMaxSize < (BUFFERMEM * 10))iMaxMem = BUFFERMEM * 10;
//    if(iFileMaxSize > (BUFFERMEM * 10) && iFileMaxSize < (BUFFERMEM * 12))iMaxMem = BUFFERMEM * 12;
//    if(iFileMaxSize > (BUFFERMEM * 12) && iFileMaxSize < (BUFFERMEM * 14))iMaxMem = BUFFERMEM * 14;
//    if(iFileMaxSize > (BUFFERMEM * 14) && iFileMaxSize < (BUFFERMEM * 16))iMaxMem = BUFFERMEM * 16;
//    if(iFileMaxSize > (BUFFERMEM * 16) && iFileMaxSize < (BUFFERMEM * 18))iMaxMem = BUFFERMEM * 18;
//    if(iFileMaxSize > (BUFFERMEM * 18) && iFileMaxSize < (BUFFERMEM * 20))iMaxMem = BUFFERMEM * 20;
//    if(iFileMaxSize > (BUFFERMEM * 20) && iFileMaxSize < (BUFFERMEM * 22))iMaxMem = BUFFERMEM * 22;
//    if(iFileMaxSize > (BUFFERMEM * 22) && iFileMaxSize < (BUFFERMEM * 24))iMaxMem = BUFFERMEM * 24;
//    if(iFileMaxSize > (BUFFERMEM * 24) && iFileMaxSize < (BUFFERMEM * 26))iMaxMem = BUFFERMEM * 26;
//    if(iFileMaxSize > (BUFFERMEM * 26) && iFileMaxSize < (BUFFERMEM * 28))iMaxMem = BUFFERMEM * 28;
//    if(iFileMaxSize > (BUFFERMEM * 28) && iFileMaxSize < (BUFFERMEM * 30))iMaxMem = BUFFERMEM * 30;
//    if(iFileMaxSize > (BUFFERMEM * 30) && iFileMaxSize < (BUFFERMEM * 32))iMaxMem = BUFFERMEM * 32;
//    if(iFileMaxSize > (BUFFERMEM * 32) && iFileMaxSize < (BUFFERMEM * 34))iMaxMem = BUFFERMEM * 34;
//    if(iFileMaxSize > (BUFFERMEM * 34) && iFileMaxSize < (BUFFERMEM * 36))iMaxMem = BUFFERMEM * 36;
//    if(iFileMaxSize > (BUFFERMEM * 36) && iFileMaxSize < (BUFFERMEM * 38))iMaxMem = BUFFERMEM * 38;
//    if(iFileMaxSize > (BUFFERMEM * 38) && iFileMaxSize < (BUFFERMEM * 40))iMaxMem = BUFFERMEM * 40;
//    if(iFileMaxSize > (BUFFERMEM * 40) && iFileMaxSize < (BUFFERMEM * 42))iMaxMem = BUFFERMEM * 42;
//    if(iFileMaxSize > (BUFFERMEM * 42) && iFileMaxSize < (BUFFERMEM * 44))iMaxMem = BUFFERMEM * 44;
//    if(iFileMaxSize > (BUFFERMEM * 44) && iFileMaxSize < (BUFFERMEM * 46))iMaxMem = BUFFERMEM * 46;
//    if(iFileMaxSize > (BUFFERMEM * 46) && iFileMaxSize < (BUFFERMEM * 48))iMaxMem = BUFFERMEM * 48;
//    if(iFileMaxSize > (BUFFERMEM * 48) && iFileMaxSize < (BUFFERMEM * 50))iMaxMem = BUFFERMEM * 50;
//    if(iFileMaxSize > (BUFFERMEM * 50) && iFileMaxSize < (BUFFERMEM * 52))iMaxMem = BUFFERMEM * 52;
//    if(iFileMaxSize > (BUFFERMEM * 52) && iFileMaxSize < (BUFFERMEM * 54))iMaxMem = BUFFERMEM * 54;
//    if(iFileMaxSize > (BUFFERMEM * 54) && iFileMaxSize < (BUFFERMEM * 56))iMaxMem = BUFFERMEM * 56;
//    if(iFileMaxSize > (BUFFERMEM * 56) && iFileMaxSize < (BUFFERMEM * 58))iMaxMem = BUFFERMEM * 58;
//    if(iFileMaxSize > (BUFFERMEM * 58) && iFileMaxSize < (BUFFERMEM * 60))iMaxMem = BUFFERMEM * 60;
//    if(iFileMaxSize > (BUFFERMEM * 60) && iFileMaxSize < (BUFFERMEM * 62))iMaxMem = BUFFERMEM * 62;
//    if(iFileMaxSize > (BUFFERMEM * 62) && iFileMaxSize < (BUFFERMEM * 64))iMaxMem = BUFFERMEM * 64;
//    if(iFileMaxSize > (BUFFERMEM * 64) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 66;
//    if(iFileMaxSize > (BUFFERMEM * 66) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 68;
//    if(iFileMaxSize > (BUFFERMEM * 68) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 70;
//    if(iFileMaxSize > (BUFFERMEM * 70) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 72;
//    if(iFileMaxSize > (BUFFERMEM * 72) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 74;
//    if(iFileMaxSize > (BUFFERMEM * 74) && iFileMaxSize < (BUFFERMEM * 66))iMaxMem = BUFFERMEM * 76;
    //qDebug() << QString("posns_size=%1").arg(iNumBlocks * iMaxMem * 3);
    //qDebug() << QString("colors_size=%1").arg(iNumBlocks * iMaxMem);
    //qDebug() << QString("indices_size=%1").arg(iNumBlocks * iMaxMem);

#ifdef USE_CUDA_TREADS
    int iMaxMem2 = iMaxMem / iThreadsNumber;
    qDebug() << QString("    iFileMaxSize=%1 iNumBlocks=%2 iMaxMem2=%3 iThreadsNumber=%4 " ).arg(iFileMaxSize).arg(iNumBlocks).arg(iMaxMem2).arg(iThreadsNumber);
    qDebug();

    for(int i=0; i<iThreadsNumber; i++){
        ViewerVertexBufferTh[i]->allocVertexBufferMem(iMaxMem2, iNumBlocks);// posns
        ViewerVertexBufferTh[i]->allocColorBufferMem(iMaxMem2, iNumBlocks);//  colors
        ViewerVertexBufferTh[i]->allocIndexBufferMem(iMaxMem2, iNumBlocks);//  indices
    }

#else
//    qDebug() << QString("    iFileMaxSize=%1 iNumBlocks=%2 iMaxMem=%3 sizeQVector3D=%4 " ).arg(iFileMaxSize).arg(iNumBlocks).arg(iMaxMem).arg(sizeof(QVector3D));
//    qDebug();
    ViewerVertexBuffer1->allocVertexBufferMem(iMaxMem + 1024, iNumBlocks);// posns
    ViewerVertexBuffer1->allocColorBufferMem(iMaxMem + 1024, iNumBlocks);//  colors
//	ViewerVertexBuffer1->allocAllBufferMem(iMaxMem + 1024, iNumBlocks);
    ViewerVertexBuffer1->allocIndexBufferMem(iMaxMem + 1024, iNumBlocks);//  indices
#endif

//    qDebug("//********************************************************************** ");
//    ViewerVertexBuffer1->printSizes();
//    qDebug("//********************************************************************** ");

}


void SafeMain::paintEvent(QPaintEvent *event)
{
	//updateGL();

//	QPainter painter(this);
	painter.begin(this);
	QFont serifFont("Times", 10, QFont::Bold);
	painter.setFont(serifFont);

	theGLMessage.setColorTextGL(7);
	float fpscap = 0.0;
	int iNumCapFrame = 0;
	qreal fps = 1.0 / theGLMessage.getTimeSegCPU();
	//	qDebug() << QString("fps=%1").arg(fps);
//	setVars1(100, 1000, 99000);
	theGLMessage.drawGLText(&painter, fps, fpscap, iTotalPoints, iNumCapFrame);
	painter.end();
}

void SafeMain::setVars1(int iVal1, int iVal2, int iVal3)
{
	theGLMessage.setSizePCL1comp(iVal1);
	theGLMessage.setReadedPCL1comp(iVal2);
	theGLMessage.setUncomp1(iVal3);
}

void SafeMain::cudaInfo()
{
#ifdef USE_CUDA
    int deviceCount = 0;
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);

    qDebug() << QString("............................ cudaInfo ................................ ");
    if (error_id != cudaSuccess)
    {
//        printf("cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
//        printf("Result = FAIL\n");
        qDebug() << QString("cudaGetDeviceCount returned %1\n-> %2\n").arg((int)error_id).arg(cudaGetErrorString(error_id));
        qDebug() << QString("Result = FAIL\n");
        exit(EXIT_FAILURE);
    }
    // This function call returns 0 if there are no CUDA capable devices.
    if (deviceCount == 0)
    {
        //printf("There are no available device(s) that support CUDA\n");
        qDebug("There are no available device(s) that support CUDA\n");
    }
    else
    {
//        printf("Detected %d CUDA Capable device(s)\n", deviceCount);
        qDebug() << QString("Detected %1 CUDA Capable device(s)\n").arg(deviceCount);
    }
    int dev, driverVersion = 0, runtimeVersion = 0;

    for (dev = 0; dev < deviceCount; ++dev)
    {
        cudaSetDevice(dev);
        cudaDeviceProp deviceProp;
        cudaGetDeviceProperties(&deviceProp, dev);

//        printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
        qDebug() << QString("\nDevice %1: \"%2\"\n").arg( dev).arg( deviceProp.name);

        // Console log
        cudaDriverGetVersion(&driverVersion);
        cudaRuntimeGetVersion(&runtimeVersion);
//        printf("  CUDA Driver Version / Runtime Version          %d.%d / %d.%d\n", driverVersion/1000, (driverVersion%100)/10, runtimeVersion/1000, (runtimeVersion%100)/10);
//        printf("  CUDA Capability Major/Minor version number:    %d.%d\n", deviceProp.major, deviceProp.minor);
        qDebug() << QString("  CUDA Driver Version / Runtime Version          %1.%2 / %3.%4\n").arg( driverVersion/1000).arg( (driverVersion%100)/10).arg( runtimeVersion/1000).arg( (runtimeVersion%100)/10);
        qDebug() << QString("  CUDA Capability Major/Minor version number:    %1.%2\n").arg( deviceProp.major).arg( deviceProp.minor);


        char msg[256];
        sprintf(msg, "  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n",
                (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        qDebug() << QString("%1").arg( msg);

        qDebug() << QString("  (%1) Multiprocessors, (%2) CUDA Cores/MP:     %3 CUDA Cores\n")
               .arg(deviceProp.multiProcessorCount)
               .arg(_ConvertSMVer2Cores(deviceProp.major, deviceProp.minor))
               .arg(_ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount);
        qDebug() << QString("  GPU Clock rate:                                %1 MHz (%2 GHz)\n").arg( deviceProp.clockRate * 1e-3f).arg( deviceProp.clockRate * 1e-6f);


#if CUDART_VERSION >= 5000
        // This is supported in CUDA 5.0 (runtime API device properties)
        qDebug() << QString("  Memory Clock rate:                             %1 Mhz\n").arg( deviceProp.memoryClockRate * 1e-3f);
        qDebug() << QString("  Memory Bus Width:                              %2-bit\n").arg(deviceProp.memoryBusWidth);

        if (deviceProp.l2CacheSize)
        {
            qDebug() << QString("  L2 Cache Size:                                 %1 bytes\n").arg( deviceProp.l2CacheSize);
        }

#else
        // This only available in CUDA 4.0-4.2 (but these were only exposed in the CUDA Driver API)
        int memoryClock;
        getCudaAttribute<int>(&memoryClock, CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE, dev);
        printf("  Memory Clock rate:                             %.0f Mhz\n", memoryClock * 1e-3f);
        int memBusWidth;
        getCudaAttribute<int>(&memBusWidth, CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH, dev);
        printf("  Memory Bus Width:                              %d-bit\n", memBusWidth);
        int L2CacheSize;
        getCudaAttribute<int>(&L2CacheSize, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE, dev);

        if (L2CacheSize)
        {
            printf("  L2 Cache Size:                                 %d bytes\n", L2CacheSize);
        }

#endif

        qDebug() << QString("  Maximum Texture Dimension Size (x,y,z)         1D=(%1), 2D=(%2, %3), 3D=(%4, %5, %6)\n")
               .arg(deviceProp.maxTexture1D   ).arg( deviceProp.maxTexture2D[0], deviceProp.maxTexture2D[1])
               .arg(deviceProp.maxTexture3D[0]).arg( deviceProp.maxTexture3D[1]).arg( deviceProp.maxTexture3D[2]);
        qDebug() << QString("  Maximum Layered 1D Texture Size, (num) layers  1D=(%1), %2 layers\n")
               .arg(deviceProp.maxTexture1DLayered[0]).arg( deviceProp.maxTexture1DLayered[1]);
        qDebug() << QString("  Maximum Layered 2D Texture Size, (num) layers  2D=(%1, %2), %3 layers\n")
               .arg(deviceProp.maxTexture2DLayered[0]).arg( deviceProp.maxTexture2DLayered[1]).arg( deviceProp.maxTexture2DLayered[2]);


        qDebug() << QString("  Total amount of constant memory:               %1 bytes\n").arg( deviceProp.totalConstMem);
        qDebug() << QString("  Total amount of shared memory per block:       %1 bytes\n").arg( deviceProp.sharedMemPerBlock);
        qDebug() << QString("  Total number of registers available per block: %1\n").arg( deviceProp.regsPerBlock);
        qDebug() << QString("  Warp size:                                     %1\n").arg( deviceProp.warpSize);
        qDebug() << QString("  Maximum number of threads per multiprocessor:  %1\n").arg( deviceProp.maxThreadsPerMultiProcessor);
        qDebug() << QString("  Maximum number of threads per block:           %1\n").arg( deviceProp.maxThreadsPerBlock);
        qDebug() << QString("  Max dimension size of a thread block (x,y,z): (%1, %2, %3)\n")
               .arg(deviceProp.maxThreadsDim[0])
               .arg(deviceProp.maxThreadsDim[1])
               .arg(deviceProp.maxThreadsDim[2]);
        qDebug() << QString("  Max dimension size of a grid size    (x,y,z): (%1, %2, %3)\n")
               .arg(deviceProp.maxGridSize[0])
               .arg(deviceProp.maxGridSize[1])
               .arg(deviceProp.maxGridSize[2]);
        qDebug() << QString("  Maximum memory pitch:                          %1 bytes\n").arg( deviceProp.memPitch);
        qDebug() << QString("  Texture alignment:                             %1 bytes\n").arg( deviceProp.textureAlignment);
        qDebug() << QString("  Concurrent copy and kernel execution:          %1 with %2 copy engine(s)\n").arg( (deviceProp.deviceOverlap ? "Yes" : "No")).arg( deviceProp.asyncEngineCount);
        qDebug() << QString("  Run time limit on kernels:                     %1\n").arg( deviceProp.kernelExecTimeoutEnabled ? "Yes" : "No");
        qDebug() << QString("  Integrated GPU sharing Host Memory:            %1\n").arg( deviceProp.integrated ? "Yes" : "No");
        qDebug() << QString("  Support host page-locked memory mapping:       %1\n").arg( deviceProp.canMapHostMemory ? "Yes" : "No");
        qDebug() << QString("  Alignment requirement for Surfaces:            %1\n").arg( deviceProp.surfaceAlignment ? "Yes" : "No");
        qDebug() << QString("  Device has ECC support:                        %1\n").arg( deviceProp.ECCEnabled ? "Enabled" : "Disabled");
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        qDebug() << QString("  CUDA Device Driver Mode (TCC or WDDM):         %1\n").arg( deviceProp.tccDriver ? "TCC (Tesla Compute Cluster Driver)" : "WDDM (Windows Display Driver Model)");
#endif
        qDebug() << QString("  Device supports Unified Addressing (UVA):      %1\n").arg( deviceProp.unifiedAddressing ? "Yes" : "No");
        qDebug() << QString("  Device PCI Bus ID / PCI location ID:           %1 / %2\n").arg( deviceProp.pciBusID).arg( deviceProp.pciDeviceID);

        const char *sComputeMode[] =
        {
            "Default (multiple host threads can use ::cudaSetDevice() with device simultaneously)",
            "Exclusive (only one host thread in one process is able to use ::cudaSetDevice() with this device)",
            "Prohibited (no host thread can use ::cudaSetDevice() with this device)",
            "Exclusive Process (many threads in one process is able to use ::cudaSetDevice() with this device)",
            "Unknown",
            NULL
        };
        qDebug() << QString("  Compute Mode:\n");
        qDebug() << QString("     < %1 >\n").arg( sComputeMode[deviceProp.computeMode]);
    }


    // csv masterlog info
    // *****************************
    // exe and CUDA driver name
    qDebug() << QString("\n");
    std::string sProfileString = "deviceQuery, CUDA Driver = CUDART";
    char cTemp[16];

    // driver version
    sProfileString += ", CUDA Driver Version = ";
#if defined(_MSC_VER)
    sprintf_s(cTemp, 10, "%d.%d", driverVersion/1000, (driverVersion%100)/10);
#else
    sprintf(cTemp, "%d.%d", driverVersion/1000, (driverVersion%100)/10);
#endif
    sProfileString +=  cTemp;

    // Runtime version
    sProfileString += ", CUDA Runtime Version = ";
#if defined(_MSC_VER)
    sprintf_s(cTemp, 10, "%d.%d", runtimeVersion/1000, (runtimeVersion%100)/10);
#else
    sprintf(cTemp, "%d.%d", runtimeVersion/1000, (runtimeVersion%100)/10);
#endif
    sProfileString +=  cTemp;

    // Device count
    sProfileString += ", NumDevs = ";
#if defined(_MSC_VER)
    sprintf_s(cTemp, 10, "%d", deviceCount);
#else
    sprintf(cTemp, "%d", deviceCount);
#endif
    sProfileString += cTemp;

    // Print Out all device Names
    for (dev = 0; dev < deviceCount; ++dev)
    {
#if defined(_MSC_VER)
        sprintf_s(cTemp, 13, ", Device%d = ", dev);
#else
        sprintf(cTemp, ", Device%d = ", dev);
#endif
        cudaDeviceProp deviceProp;
        cudaGetDeviceProperties(&deviceProp, dev);
        sProfileString += cTemp;
        sProfileString += deviceProp.name;
    }

    sProfileString += "\n";
//    printf("%s", sProfileString.c_str());
    qDebug() << QString("%1").arg(sProfileString.c_str());

    qDebug() << QString("Result = PASS\n");

    // finish
    // cudaDeviceReset causes the driver to clean up all state. While
    // not mandatory in normal operation, it is good practice.  It is also
    // needed to ensure correct operation when the application is being
    // profiled. Calling cudaDeviceReset causes all profile data to be
    // flushed before the application exits
    cudaDeviceReset();

    qDebug() << QString("\n\n");
#endif
}


#ifdef USE_CUDA
/**
 * @brief BezierWidget3x3::setCudaParam
 */
void SafeMain::setCudaParam()
{
    //    int m_iUsize = drawParams->getUsize();
    //    int m_iVsize = drawParams->getVsize();
    qDebug() << QString("VRtogetherWidget::setCudaParam         Start  ,,,,,,,,,,,,,,,,,,,,,,,,,, ");
    CUresult error;
    // Initialize CUDA Driver API
    error = cuInit(0);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }

    setUseCUDA(true);
    // Get number of devices supporting CUDA
    int deviceCount = 0;
    error = cuDeviceGetCount(&deviceCount);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }
    if (deviceCount == 0) {
        qDebug() << QString("There is no device supporting CUDA.\r\n");
        exit(0);
    }
    error = cuDeviceGet(&cuDevice, 0);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to get the first device CUDA.\r\n");
        exit(0);
    }
    // Create context
    error = cuGLCtxCreate(&cuContext, 0, cuDevice);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al crear contexto CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContext);
        cuCtxDestroy(cuContext);
        exit(0);
    }
    qDebug() << QString("VRtogetherWidget::setCudaParam  cuContext=%1 cuDevice=%2 ").arg((ulong)cuContext).arg((ulong)cuDevice);

    // CU_FUNC_CACHE_PREFER_NONE CU_FUNC_CACHE_PREFER_SHARED CU_FUNC_CACHE_PREFER_L1 CU_FUNC_CACHE_PREFER_EQUAL
    error = cuCtxSetCacheConfig(CU_FUNC_CACHE_PREFER_EQUAL);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to Set Cache Config CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContext);
        cuCtxDestroy(cuContext);
        exit(0);
    }
    // D:/i2cat_vrtogether/VRtogetherCUDA/vrtg01_cooda.ptx
#ifdef Q_OS_WIN
    error = cuModuleLoad(&cuModule, "D:/i2cat_vrtogether/VRtogetherCUDA/vrtg01_cooda.ptx");
#else
    error = cuModuleLoad(&cuModule, "/home/juanpinto/QT_OpenCL/qt-labs-opencl_matrix3x3/demos/temp/bezierpatch3x3.ptx");
#endif
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al leer el archivo PTX  error=%1 \r\n").arg(error);
        error = cuCtxDetach(cuContext);
        cuCtxDestroy(cuContext);
        exit(0);
    }
    // _Z7vrtogCUPfPcPif
    // _Z7vrtogCUPfPcPif
    error = cuModuleGetFunction(&vrtogCU, cuModule, "_Z7vrtogCUPfPcPif");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        exit(0);
    }
    /*
        sizeCU = 100 * sizeof(int);
        error= cuMemAlloc(&nothingmemCU, sizeCU);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc nothingmemCU  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            exit(0);
        }

    //*/
    qDebug() << QString("PTX loaded ......................................................... ");
#ifdef DEBUGVR
    sizeCU10 = 10000 * sizeof(int);
    debugmemCUbytes0 = (int *)malloc(sizeCU10);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU0, sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU0, reinterpret_cast<const void *>(debugmemCUbytes0), sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU11 = 10000 * sizeof(int);
    debugmemCUbytes1 = (int *)malloc(sizeCU11);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU1, sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU1, reinterpret_cast<const void *>(debugmemCUbytes1), sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU12 = 10000 * sizeof(int);
    debugmemCUbytes2 = (int *)malloc(sizeCU12);
    memset(debugmemCUbytes2, 0, sizeCU12);
    error = cuMemAlloc(&debugmemCU2, sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU2, reinterpret_cast<const void *>(debugmemCUbytes2), sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU20 = 10000 * sizeof(float);
    debugmemFloatCUbytes0 = (float *)malloc(sizeCU20);
    memset(debugmemFloatCUbytes0, 0, sizeCU20);
    error = cuMemAlloc(&debugmemFloatCU0, sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU0, reinterpret_cast<const void *>(debugmemFloatCUbytes0), sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU21 = 10000 * sizeof(float);
    debugmemFloatCUbytes1 = (float *)malloc(sizeCU21);
    memset(debugmemFloatCUbytes1, 0, sizeCU21);
    error = cuMemAlloc(&debugmemFloatCU1, sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU1, reinterpret_cast<const void *>(debugmemFloatCUbytes1), sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU22 = 10000 * sizeof(float);
    debugmemFloatCUbytes2 = (float *)malloc(sizeCU22);
    memset(debugmemFloatCUbytes2, 0, sizeCU22);
    error = cuMemAlloc(&debugmemFloatCU2, sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU2, reinterpret_cast<const void *>(debugmemFloatCUbytes2), sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU30 = 10000 * sizeof(char);
    debugmemCharCUbytes0 = (char *)malloc(sizeCU30);
    memset(debugmemCharCUbytes0, 0, sizeCU30);
    error = cuMemAlloc(&debugmemCharCU0, sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU0, reinterpret_cast<const void *>(debugmemCharCUbytes0), sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU31 = 10000 * sizeof(char);
    debugmemCharCUbytes1 = (char *)malloc(sizeCU31);
    memset(debugmemCharCUbytes1, 0, sizeCU31);
    error = cuMemAlloc(&debugmemCharCU1, sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU1, reinterpret_cast<const void *>(debugmemCharCUbytes1), sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU32 = 10000 * sizeof(char);
    debugmemCharCUbytes2 = (char *)malloc(sizeCU32);
    memset(debugmemCharCUbytes2, 0, sizeCU32);
    error = cuMemAlloc(&debugmemCharCU2, sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU2, reinterpret_cast<const void *>(debugmemCharCUbytes2), sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
#endif
    //*/
    int n = 0;
    seedmemCUbytes = (int *)malloc(MAX_X * sizeof(int));
    for (int j = 0; j < MAX_X; j++)
    {
        seedmemCUbytes[n] = rand();
        n++;
    }
    error = cuMemAlloc(&seedmemCU, MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(seedmemCU, reinterpret_cast<const void *>(seedmemCUbytes), MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }



	error = cuMemAlloc(&seedmemCU, MAX_X * sizeof(int));
	if (error != CUDA_SUCCESS) {
		qDebug() << QString("Error cuMemAlloc seedmemCU  error=%1 \r\n").arg(error);
		cuCtxDestroy(cuContext);
		return;
	}
	error = cuMemcpyHtoD(seedmemCU, reinterpret_cast<const void *>(seedmemCUbytes), MAX_X * sizeof(int));
	if (error != CUDA_SUCCESS) {
		qDebug() << QString("Error cuMemcpyHtoD seedmemCU  error=%1 \r\n").arg(error);
		cuCtxDestroy(cuContext);
		return;
	}

    //*/

    /*
        int iFrameWidth  = commonParams->getFrameWidth();
        int iFrameHeight = commonParams->getFrameHeight();
        bufferSize = iFrameWidth * iFrameHeight * sizeof(uchar3);
        error= cuMemAlloc(&bufferEffects1cu, bufferSize);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc bufferEffects1cu  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            return;
        }
        error= cuMemAlloc(&bufferEffects2cu, bufferSize);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc bufferEffects1cu  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            return;
        }
    //*/
    //    setPBOcu();
    qDebug() << QString("VRtogetherWidget::setCudaParam         End ");
    qDebug() << QString("deviceCount=%1 \r\n").arg(deviceCount);
}
#endif

#ifdef USE_CUDA
/**
 * @brief BezierWidget3x3::setCudaParamMulti
 */
void SafeMain::setCudaParamMulti()
{
    //    int m_iUsize = drawParams->getUsize();
    //    int m_iVsize = drawParams->getVsize();
    qDebug() << QString("VRtogetherWidget::setCudaParamMulti         Start  ,,,,,,,,,,,,,,,,,,,,,,,,,, ");
    CUresult error;
    // Initialize CUDA Driver API
    error = cuInit(0);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }

    setUseCUDA(true);
    // Get number of devices supporting CUDA
    int deviceCount = 0;
    error = cuDeviceGetCount(&deviceCount);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }
    if (deviceCount == 0) {
        qDebug() << QString("There is no device supporting CUDA.\r\n");
        exit(0);
    }
    error = cuDeviceGet(&cuDevice, 0);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to get the first device CUDA.\r\n");
        exit(0);
    }
    // Create context
    error = cuGLCtxCreate(&cuContextMulti, 0, cuDevice);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al crear contexto CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContextMulti);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }

    qDebug() << QString("VRtogetherWidget::setCudaParamMulti  cuContextMulti=%1 cuDevice=%2 ").arg((uint)cuContextMulti).arg((uint)cuDevice);


    // CU_FUNC_CACHE_PREFER_NONE CU_FUNC_CACHE_PREFER_SHARED CU_FUNC_CACHE_PREFER_L1 CU_FUNC_CACHE_PREFER_EQUAL
    error = cuCtxSetCacheConfig(CU_FUNC_CACHE_PREFER_EQUAL);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to Set Cache Config CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContextMulti);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }
    // D:/i2cat_vrtogether/VRtogetherCUDA/vrtg01_cooda.ptx
#ifdef Q_OS_WIN
    error = cuModuleLoad(&cuModule, "D:/i2cat_vrtogether/v30_stable_qt5/getSUB/vrtg01_cooda.ptx");
#else
    error = cuModuleLoad(&cuModule, "/home/juanpinto/QT_OpenCL/qt-labs-opencl_matrix3x3/demos/temp/bezierpatch3x3.ptx");
#endif
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al leer el archivo PTX  error=%1 \r\n").arg(error);
        error = cuCtxDetach(cuContextMulti);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }

    error = cuModuleGetFunction(&vrtogCU01, cuModule, "_Z9vrtogCU01PfPcPifjj");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU01   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }
    error = cuModuleGetFunction(&vrtogCU02, cuModule, "_Z9vrtogCU02PfPcPifjj");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU02   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }
    error = cuModuleGetFunction(&vrtogCU03, cuModule, "_Z9vrtogCU03PfPcPifjj");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU03   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }
    error = cuModuleGetFunction(&vrtogCU04, cuModule, "_Z9vrtogCU04PfPcPifjj");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU04   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        exit(0);
    }


    //*/
    qDebug() << QString("PTX loaded ......................................................... ");
#ifdef DEBUGVR
    sizeCU10 = 10000 * sizeof(int);
    debugmemCUbytes0 = (int *)malloc(sizeCU10);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU0, sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU0, reinterpret_cast<const void *>(debugmemCUbytes0), sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU11 = 10000 * sizeof(int);
    debugmemCUbytes1 = (int *)malloc(sizeCU11);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU1, sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU1, reinterpret_cast<const void *>(debugmemCUbytes1), sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU12 = 10000 * sizeof(int);
    debugmemCUbytes2 = (int *)malloc(sizeCU12);
    memset(debugmemCUbytes2, 0, sizeCU12);
    error = cuMemAlloc(&debugmemCU2, sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU2, reinterpret_cast<const void *>(debugmemCUbytes2), sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU20 = 10000 * sizeof(float);
    debugmemFloatCUbytes0 = (float *)malloc(sizeCU20);
    memset(debugmemFloatCUbytes0, 0, sizeCU20);
    error = cuMemAlloc(&debugmemFloatCU0, sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU0, reinterpret_cast<const void *>(debugmemFloatCUbytes0), sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU21 = 10000 * sizeof(float);
    debugmemFloatCUbytes1 = (float *)malloc(sizeCU21);
    memset(debugmemFloatCUbytes1, 0, sizeCU21);
    error = cuMemAlloc(&debugmemFloatCU1, sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU1, reinterpret_cast<const void *>(debugmemFloatCUbytes1), sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU22 = 10000 * sizeof(float);
    debugmemFloatCUbytes2 = (float *)malloc(sizeCU22);
    memset(debugmemFloatCUbytes2, 0, sizeCU22);
    error = cuMemAlloc(&debugmemFloatCU2, sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU2, reinterpret_cast<const void *>(debugmemFloatCUbytes2), sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU30 = 10000 * sizeof(char);
    debugmemCharCUbytes0 = (char *)malloc(sizeCU30);
    memset(debugmemCharCUbytes0, 0, sizeCU30);
    error = cuMemAlloc(&debugmemCharCU0, sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU0, reinterpret_cast<const void *>(debugmemCharCUbytes0), sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU31 = 10000 * sizeof(char);
    debugmemCharCUbytes1 = (char *)malloc(sizeCU31);
    memset(debugmemCharCUbytes1, 0, sizeCU31);
    error = cuMemAlloc(&debugmemCharCU1, sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU1, reinterpret_cast<const void *>(debugmemCharCUbytes1), sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU32 = 10000 * sizeof(char);
    debugmemCharCUbytes2 = (char *)malloc(sizeCU32);
    memset(debugmemCharCUbytes2, 0, sizeCU32);
    error = cuMemAlloc(&debugmemCharCU2, sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU2, reinterpret_cast<const void *>(debugmemCharCUbytes2), sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
#endif
    //*/
    int n = 0;
    seedmemCUbytes = (int *)malloc(MAX_X * sizeof(int));
    for (int j = 0; j < MAX_X; j++)
    {
        seedmemCUbytes[n] = rand();
        n++;
    }
    error = cuMemAlloc(&seedmemCU, MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        return;
    }
    error = cuMemcpyHtoD(seedmemCU, reinterpret_cast<const void *>(seedmemCUbytes), MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextMulti);
        return;
    }
    //*/

    /*
        int iFrameWidth  = commonParams->getFrameWidth();
        int iFrameHeight = commonParams->getFrameHeight();
        bufferSize = iFrameWidth * iFrameHeight * sizeof(uchar3);
        error= cuMemAlloc(&bufferEffects1cu, bufferSize);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc bufferEffects1cu  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            return;
        }
        error= cuMemAlloc(&bufferEffects2cu, bufferSize);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc bufferEffects1cu  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            return;
        }
    //*/
    //    setPBOcu();
    qDebug() << QString("VRtogetherWidget::setCudaParamMulti         End ");
    qDebug() << QString("deviceCount=%1 \r\n").arg(deviceCount);
}
#endif


void SafeMain::setVerticesCUDA()
{
#ifdef USE_CUDA
    int n = 0;
    bool useCUDA = getUseCUDA();

    if (useCUDA == true) {
                qDebug() << QString("VRtogetherWidget::setVerticesCUDA ............... incMov=%1 ").arg(incMov);
        gmr = cuGraphicsMapResources(1, &positionCUDABuffer, 0);
        gmr = cuGraphicsMapResources(1, &posColorCUDABuffer, 0);

        cuGraphicsResourceGetMappedPointer(&pDevPtr[0], &num_bytes[0], positionCUDABuffer);
        cuGraphicsResourceGetMappedPointer(&pDevPtr[1], &num_bytes[1], posColorCUDABuffer);
		cuGraphicsResourceGetMappedPointer(&pDevPtr[2], &num_bytes[2], indicesCUDABuffer);

        uint argTestSize;
        void *argsTest[64];
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];       n++;
        argsTest[n] = &pDevPtr[1];       n++;
		argsTest[n] = &pDevPtr[2];       n++;
		argsTest[n] = &seedmemCU;        n++;
        argsTest[n] = &incMov;           n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);
        //		void *config[] = {
        //			CU_LAUNCH_PARAM_BUFFER_POINTER, argsTest,
        //			CU_LAUNCH_PARAM_BUFFER_SIZE,    &argTestSize,
        //			CU_LAUNCH_PARAM_END
        //		};


        int iValXocl = MAX_X;
        int iValYocl = MAX_Y;
        unsigned int blqY = iValYocl / block_size;
        unsigned int blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        gmr = cuLaunchKernel(vrtogCU, blqX, blqY, 1,
            block_size, block_size, 1,
            0, NULL, argsTest, NULL);
        if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDA Error cuLaunchKernel=%1").arg(gmr); }
        cuGraphicsUnmapResources(1, &positionCUDABuffer, 0);
        cuGraphicsUnmapResources(1, &posColorCUDABuffer, 0);
		cuGraphicsUnmapResources(1, &indicesCUDABuffer, 0);
        //        theGLMessage.stopCUDA();

#ifdef DEBUGVR
        // copy result from device to host
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes0, debugmemCU0, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error10 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes1, debugmemCU1, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error11 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes2, debugmemCU2, sizeCU10);
        if (gmr != CUDA_SUCCESS)
        {
            qDebug() << QString("setVerticesCUDA  Error12 cuMemcpyDtoH=%1").arg(gmr);
        }

        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes0, debugmemFloatCU0, sizeCU20);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error20 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes1, debugmemFloatCU1, sizeCU21);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error21 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes2, debugmemFloatCU2, sizeCU22);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error22 cuMemcpyDtoH=%1").arg(gmr);
        }


        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes0, debugmemCharCU0, sizeCU30);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error30 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes1, debugmemCharCU1, sizeCU31);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error31 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes2, debugmemCharCU2, sizeCU32);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error32 cuMemcpyDtoH=%1").arg(gmr);
        }
#endif

        //        for (int i = 1024; i < 1024+16; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 ").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i]);
        //        }
        //        qDebug("\n");

        //                for (int i = 0; i < 32; i++) {
        //                    qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 ").arg(i)
        //                                .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                                .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i]);
        //                }
        //                qDebug("\n");

        //        for (int i = 0; i < 32; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 char=%8 %9 %10").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                        .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i])
        //                        .arg((uchar)debugmemCharCUbytes0[i]).arg((uchar)debugmemCharCUbytes1[i]).arg((uchar)debugmemCharCUbytes2[i]);
        //        }
        //        qDebug("\n");
    }
#endif
}

void SafeMain::freeVerticesCUDA()
{
    qDebug("Deallocating Memory........................");

#ifdef USE_CUDA
    qDebug("Freeing CUDA  buffers ");
    cuGraphicsUnregisterResource(positionCUDABuffer);
    cuGraphicsUnregisterResource(posColorCUDABuffer);
	cuGraphicsUnregisterResource(indicesCUDABuffer);
#endif

    m_iNumVertices = 0;
    qDebug("Deallocated Memory........................");
}

void SafeMain::setVerticesCUDAMulti()
{
#ifdef USE_CUDA
    int n = 0;
    ulong ulPosStart = 0;
    ulong ulColStart = 0;
    bool useCUDA = getUseCUDA();

    if (useCUDA == true) {
        qDebug() << QString("VRtogetherWidget::setVerticesCUDAMulti ............... incMov=%1 ").arg(incMov);
        gmr = cuGraphicsMapResources(1, &positionCUDABuffer, 0);
        gmr = cuGraphicsMapResources(1, &posColorCUDABuffer, 0);
		gmr = cuGraphicsMapResources(1, &indicesCUDABuffer, 0);

        cuGraphicsResourceGetMappedPointer(&pDevPtr[0], &num_bytes[0], positionCUDABuffer);
        cuGraphicsResourceGetMappedPointer(&pDevPtr[1], &num_bytes[1], posColorCUDABuffer);
		cuGraphicsResourceGetMappedPointer(&pDevPtr[2], &num_bytes[2], indicesCUDABuffer);


		QGLBuffer *vertexBuffer = ViewerVertexBuffer1->getVertexBuffer();
		QGLBuffer *colorBuffer = ViewerVertexBuffer1->getColorBuffer();
		QGLBuffer *indexBuffer = ViewerVertexBuffer1->getIndexBuffer();
		if (vertexBuffer) {
			vertexBuffer->bind();
			glVertexPointer(3, GL_FLOAT, 0, 0);
			QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
			if (mappedV) {
				for (int q = 0; q < 32; q++)//iNumIndices; q++)
				{
					//  if((q%3) == 0)qDebug(" \n");
				  //    if((q % 4) == 0)qDebug() << " \n";
					qDebug() << QString("GLVertexBuffers::bindVertexBuffer    q=%0 vertex=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
				}
			}
			vertexBuffer->unmap();
			vertexBuffer->release();
		}
		pDevPtr[0][0];


        uint argTestSize;
        void *argsTest[64];
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];       n++;
        argsTest[n] = &pDevPtr[1];       n++;
		argsTest[n] = &pDevPtr[2];		 n++;
        argsTest[n] = &seedmemCU;        n++;
        argsTest[n] = &incMov;           n++;
        argsTest[n] = &ulPosStart;           n++;
        argsTest[n] = &ulColStart;           n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);
        //		void *config[] = {
        //			CU_LAUNCH_PARAM_BUFFER_POINTER, argsTest,
        //			CU_LAUNCH_PARAM_BUFFER_SIZE,    &argTestSize,
        //			CU_LAUNCH_PARAM_END
        //		};

//		glTranslatef(200, -100, 0);

        int iValXocl = MAX_X;
        int iValYocl = MAX_Y / 4;
        unsigned int blqY = iValYocl / block_size;
        unsigned int blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        qDebug() << QString("setVerticesCUDAMulti  1  iValYocl=%1 iValXocl=%2 blqY=%3 bkqX=%4 block_size=%5 ulPosStart=%6 ulColStart=%7 ")
            .arg(iValYocl).arg(iValXocl).arg(blqY).arg(blqX).arg(block_size).arg(ulPosStart).arg(ulColStart);
        if (!iAvatar || iAvatar == 1) {
            gmr = cuLaunchKernel(vrtogCU01, blqX, blqY, 1,
                block_size, block_size, 1,
                0, NULL, argsTest, NULL);
            if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDAMulti  1  Error cuLaunchKernel=%1").arg(gmr); }
        }


        ulPosStart = iValYocl * iValXocl;
        ulColStart = ulPosStart;
        n = 0;
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];      n++;
        argsTest[n] = &pDevPtr[1];      n++;
		argsTest[n] = &pDevPtr[2];		n++;
        argsTest[n] = &seedmemCU;       n++;
        argsTest[n] = &incMov;          n++;
        argsTest[n] = &ulPosStart;      n++;
        argsTest[n] = &ulColStart;      n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);

        iValXocl = MAX_X;
        iValYocl = MAX_Y / 4;
        blqY = iValYocl / block_size;
        blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        qDebug() << QString("setVerticesCUDAMulti  2  iValYocl=%1 iValXocl=%2 blqY=%3 bkqX=%4 block_size=%5 ulPosStart=%6 ulColStart=%7 ")
            .arg(iValYocl).arg(iValXocl).arg(blqY).arg(blqX).arg(block_size).arg(ulPosStart).arg(ulColStart);
        if (!iAvatar || iAvatar == 2) {
            gmr = cuLaunchKernel(vrtogCU02, blqX, blqY, 1,
                block_size, block_size, 1,
                0, NULL, argsTest, NULL);
            if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDAMulti  2  Error cuLaunchKernel=%1").arg(gmr); }
        }



        ulPosStart = (iValYocl * iValXocl) * 2;
        ulColStart = ulPosStart;
        n = 0;
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];       n++;
        argsTest[n] = &pDevPtr[1];       n++;
		argsTest[n] = &pDevPtr[2];       n++;
        argsTest[n] = &seedmemCU;        n++;
        argsTest[n] = &incMov;           n++;
        argsTest[n] = &ulPosStart;           n++;
        argsTest[n] = &ulColStart;           n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);

        iValXocl = MAX_X;
        iValYocl = MAX_Y / 4;
        blqY = iValYocl / block_size;
        blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        qDebug() << QString("setVerticesCUDAMulti  3  iValYocl=%1 iValXocl=%2 blqY=%3 bkqX=%4 block_size=%5 ulPosStart=%6 ulColStart=%7 ")
            .arg(iValYocl).arg(iValXocl).arg(blqY).arg(blqX).arg(block_size).arg(ulPosStart).arg(ulColStart);
        if (!iAvatar || iAvatar == 3) {
            gmr = cuLaunchKernel(vrtogCU03, blqX, blqY, 1,
                block_size, block_size, 1,
                0, NULL, argsTest, NULL);
            if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDAMulti  3  Error cuLaunchKernel=%1").arg(gmr); }
        }



        ulPosStart = (iValYocl * iValXocl) * 3;
        ulColStart = ulPosStart;
        n = 0;
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];       n++;
        argsTest[n] = &pDevPtr[1];       n++;
		argsTest[n] = &pDevPtr[2];       n++;
		argsTest[n] = &seedmemCU;        n++;
        argsTest[n] = &incMov;           n++;
        argsTest[n] = &ulPosStart;           n++;
        argsTest[n] = &ulColStart;           n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);

//		glTranslatef(-200, 100, 0);


        iValXocl = MAX_X;
        iValYocl = MAX_Y / 4;
        blqY = iValYocl / block_size;
        blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        qDebug() << QString("setVerticesCUDAMulti  4  iValYocl=%1 iValXocl=%2 blqY=%3 bkqX=%4 block_size=%5 ulPosStart=%6 ulColStart=%7 ")
            .arg(iValYocl).arg(iValXocl).arg(blqY).arg(blqX).arg(block_size).arg(ulPosStart).arg(ulColStart);
        if (!iAvatar || iAvatar == 4) {
            gmr = cuLaunchKernel(vrtogCU04, blqX, blqY, 1,
                block_size, block_size, 1,
                0, NULL, argsTest, NULL);
            if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDAMulti  4  Error cuLaunchKernel=%1").arg(gmr); }
        }



//*/
//*****************************************************************************************************************************

        cuGraphicsUnmapResources(1, &positionCUDABuffer, 0);
        cuGraphicsUnmapResources(1, &posColorCUDABuffer, 0);
		cuGraphicsUnmapResources(1, &indicesCUDABuffer, 0);

        //        theGLMessage.stopCUDA();

#ifdef DEBUGVR
        // copy result from device to host
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes0, debugmemCU0, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error10 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes1, debugmemCU1, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error11 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes2, debugmemCU2, sizeCU10);
        if (gmr != CUDA_SUCCESS)
        {
            qDebug() << QString("setVerticesCUDA  Error12 cuMemcpyDtoH=%1").arg(gmr);
        }

        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes0, debugmemFloatCU0, sizeCU20);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error20 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes1, debugmemFloatCU1, sizeCU21);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error21 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes2, debugmemFloatCU2, sizeCU22);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error22 cuMemcpyDtoH=%1").arg(gmr);
        }


        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes0, debugmemCharCU0, sizeCU30);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error30 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes1, debugmemCharCU1, sizeCU31);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error31 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes2, debugmemCharCU2, sizeCU32);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error32 cuMemcpyDtoH=%1").arg(gmr);
        }
#endif

        //        for (int i = 1024; i < 1024+16; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 ").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i]);
        //        }
        //        qDebug("\n");

        //                for (int i = 0; i < 32; i++) {
        //                    qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 ").arg(i)
        //                                .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                                .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i]);
        //                }
        //                qDebug("\n");

        //        for (int i = 0; i < 32; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 char=%8 %9 %10").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                        .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i])
        //                        .arg((uchar)debugmemCharCUbytes0[i]).arg((uchar)debugmemCharCUbytes1[i]).arg((uchar)debugmemCharCUbytes2[i]);
        //        }
        //        qDebug("\n");
    }
#endif
}

void SafeMain::allocVerticesCUDA()
{
#ifdef USE_CUDA
    IndexType *indices = ViewerVertexBuffer1->getIndices();

    int iIndex = 0;
    for (int i = 0; i < MAX_Y; i++) {
        for (int j = 0; j < MAX_X; j++) {
            indices[iIndex] = IndexType(iIndex);
            iIndex++;
        }
    }
    num_elems = iIndex;
    m_iNumVertices = iIndex;
    m_iNumVerticesPerThread = m_iNumVertices / iThreadsNumber;
    qDebug() << QString("VRtogetherWidget::allocVerticesCUDA ............................... m_iNumVertices=%1  m_iNumVerticesPerThread=%2").arg(m_iNumVertices).arg(m_iNumVerticesPerThread);

    filePos = 0;
    ViewerVertexBuffer1->writeIndexBuffer(indices, num_elems, false);


    qDebug() << QString("VRtogetherWidget::allocVerticesCUDA        Allocating CUDA Buffers...");
    QGLBuffer *vertexBuffer = ViewerVertexBuffer1->getVertexBuffer();
    QGLBuffer *colorBuffer = ViewerVertexBuffer1->getColorBuffer();
	QGLBuffer *indexBuffer = ViewerVertexBuffer1->getIndexBuffer();
    qDebug() << QString("VRtogetherWidget::allocVerticesCUDA        Setting CUDA interoperability .....");
    qDebug() << QString("VRtogetherWidget::allocVerticesCUDA        vertexID=%1 colorID=%2 ").arg(vertexBuffer->bufferId()).arg(colorBuffer->bufferId());
    cuGraphicsGLRegisterBuffer(&positionCUDABuffer, vertexBuffer->bufferId(), CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
    cuGraphicsGLRegisterBuffer(&posColorCUDABuffer, colorBuffer->bufferId(), CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
	cuGraphicsGLRegisterBuffer(&indicesCUDABuffer, indexBuffer->bufferId(), CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
    qDebug() << QString("VRtogetherWidget::allocVerticesCUDA        Finished CUDA interoperability setting .....");
#endif

    setAllocated(true);
}

#ifdef USE_CUDA
/**
 * @brief BezierWidget3x3::setCudaParamThreads
 */
void SafeMain::setCudaParamThreads()
{
    //    int m_iUsize = drawParams->getUsize();
    //    int m_iVsize = drawParams->getVsize();
    qDebug() << QString("VRtogetherWidget::setCudaParamThreads         Start  ,,,,,,,,,,,,,,,,,,,,,,,,,, ");
    CUresult error;
    // Initialize CUDA Driver API
    error = cuInit(0);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }

    setUseCUDA(true);
    // Get number of devices supporting CUDA
    int deviceCount = 0;
    error = cuDeviceGetCount(&deviceCount);
    if (error != CUDA_SUCCESS) { qDebug() << QString("Error al inicializar el CUDA \r\n"); exit(0); }
    if (deviceCount == 0) {
        qDebug() << QString("There is no device supporting CUDA.\r\n");
        exit(0);
    }
    error = cuDeviceGet(&cuDevice, 0);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to get the first device CUDA.\r\n");
        exit(0);
    }

    // Create context
    error = cuCtxCreate(&cuContextThread, 0, cuDevice);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al crear contexto CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContextThread);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }
    qDebug() << QString("VRtogetherWidget::setCudaParamThreads   cuContext=%1   cuDevice=%2 ").arg((uint)cuContextThread).arg((uint)cuDevice);
    HGLRC myContext1 = wglGetCurrentContext();
    qDebug() << QString("VRtogetherWidget::setCudaParamThreads  glContext1=%2   cuDevice=%1 ").arg(cuDevice).arg((qlonglong)myContext1);
    CUcontext   cuContextT; cuCtxGetCurrent(&cuContextT);
    qDebug() << QString("VRtogetherWidget::setCudaParamThreads   cuCtxGetCurrent=%1   ").arg((ulong)cuContextT);

    // CU_FUNC_CACHE_PREFER_NONE CU_FUNC_CACHE_PREFER_SHARED CU_FUNC_CACHE_PREFER_L1 CU_FUNC_CACHE_PREFER_EQUAL
    error = cuCtxSetCacheConfig(CU_FUNC_CACHE_PREFER_EQUAL);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error to Set Cache Config CUDA %1  error=%2 \r\n").arg(cuDevice).arg(error);
        error = cuCtxDetach(cuContextThread);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }
    // D:/i2cat_vrtogether/VRtogetherCUDA/vrtg01_cooda.ptx
#ifdef Q_OS_WIN
    error = cuModuleLoad(&cuModule, "D:/i2cat_vrtogether/VRtogetherCUDA/vrtg01_cooda.ptx");
#else
    error = cuModuleLoad(&cuModule, "/home/juanpinto/QT_OpenCL/qt-labs-opencl_matrix3x3/demos/temp/bezierpatch3x3.ptx");
#endif
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al leer el archivo PTX  error=%1 \r\n").arg(error);
        error = cuCtxDetach(cuContextThread);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }
    // _Z7vrtogCUPfPcPif
    // _Z7vrtogCUPfPcPif
    error = cuModuleGetFunction(&vrtogCU, cuModule, "_Z7vrtogCUPfPcPif");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }
    error = cuModuleGetFunction(&vrtogCU01, cuModule, "_Z9vrtogCU01PfPcPifjj");
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error al cargar funcion vrtogCU01   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }

    //error = cuMemAlloc(&pDevPtr[0], MAX_X * (MAX_Y / 4) * 3 * sizeof(float));
    //if (error != CUDA_SUCCESS) {
    //	qDebug() << QString("Error cuMemAlloc pDevPtr[0]  error=%1 \r\n").arg(error);
    //	cuCtxDestroy(cuContext);
    //	return;
    //}

    /*
        sizeCU = 100 * sizeof(int);
        error= cuMemAlloc(&nothingmemCU, sizeCU);
        if (error != CUDA_SUCCESS){
            qDebug() << QString("Error cuMemAlloc nothingmemCU  error=%1 \r\n").arg(error);
            cuCtxDestroy(cuContext);
            exit(0);
        }

    //*/
    qDebug() << QString("PTX loaded ......................................................... ");


#ifdef DEBUGVR
    sizeCU10 = 10000 * sizeof(int);
    debugmemCUbytes0 = (int *)malloc(sizeCU10);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU0, sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU0, reinterpret_cast<const void *>(debugmemCUbytes0), sizeCU10);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU11 = 10000 * sizeof(int);
    debugmemCUbytes1 = (int *)malloc(sizeCU11);
    memset(debugmemCUbytes0, 0, sizeCU10);
    error = cuMemAlloc(&debugmemCU1, sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU1, reinterpret_cast<const void *>(debugmemCUbytes1), sizeCU11);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU12 = 10000 * sizeof(int);
    debugmemCUbytes2 = (int *)malloc(sizeCU12);
    memset(debugmemCUbytes2, 0, sizeCU12);
    error = cuMemAlloc(&debugmemCU2, sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCU2, reinterpret_cast<const void *>(debugmemCUbytes2), sizeCU12);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU20 = 10000 * sizeof(float);
    debugmemFloatCUbytes0 = (float *)malloc(sizeCU20);
    memset(debugmemFloatCUbytes0, 0, sizeCU20);
    error = cuMemAlloc(&debugmemFloatCU0, sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU0, reinterpret_cast<const void *>(debugmemFloatCUbytes0), sizeCU20);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU21 = 10000 * sizeof(float);
    debugmemFloatCUbytes1 = (float *)malloc(sizeCU21);
    memset(debugmemFloatCUbytes1, 0, sizeCU21);
    error = cuMemAlloc(&debugmemFloatCU1, sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU1, reinterpret_cast<const void *>(debugmemFloatCUbytes1), sizeCU21);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU22 = 10000 * sizeof(float);
    debugmemFloatCUbytes2 = (float *)malloc(sizeCU22);
    memset(debugmemFloatCUbytes2, 0, sizeCU22);
    error = cuMemAlloc(&debugmemFloatCU2, sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemFloatCU2, reinterpret_cast<const void *>(debugmemFloatCUbytes2), sizeCU22);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemFloatCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }

    sizeCU30 = 10000 * sizeof(char);
    debugmemCharCUbytes0 = (char *)malloc(sizeCU30);
    memset(debugmemCharCUbytes0, 0, sizeCU30);
    error = cuMemAlloc(&debugmemCharCU0, sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU0, reinterpret_cast<const void *>(debugmemCharCUbytes0), sizeCU30);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU0  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU31 = 10000 * sizeof(char);
    debugmemCharCUbytes1 = (char *)malloc(sizeCU31);
    memset(debugmemCharCUbytes1, 0, sizeCU31);
    error = cuMemAlloc(&debugmemCharCU1, sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU1, reinterpret_cast<const void *>(debugmemCharCUbytes1), sizeCU31);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU1  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    sizeCU32 = 10000 * sizeof(char);
    debugmemCharCUbytes2 = (char *)malloc(sizeCU32);
    memset(debugmemCharCUbytes2, 0, sizeCU32);
    error = cuMemAlloc(&debugmemCharCU2, sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
    error = cuMemcpyHtoD(debugmemCharCU2, reinterpret_cast<const void *>(debugmemCharCUbytes2), sizeCU32);
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD debugmemCharCU2  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContext);
        return;
    }
#endif
    //*/
    int n = 0;
    seedmemCUbytes = (int *)malloc(MAX_X * sizeof(int));
    for (int j = 0; j < MAX_X; j++)
    {
        seedmemCUbytes[n] = rand();
        n++;
    }
    error = cuMemAlloc(&seedmemCU, MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemAlloc seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextThread);
        return;
    }
    error = cuMemcpyHtoD(seedmemCU, reinterpret_cast<const void *>(seedmemCUbytes), MAX_X * sizeof(int));
    if (error != CUDA_SUCCESS) {
        qDebug() << QString("Error cuMemcpyHtoD seedmemCU  error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextThread);
        return;
    }
    //*/

    // Here we must release the CUDA context from the thread context
    error = cuCtxPopCurrent(NULL);

    if (CUDA_SUCCESS != error)
    {
        qDebug() << QString("cuCtxPopCurrent failed   error=%1 \r\n").arg(error);
        cuCtxDestroy(cuContextThread);
        exit(0);
    }

    qDebug() << QString("VRtogetherWidget::setCudaParamThreads         End ");
    qDebug() << QString("deviceCount=%1 \r\n").arg(deviceCount);

}
#endif

void SafeMain::setVerticesCUDAThreads(uint uThID)
{
    Q_UNUSED(uThID);
#ifdef USE_CUDAaaa
    int n = 0;
    ulong ulPosStart = 0;
    ulong ulColStart = 0;
    bool useCUDA = getUseCUDA();

    if (useCUDA == true) {
        gmr = cuCtxPushCurrent(cuContext);
        if (gmr != CUDA_SUCCESS) { qDebug() << QString("setVerticesCUDAThreads Error cuCtxPushCurrent=%1").arg(gmr); }

        qDebug() << QString("VRtogetherWidget::setVerticesCUDAThreads ............... incMov=%1 ").arg(incMov);
        gmr = cuGraphicsMapResources(1, &positionCUDABufferTh[uThID], 0);
        gmr = cuGraphicsMapResources(1, &posColorCUDABufferTh[uThID], 0);

        cuGraphicsResourceGetMappedPointer(&pDevPtr[0], &num_bytes[0], positionCUDABufferTh[uThID]);
        cuGraphicsResourceGetMappedPointer(&pDevPtr[1], &num_bytes[1], posColorCUDABufferTh[uThID]);

        uint argTestSize;
        void *argsTest[64];
        memset(argsTest, 0, sizeof(argsTest));

        argsTest[n] = &pDevPtr[0];       n++;
        argsTest[n] = &pDevPtr[1];       n++;
        argsTest[n] = &seedmemCU;        n++;
        argsTest[n] = &incMov;           n++;
        argsTest[n] = &ulPosStart;           n++;
        argsTest[n] = &ulColStart;           n++;
#ifdef DEBUGVR
        argsTest[n] = &debugmemCU0;       n++;
        argsTest[n] = &debugmemCU1;       n++;
        argsTest[n] = &debugmemCU2;       n++;
        argsTest[n] = &debugmemFloatCU0;  n++;
        argsTest[n] = &debugmemFloatCU1;  n++;
        argsTest[n] = &debugmemFloatCU2;  n++;
        argsTest[n] = &debugmemCharCU0;   n++;
        argsTest[n] = &debugmemCharCU1;   n++;
        argsTest[n] = &debugmemCharCU2;   n++;
#endif
        argTestSize = sizeof(argsTest);
        //		void *config[] = {
        //			CU_LAUNCH_PARAM_BUFFER_POINTER, argsTest,
        //			CU_LAUNCH_PARAM_BUFFER_SIZE,    &argTestSize,
        //			CU_LAUNCH_PARAM_END
        //		};


        int iValXocl = MAX_X;
        int iValYocl = MAX_Y / 4;
        unsigned int blqY = iValYocl / block_size;
        unsigned int blqX = iValXocl / block_size;
        if (((int)blqX * block_size) < iValXocl)blqX++;
        if (((int)blqY * block_size) < iValYocl)blqY++;
        gmr = cuLaunchKernel(vrtogCU01, blqX, blqY, 1,
            block_size, block_size, 1,
            0, NULL, argsTest, NULL);
        if (gmr != CUDA_SUCCESS) { qDebug() << QString("                                               VRtogetherWidget::setVerticesCUDAThreads Error cuLaunchKernel=%1  ID=%2").arg(gmr).arg(iBufferIDTh[uThID]); }
        cuCtxSynchronize();

        cuGraphicsUnmapResources(1, &positionCUDABufferTh[uThID], 0);
        cuGraphicsUnmapResources(1, &posColorCUDABufferTh[uThID], 0);
        //        theGLMessage.stopCUDA();

#ifdef DEBUGVR
        // copy result from device to host
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes0, debugmemCU0, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error10 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes1, debugmemCU1, sizeCU10);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error11 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCUbytes2, debugmemCU2, sizeCU10);
        if (gmr != CUDA_SUCCESS)
        {
            qDebug() << QString("setVerticesCUDA  Error12 cuMemcpyDtoH=%1").arg(gmr);
        }

        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes0, debugmemFloatCU0, sizeCU20);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error20 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes1, debugmemFloatCU1, sizeCU21);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error21 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((float *)debugmemFloatCUbytes2, debugmemFloatCU2, sizeCU22);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error22 cuMemcpyDtoH=%1").arg(gmr);
        }


        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes0, debugmemCharCU0, sizeCU30);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error30 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes1, debugmemCharCU1, sizeCU31);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error31 cuMemcpyDtoH=%1").arg(gmr);
        }
        gmr = cuMemcpyDtoH((int *)debugmemCharCUbytes2, debugmemCharCU2, sizeCU32);
        if (gmr != CUDA_SUCCESS) {
            qDebug() << QString("setVerticesCUDA  Error32 cuMemcpyDtoH=%1").arg(gmr);
        }
#endif
        cuCtxPopCurrent(NULL);

        //        for (int i = 1024; i < 1024+16; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 ").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i]);
        //        }
        //        qDebug("\n");

        //                for (int i = 0; i < 32; i++) {
        //                    qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 ").arg(i)
        //                                .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                                .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i]);
        //                }
        //                qDebug("\n");

        //        for (int i = 0; i < 32; i++) {
        //            qDebug() << QString("i=%1 val=%2 %3 %4 float=%5 %6 %7 char=%8 %9 %10").arg(i)
        //                        .arg(debugmemCUbytes0[i]).arg(debugmemCUbytes1[i]).arg(debugmemCUbytes2[i])
        //                        .arg(debugmemFloatCUbytes0[i]).arg(debugmemFloatCUbytes1[i]).arg(debugmemFloatCUbytes2[i])
        //                        .arg((uchar)debugmemCharCUbytes0[i]).arg((uchar)debugmemCharCUbytes1[i]).arg((uchar)debugmemCharCUbytes2[i]);
        //        }
        //        qDebug("\n");
    }
#endif
    if (drawed > 12)exit(0);
}

void SafeMain::allocVerticesCUDAThreads(uint uThID)
{
#ifdef USE_CUDA
    IndexType *indices = ViewerVertexBufferTh[uThID]->getIndices();

    int iIndex = 0;
    for (int i = 0; i < MAX_Y / iThreadsNumber; i++) {
        for (int j = 0; j < MAX_X; j++) {
            indices[iIndex] = IndexType(iIndex);
            iIndex++;
        }
    }
    num_elems = iIndex;
    m_iNumVertices = iIndex;
    m_iNumVerticesPerThread = m_iNumVertices;// / iThreadsNumber;
    qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads ............................... m_iNumVertices=%1  m_iNumVerticesPerThread=%2").arg(m_iNumVertices).arg(m_iNumVerticesPerThread);

    //	filePos = 0;
//    ViewerVertexBufferTh[uThID]->writeIndexBuffer(indices, num_elems, filePos, false);

//	for(int i=0; i < iThreadsNumber; i++)
//	{
        qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads        Allocating CUDA Buffers....... uThID=%1 ").arg(uThID);
        QGLBuffer *vertexBuffer = ViewerVertexBufferTh[uThID]->getVertexBuffer();
        QGLBuffer *colorBuffer = ViewerVertexBufferTh[uThID]->getColorBuffer();

        if (vertexBuffer->bind()) {
            if (vertexBuffer->map(QGLBuffer::WriteOnly)) {
                qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads    vertexBuffer allocated .......................................  size=%1  ID=%2").arg(vertexBuffer->size()).arg(vertexBuffer->bufferId());
                vertexBuffer->unmap();
                vertexBuffer->release();
            }
        }
        if (colorBuffer->bind()) {
            if (colorBuffer->map(QGLBuffer::WriteOnly)) {
                qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads    colorBuffer allocated .......................................  size=%1  ID=%2").arg(colorBuffer->size()).arg(colorBuffer->bufferId());
                colorBuffer->unmap();
                colorBuffer->release();
            }
        }

        qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads        Setting CUDA interoperability .....");
        qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads        vertexID=%1 colorID=%2   ").arg(vertexBuffer->bufferId()).arg(colorBuffer->bufferId());
        cuGraphicsGLRegisterBuffer(&positionCUDABufferTh[uThID], vertexBuffer->bufferId(), CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
        cuGraphicsGLRegisterBuffer(&posColorCUDABufferTh[uThID], colorBuffer->bufferId(), CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);
        qDebug() << QString("VRtogetherWidget::allocVerticesCUDAThreads        Finished CUDA interoperability setting .....");
//	}
#endif
    setAllocated(true);
}

void SafeMain::freeVerticesCUDAThreads(uint uThID)
{
    Q_UNUSED(uThID)
    qDebug("Deallocating Memory........................");

#ifdef USE_CUDA
    qDebug("Freeing CUDA  buffers ");
    cuGraphicsUnregisterResource(positionCUDABuffer);
    cuGraphicsUnregisterResource(posColorCUDABuffer);
	cuGraphicsUnregisterResource(indicesCUDABuffer);
#endif

    m_iNumVertices = 0;
    qDebug("Deallocated Memory........................");
}


//****************************************************************************

