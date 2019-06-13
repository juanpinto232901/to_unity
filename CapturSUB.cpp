
#include "signals_unity_bridge.h"
#include "cwipc_codec/include/api.h"

#include "CapturSUB.h"
#include <QtDebug>

#define STAR_PCL_DEBUG 0 // 500

#define IMPORT(name) ((decltype(name)*)SDL_LoadFunction(lib, # name))


CapturSUB::CapturSUB()
    : QThread()
    , uIDa(0)
    , doStop(false)
	, iNumPoints(0)
	, mypcl(0)
	, iNumReads(1)
{
    qDebug("Constructor CapturSUB ");
	size_t  iSizeB = 1048576 * sizeof(cwipc_point) * 4;
	mypcl = (cwipc_point*)malloc(iSizeB);
	memset(mypcl, 0, iSizeB);
	qDebug() << QString("*********   Constructor  uID=%2 mypcl=%3 size=%4  cwipc_point=%5").arg(uIDa).arg((qlonglong)mypcl).arg(_msize(mypcl)).arg(sizeof(cwipc_point));

	dst = (uint8_t*)malloc(iSizeB);
	memset(dst, 0, iSizeB);

	timestampInfo = (FrameInfo*)malloc(1048576 * 4 * sizeof(FrameInfo));
	memset(timestampInfo, 0, 1048576 * 2 * sizeof(FrameInfo));

	iNumReads = 1;
}

void CapturSUB::read_ini(uint uID)
{
	//	QMutexLocker l1(&m_tVectorAccessMutex);
	//	doStopM01.lock();
	const std::string mediaUrl2 = "http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd";
	char buff1[1024];
	char buff2[64];
	char buff3[64];
	//uID = 0;

	QString avatarName = QString("loot");
	QString mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd");
	if (uID == 2) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd"); avatarName = QString("loot"); }
	if (uID == 1) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/longdress/vrtogether.mpd"); avatarName = QString("longdress"); }
	if (uID == 0) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/redandblack/vrtogether.mpd"); avatarName = QString("redandblack"); }
	//if (uID == 0) { mediaUrl = QString("http://livesim.dashif.org/livesim/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 1"); }
	//if (uID == 1) { mediaUrl = QString("http://livesim.dashif.org/livesim/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 2"); }
	//if (uID == 2) { mediaUrl = QString("http://vm2.dashif.org/livesim-dev/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 3"); }
	memset(buff1, 0, 1024);
	memset(buff2, 0, 64);
	memset(buff3, 0, 64);

	iNumReads = 0;

#if defined( Q_OS_WIN )
    SetDllDirectoryA("D:/i2cat_vrtogether/v31_stable_qt5/getSUB/w64");
#endif
	uint usize = GetDllDirectoryA(1024, buff1);
	qDebug() << QString("DLLdir=%1   th=%2").arg(buff1).arg(uID);

	QFile myFile("signals-unity-bridge.dll");
	QDir myDir;
	QString aasd = myDir.absoluteFilePath("signals-unity-bridge.dll");
	qDebug() << QString("path=%1  th=%2 ").arg(aasd).arg(uID);
	QLibrary library(aasd);
	library.setFileName(aasd);

	qDebug() << QString(".... %1 ....  th=%2 ").arg(library.fileName()).arg(uID);

	if (!library.load()) { qDebug() << library.errorString(); doStopM01.unlock(); exit(0); }
	if (library.load())qDebug() << QString("signals-unity-bridge library loaded.............  th=%1 ").arg(uID);


	func_sub_create = (sub_create)library.resolve("sub_create");

	func_sub_destroy = (sub_destroy)library.resolve("sub_destroy");

	func_sub_get_stream_count = (sub_get_stream_count)library.resolve("sub_get_stream_count");

	func_sub_play = (sub_play)library.resolve("sub_play");

	func_sub_grab_frame = (sub_grab_frame)library.resolve("sub_grab_frame");

	info = (FrameInfo*)malloc(sizeof(FrameInfo));
	int streamIndex = 0;

	//		doStopM01.lock();//----------------------------- lock --------------------------
	itoa(uID, buff3, 10);
	strcpy_s(buff2, 64, "MyMediaPipeline");
	strcat_s(buff2, buff3);
	handle = func_sub_create(buff2);
	//	auto handle = func_sub_create("MyMediaPipeline");

	strcpy_s(buff1, 1024, mediaUrl.toLocal8Bit());
	qDebug() << QString("buff1=%1  th=%2 buff2=%3 ").arg(buff1).arg(uID).arg(buff2);
	bool bRes = func_sub_play(handle, buff1);
	//		doStopM01.unlock();//--------------------------- unlock ------------------------------
}

