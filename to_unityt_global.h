#ifndef TO_UNITYT_GLOBAL_H
#define TO_UNITYT_GLOBAL_H

//#include <QtCore/qglobal.h>

#if defined(TO_UNITYT_LIBRARY)
#  define TO_UNITYTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TO_UNITYTSHARED_EXPORT Q_DECL_IMPORT
#endif

//extern "C" { 
//	TO_UNITYTSHARED_EXPORT void setSUBThreadsNumber(int iVal); 
//	TO_UNITYTSHARED_EXPORT int getSUBThreadsNumber(); 
//}

//extern "C" {
//	TO_UNITYTSHARED_EXPORT void setSUBThreadsNumber(int iVal);
//	TO_UNITYTSHARED_EXPORT int getSUBThreadsNumber();
//	TO_UNITYTSHARED_EXPORT void addUser(int uID, char* UrlName);
//}

#define NUMPCL 64

struct UserUrl
{
	int uID;
	char url[256];
};

#endif // TO_UNITYT_GLOBAL_H
