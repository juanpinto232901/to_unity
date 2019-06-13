#ifndef TO_UNITYT_H
#define TO_UNITYT_H

#include <QMutex>
#include <qdebug.h>
#include "signals_unity_bridge.h"
#include "cwipc_codec/include/api.h"

#include "to_unityt_global.h"
//extern "C" {
//	TO_UNITYTSHARED_EXPORT void setSUBThreadsNumber(int iVal);
//	TO_UNITYTSHARED_EXPORT int getSUBThreadsNumber();
//	TO_UNITYTSHARED_EXPORT void addUser(int uID, char* UrlName);
//	TO_UNITYTSHARED_EXPORT void removeUser(int uID);
//
//}

#ifdef __cplusplus
extern "C"{
#endif
//class TO_UNITYTSHARED_EXPORT To_Unityt
////class To_Unityt
//{
//
//public:
//    To_Unityt();
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall init();
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall addUser(int uID, char* UrlName, char* UserName);
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall setSUBThreadsNumber(int iVal);// { iSUBThreadsNumber = iVal; qDebug() << QString("666666666666666666666666666"); }
	extern "C" TO_UNITYTSHARED_EXPORT int __stdcall getSUBThreadsNumber();// { fprintf(stdout, "111111111111111111111111"); return iSUBThreadsNumber; }
	extern "C" TO_UNITYTSHARED_EXPORT void stopSUBThreads();
	extern "C" TO_UNITYTSHARED_EXPORT void unStopSUBThreads();
	extern "C" TO_UNITYTSHARED_EXPORT void setSUBThreads();
	extern "C" TO_UNITYTSHARED_EXPORT void destroySUBThreads();
	extern "C" TO_UNITYTSHARED_EXPORT void startSUBThreads();
	extern "C" TO_UNITYTSHARED_EXPORT void setUseSUBThreads(bool value);
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall printUsers();
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall removeUserbyID(int uID);
	extern "C" TO_UNITYTSHARED_EXPORT void __stdcall removeUserbyName(char* UserName);


	FrameInfo* timestampInfo[NUMPCL];
	cwipc_point* mypcl[NUMPCL];
//	UserUrl* myUserUrl;

//private:
	int iSUBThreadsNumber;
	int iNumProcessorsQTH;
	int iNUMPCL;
	int xRot;
	int yRot;
	int zRot;
	int yElev;
	float m_fDistEye;
	bool initialized;                           /// The devices was initialized
	bool allocated;                             /// The memory was allocated
	bool plyloaded;                             /// The file PLY was loaded
	QMutex doStopMutex;

	//int uID;
	//char* UrlName;

//};

#ifdef __cplusplus
}
#endif
#endif // TO_UNITYT_H