void CapturSUB::read(uint uID)
{
	int n1 = 0;
	int iNum = 0;
	size_t iSize = 0;
	size_t iSizeB = 0;
	uint iNumPLY = 0;
	int streamIndex = 0;

		//		doStopM01.lock();
	myTimer.start();
	Sleep(DELAY);

	iNumReads++;

	iNum = 0;
	iSize = 1048576 * sizeof(cwipc_point) * 4;

	int iStreamCount = 0;
	iStreamCount = func_sub_get_stream_count(handle);

	if (iStreamCount > 0) {
		streamIndex = 0;
//		iSize = (size_t)func_sub_grab_frame(handle, streamIndex, 0, 0, info);
		//		//QString msg01 = QString("First  iSize_compresed= %1  th=%2").arg( (int)iSize ).arg(uID);
		//		//qDebug() << msg01;
				//if (uID == 0)theGLMessageTh->setSizePCL1comp((int)iSize);
				//if (uID == 1)theGLMessageTh->setSizePCL2comp((int)iSize);
				//if (uID == 2)theGLMessageTh->setSizePCL3comp((int)iSize);
		//if (iSize > 0)
		//{
			//if (uID == 0)theGLMessageTh->setAvatar1Name(avatarName);
			//if (uID == 1)theGLMessageTh->setAvatar2Name(avatarName);
			//if (uID == 2)theGLMessageTh->setAvatar3Name(avatarName);

		qDebug() << QString("handle=%1 streamIndex=%2 dst=%3 iSize=%4 info=%5  uID=%6").arg((qlonglong)handle).arg(streamIndex).arg(_msize(dst)).arg(iSize).arg(info->timestamp).arg(uID);
			iSizeB = (size_t)func_sub_grab_frame(handle, streamIndex, dst, iSize, info);
				qDebug() << QString("CapturSUBThread::run   iSizeB_compresed_readed=%1   uID=%2  ").arg(iSizeB).arg(uID);
				//if (uID == 0)theGLMessageTh->setReadedPCL1comp((int)iSizeB);
				//if (uID == 1)theGLMessageTh->setReadedPCL2comp((int)iSizeB);
				//if (uID == 2)theGLMessageTh->setReadedPCL3comp((int)iSizeB);


			if (iSizeB > 0) {
				//for (int i = 0; i < 20; i++) {
				//	qDebug() << QString("noThreads   i=%1 buffer_sub=%2  ")
				//		.arg(i)
				//		.arg(dst[i]);
				//}

				//
				// Uncompress
				//
				cwipc_decoder *decoder = cwipc_new_decoder();
				if (decoder == NULL) {
					qDebug() << "CapturSUBThread" << ": Could not create decoder \n";
					doStopMutex.lock();
					doStop = true;
					doStopMutex.unlock();
					//				return 1;
				}
				decoder->feed((void*)dst, iSizeB);
				//			free((uint8_t *)dst); // After feed() we can free the input buffer

				bool ok = decoder->available(true);
				if (!ok) {
					qDebug() << "CapturSUBthread" << ": Decoder did not create pointcloud \n";
					doStopMutex.lock();
					doStop = true;
					doStopMutex.unlock();
					//				return 1;
				}
				cwipc *pc = decoder->get();
				if (pc == NULL) {
					qDebug() << "CapturSUBThread" << ": Decoder did not return cwipc \n";
					doStopMutex.lock();
					doStop = true;
					doStopMutex.unlock();
					//				return 1;
				}

				decoder->free(); // We don't need the decoder anymore
				if (pc) {
					int isizepcl = pc->get_uncompressed_size(CWIPC_POINT_VERSION);
					iNum = pc->copy_uncompressed(mypcl, isizepcl);
					timestampInfo->timestamp = info->timestamp;
					//		qDebug() << QString("CapturSUBThread::run  _________ isizepcl=%1 nelems=%2 iNum_readed=%3  uID=%4 mypcl=%5").arg(isizepcl).arg(isizepcl / sizeof(cwipc_point)).arg(iNum).arg(uID).arg((qlonglong)mypcl);
							//if (uID == 0)theGLMessageTh->setUncomp1(iNum);
							//if (uID == 1)theGLMessageTh->setUncomp2(iNum);
							//if (uID == 2)theGLMessageTh->setUncomp3(iNum);

					iNumPoints = iNum;

					//for (int i = 0; i < 100; i++) {
					//	qDebug() << QString("decodec i=%1 pos=%2 %3 %4 color=%5 %6 %7   isizepcl=%8 ")
					//		.arg(i)
					//		.arg(mypcl[i].x).arg(mypcl[i].y).arg(mypcl[i].z)
					//		.arg(mypcl[i].r).arg(mypcl[i].g).arg(mypcl[i].b)
					//		.arg(isizepcl);
					//}

					//
					// Save pointcloud file
					//
	//char filename[128];
	//sprintf_s(filename, "D:/i2cat_vrtogether/v30_stable_qt5/getSUB/w64/ply/myply_%d.ply", iNumPLY);
	//qDebug() << QString("file=%1 ").arg(filename);
	//if (cwipc_write(filename, pc, NULL) < 0) {
	//	qDebug() << filename << ": Error writing PLY file " << filename ;
	//	exit(0) ;
	//}
	//iNumPLY++;

					pc->free(); // We no longer need to pointcloud

				}
			}
//		}
			if (!iSizeB && iNumReads > 100) {
				const std::string mediaUrl2 = "http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd";
				char buff1[1024];
				char buff2[64];
				char buff3[64];
				//uID = 0;
				Sleep(100);

				QString avatarName = QString("loot");
				QString mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd");
				if (uID == 2) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/loot/vrtogether.mpd"); avatarName = QString("loot"); }
				if (uID == 1) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/longdress/vrtogether.mpd"); avatarName = QString("longdress"); }
				if (uID == 0) { mediaUrl = QString("http://vrt-pcl2dash.viaccess-orca.com/redandblack/vrtogether.mpd"); avatarName = QString("redandblack"); }
				//if (uID == 0) { mediaUrl = QString("http://livesim.dashif.org/livesim/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 1"); }
				//if (uID == 1) { mediaUrl = QString("http://livesim.dashif.org/livesim/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 2"); }
				//if (uID == 2) { mediaUrl = QString("http://vm2.dashif.org/livesim-dev/testpic_2s/Manifest.mpd"); avatarName = QString("testpic 2s 3"); }
				memset(buff1, 0, 1024);
				memset(buff2, 0, 64);
				memset(buff3, 0, 64);


				func_sub_destroy(handle);

				itoa(uID, buff3, 10);
				strcpy_s(buff2, 64, "MyMediaPipeline");
				strcat_s(buff2, buff3);
				handle = func_sub_create(buff2);
				//	auto handle = func_sub_create("MyMediaPipeline");

				strcpy_s(buff1, 1024, mediaUrl.toLocal8Bit());
				qDebug() << QString("....... buff1=%1  th=%2 buff2=%3  iNumReads=%4 ").arg(buff1).arg(uID).arg(buff2).arg(iNumReads);
				bool bRes = func_sub_play(handle, buff1);

				iSizeB = (size_t)func_sub_grab_frame(handle, streamIndex, dst, iSize, info);
				qDebug() << QString("CapturSUBThread::run   iSizeB_compresed_reading=%1     ").arg(iSizeB);

				iNumReads = 0;
			}
	}
	//		iNumPoints = iNum;
	myTimer.stop();
	double dTime = myTimer.getElapsedTimeInMilliSec();

	//		doStopM01.unlock();
}

