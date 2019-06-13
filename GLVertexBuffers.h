#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include <malloc.h>

#include <QtCore/QtGlobal> // Q_OS_WIN definition
//#include <QtGlobal>
//#include <QtCore/qglobal.h>

#include <QMutex>
#include <QtCore/qvector.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include "qvector4duchar.h"
#include "qvector3duchar.h"
#include <QtDebug>

#include <QGLWidget>
#include <QGLBuffer>
//#include <QGLPixelBuffer>
#include <QtOpenGL/QGLFramebufferObject>
#include <QtOpenGL/QGLFramebufferObjectFormat>

#define BUFFERMEM 1024 * 1024

typedef uint IndexType;
const GLenum IndexTypeEnum = GL_UNSIGNED_INT;

class GLVertexBuffers
{
public:
//    static GLVertexBuffers& getInstance() { return m_tGLVertexBuffers; }
    GLVertexBuffers();
    ~GLVertexBuffers();

    void setParams(int iID);
    void setOnlyParams(int meshSiz, int m_iNumNur, int iU, int iV, QVector<int> vVal);
    void setOnlySizes(int meshSiz, int m_iNumNur, int iU, int iV);

    QVector3D *getPosns() { return posns; }                 // Vertices
    QVector4Du *getColors() { return colors; }
    IndexType *getIndices() { return indices; }
    void setNumIndices(int iVal) { numIndices = iVal; }
	char* getPosnsChar() { return (char*)posns; }
	float* getPosnsFloat() { return (float*)posns; }

    QGLBuffer* getVertexBuffer() { return vertexBuffer; }
    QGLBuffer* getColorBuffer() { return colorBuffer; }
    QGLBuffer* getIndexBuffer() { return indexBuffer; }


    void setPrint(bool bVal) { b_print = bVal; }
    void allocBuffers(int iNumVertices, int iNumIndices, int iMaxMem, int iNumBlocks);
    void freeBuffers();


    void allocVertexBuffer(int iNumVertices);
    void allocColorBuffer(int iNumVertices);
	void allocAllBuffer(int iNumVertices);

    void allocIndexBuffer(int iNumIndices);

    void setMemIndices(int iNumIndices);
    void setMemIndicesContourn(int iNumIndices);


    void freeVertexBuffer();
    void freeColorBuffer();
    void freeIndexBuffer();
	void freeAllBuffer();


    void bindVertexBuffer();
    void bindColorBuffer();
    void bindIndexBuffer(int iNumIndices, bool bText, bool bNor, bool bCol);
    void bindIndexSegmentsBuffer(int iNumIndices);
    void bindIndexBufferAll(int iNumIndices, char* mypcl, bool bText, bool bNor, bool bPrint);
	void drawBuffers();

 //   void writeVertexBuffer(QVector3D *posns, int iNumWrite, int iPosBlock, bool bPrint);
 //   void writeColorBuffer(QVector4Du *colors, int iNumWrite, int iPosBlock, bool bPrint);
 //   void writeIndexBuffer(IndexType *indices, int iNumWrite, int iPosBlock, bool bPrint);
	//void writeAllBuffer(uchar* pcl, int iNumWrite, bool bPrint);
	void writeVertexBuffer(QVector3D *posns, int iNumWrite, bool bPrint);
	void writeColorBuffer(QVector4Du *colors, int iNumWrite, bool bPrint);
	void writeIndexBuffer(IndexType *indices, int iNumWrite, bool bPrint);
	void writeAllBuffer(uchar* pcl, int iNumWrite, bool bPrint);

    void allocVertexBufferMem(int iNumIndices, int iNumBlocks);
    void freeVertexBufferMem();
    void clearVertexBufferMem(int iNumTriangles, int iNumBlocks);

    void allocColorBufferMem(int iNumIndices, int iNumBlocks);
    void freeColorBufferMem();
    void clearColorBufferMem(int iNumTriangles, int iNumBlocks);

    void allocIndexBufferMem(int iNumIndices, int iNumBlocks);
    void freeIndexBufferMem();
    void clearIndexBufferMem(int iNumIndices, int iNumBlocks);

	void allocAllBufferMem(int iNumIndices, int iNumBlocks);
	void freeAllBufferMem();
	void clearAllBufferMem(int iNumTriangles, int iNumBlocks);

    void printSizes();
    void printIndexSegmentsBuffer(int iNumWrite);
    void printVertexBuffer(int iNumWrite);


private:
    QGLBuffer *vertexBuffer;
    QGLBuffer *colorBuffer;
    QGLBuffer *indexBuffer;
	QGLBuffer *allBuffer;


    QVector3D *posns;
    QVector4Du *colors;
    IndexType *indices;
//------------------------------------------------------------
// If we decomment this, will produces compilation errors.
//------------------------------------------------------------
//    typedef uint IndexType;
//    static const GLenum IndexTypeEnum = GL_UNSIGNED_INT;

    int numIndices;
    QGLBuffer::UsagePattern vbodraw;
    int m_iID;

    bool gbCDataInitialized;
    bool gbCDataInitialized2;
    int ITERATIONS;

    float gaCAng1[72];
    float gaCAng2[72];

    bool b_print;
};

#endif // GLBUFFERS_H