void CapturSUB::read_end(uint uID)
{
	free(dst);
	dst = 0;
	free(mypcl);
	mypcl = 0;
	func_sub_destroy(handle);
    qDebug() << QString("Thread  OUT ......  uID=%1 ").arg(uID);
}

void CapturSUB::processGeom(GLVertexBuffers *ViewerVertexBuffer1, GLMessages* theGLMessage, bool bSeeColor, float despX, float despY, float despZ)
{
	QVector3D* vVertex = ViewerVertexBuffer1->getPosns();
	QVector4Du* vColor = ViewerVertexBuffer1->getColors();
	IndexType* vIndices = ViewerVertexBuffer1->getIndices();
	bool bOnlyOne = false;
	bool bDraw = false;
	int iTest = 0;
	int iNumAvat = 6;
	FrameInfo* timestampInfo;

	int iposmat = 0; int i = 0;
	//mypcl[i] = mySUB01->getPCL();
	//timestampInfo[i] = mySUB01->getInfo();
	//iNumPoints[i] = mySUB01->getNumPoints();
	//num_elems = iNumPoints[i];
	if (iNumPoints > 0) {
		theGLMessage->startOpenCL();

		for (int i1 = 0; i1 < iNumPoints; i1++) {
			float vx = mypcl[i1].x;
			float vy = mypcl[i1].y;
			float vz = mypcl[i1].z;
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

			uint8_t cr = (mypcl[i1].r) * 0.5;
			uint8_t cg = (mypcl[i1].g) * 0.5;
			uint8_t cb = (mypcl[i1].b) * 0.5;
			uint8_t ca = 255;
			vColor[i1].setX(cr);
			vColor[i1].setY(cg);
			vColor[i1].setZ(cb);
			//				if (i1 < 20) { qDebug() << QString("i1=%0 color=%1 %2 %3 ").arg(i1).arg(vColor[i1].x()).arg(vColor[i1].y()).arg(vColor[i1].z()); }
		}
		theGLMessage->stopOpenCL();
		//			qDebug() << QString("time=%1 us").arg(time01.getElapsedTimeInMicroSec());

		glPointSize(3.0);

		ViewerVertexBuffer1->writeVertexBuffer(vVertex, iNumPoints, false);

		ViewerVertexBuffer1->writeColorBuffer(vColor, iNumPoints, false);

		ViewerVertexBuffer1->writeIndexBuffer(vIndices, iNumPoints, false);

		ViewerVertexBuffer1->bindVertexBuffer();
		ViewerVertexBuffer1->bindColorBuffer();
		if (bSeeColor)ViewerVertexBuffer1->bindIndexBuffer(iNumPoints, false, false, true);
		else ViewerVertexBuffer1->bindIndexBuffer(iNumPoints, false, false, false);

	}

}
