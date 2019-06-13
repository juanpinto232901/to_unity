#include <QtGlobal>
#include "GLVertexBuffers.h"

#define STAR_PCL_DEBUG 0 // 500

//#undef Q_WS_WIN
//GLVertexBuffers GLVertexBuffers::m_tGLVertexBuffers;

GLVertexBuffers::GLVertexBuffers()
    : vertexBuffer(0)
    , colorBuffer(0)

    , indexBuffer(0)
	, allBuffer(0)
    , indices(0)

    , numIndices(0)
    , m_iID(0)

    , posns(0)
    , colors(0)

    , b_print(false)
{
//StreamDraw          = 0x88E0, // GL_STREAM_DRAW
//StreamRead          = 0x88E1, // GL_STREAM_READ
//StreamCopy          = 0x88E2, // GL_STREAM_COPY
//StaticDraw          = 0x88E4, // GL_STATIC_DRAW
//StaticRead          = 0x88E5, // GL_STATIC_READ
//StaticCopy          = 0x88E6, // GL_STATIC_COPY
//DynamicDraw         = 0x88E8, // GL_DYNAMIC_DRAW
//DynamicRead         = 0x88E9, // GL_DYNAMIC_READ
//DynamicCopy         = 0x88EA  // GL_DYNAMIC_COPY

    vbodraw = QGLBuffer::StaticDraw;
    //vbodraw = QGLBuffer::DynamicDraw;
    qDebug(" ======== GLVertexBuffers Constructor ========= ");
    if(vbodraw == QGLBuffer::StreamDraw) qDebug() << QString("StreamDraw          = 0x88E0, // GL_STREAM_DRAW ");
    if(vbodraw == QGLBuffer::StreamRead) qDebug() << QString("StreamRead          = 0x88E1, // GL_STREAM_READ ");
    if(vbodraw == QGLBuffer::StreamCopy) qDebug() << QString("StreamCopy          = 0x88E2, // GL_STREAM_COPY ");
    if(vbodraw == QGLBuffer::StaticDraw) qDebug() << QString("StaticDraw          = 0x88E4, // GL_STATIC_DRAW");
    if(vbodraw == QGLBuffer::StaticRead) qDebug() << QString("StaticRead          = 0x88E5, // GL_STATIC_READ ");
    if(vbodraw == QGLBuffer::StaticCopy) qDebug() << QString("StaticCopy          = 0x88E6, // GL_STATIC_COPY ");
    if(vbodraw == QGLBuffer::DynamicDraw)qDebug() << QString("DynamicDraw         = 0x88E8, // GL_DYNAMIC_DRAW ");
    if(vbodraw == QGLBuffer::DynamicRead)qDebug() << QString("DynamicRead         = 0x88E9, // GL_DYNAMIC_READ ");
    if(vbodraw == QGLBuffer::DynamicCopy)qDebug() << QString("DynamicCopy         = 0x88EA  // GL_DYNAMIC_COPY ");
}

GLVertexBuffers::~GLVertexBuffers()
{
    freeIndexBuffer();

    delete vertexBuffer;
    vertexBuffer = 0;
    delete colorBuffer;
    colorBuffer = 0;
    delete indexBuffer;
    indexBuffer = 0;
	delete allBuffer;
	allBuffer = 0;
}

void GLVertexBuffers::setParams(int iID)
{
    m_iID = iID;

qDebug() << QString("GLVertexBuffers::setParams _______ in _________-- ID=%1 ").arg(m_iID);
    vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    vertexBuffer->setUsagePattern(vbodraw);
    if ( !vertexBuffer->create() )
    {
        qDebug() << QString("GLVertexBuffers::setParams     Error to create vertexBuffer");
        if(vertexBuffer)delete vertexBuffer;
        vertexBuffer = 0;
        exit(0);
    }

    colorBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    colorBuffer->setUsagePattern(vbodraw);
    if(!colorBuffer->create()){
        qDebug() << QString("GLVertexBuffers::setParams     Error to create colorBuffer");
        if(colorBuffer)delete colorBuffer;
        colorBuffer = 0;
        exit(0);
    }

    // Upload the indices into a server-side buffer.
    indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
    indexBuffer->setUsagePattern(vbodraw);
    if (!indexBuffer->create()) {
        qDebug() << QString("GLVertexBuffers::setParams     Error to create indexBuffer");
        delete indexBuffer;
        indexBuffer = 0;
        exit(0);
    }

	allBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
	allBuffer->setUsagePattern(vbodraw);
	if (!allBuffer->create())
	{
		qDebug() << QString("GLVertexBuffers::setParams     Error to create allBuffer");
		if (allBuffer)delete allBuffer;
		allBuffer = 0;
		exit(0);
	}

    qDebug() << QString("GLVertexBuffers::setParams _______ out _________-- ");
}

void GLVertexBuffers::allocAllBuffer(int iNumVertices)
{
	if (iNumVertices < 1)return;
	Q_ASSERT(allBuffer);
	if (allBuffer) {
		qDebug() << QString("GLVertexBuffers::allocAllBuffer     iNumVertices=%1  ").arg(iNumVertices);
		if (allBuffer->bind()) {
			allBuffer->allocate(16 * iNumVertices);
			if (!allBuffer->map(QGLBuffer::WriteOnly)) {
				// No point using a vertex buffer if we cannot map it.
				allBuffer->unmap();
				allBuffer->release();
				delete allBuffer;
				allBuffer = 0;
				exit(0);
			}
			else {
				qDebug() << QString("allBuffer allocated .......................................  size=%1  ID=%2").arg(allBuffer->size()).arg(allBuffer->bufferId());
				allBuffer->unmap();
				allBuffer->release();
			}
		}
		else {
			qDebug() << QString("Error:  We not can alloc allBuffer ............ ");
			delete allBuffer;
			allBuffer = 0;
			exit(0);
		}
	}

}

void GLVertexBuffers::allocVertexBuffer(int iNumVertices)
{
    if(iNumVertices < 1)return;
    Q_ASSERT(vertexBuffer);
    if (vertexBuffer) {
        qDebug() << QString("GLVertexBuffers::allocVertexBuffer     iNumVertices=%1  ").arg(iNumVertices);
        if(vertexBuffer->bind()){
            vertexBuffer->allocate(sizeof(GLfloat) * 3 * iNumVertices);
//            vertexBuffer->allocate(vertices, sizeof(GLfloat) * 3 * iNumVertices);
            if (!vertexBuffer->map(QGLBuffer::WriteOnly)) {
                // No point using a vertex buffer if we cannot map it.
                vertexBuffer->unmap();
                vertexBuffer->release();
                delete vertexBuffer;
                vertexBuffer = 0;
                exit(0);
            } else {
                qDebug() << QString("vertexBuffer allocated .......................................  size=%1  ID=%2").arg(vertexBuffer->size()).arg(vertexBuffer->bufferId());
                vertexBuffer->unmap();
                vertexBuffer->release();
            }
        } else {
            qDebug() << QString("Error:  We not can alloc vertexBuffer ............ ");
            delete vertexBuffer;
            vertexBuffer = 0;
            exit(0);
        }
    }

}

void GLVertexBuffers::allocColorBuffer(int iNumVertices)
{
    if(iNumVertices < 1)return;
    Q_ASSERT(colorBuffer);
    if (colorBuffer) {
        qDebug() << QString("GLVertexBuffers::allocColorBuffer     iNumVertices=%1  ").arg(iNumVertices);
        if(colorBuffer->bind()){
            colorBuffer->allocate(sizeof(uchar) * 4 * iNumVertices);
            if (!colorBuffer->map(QGLBuffer::WriteOnly)) {
                // No point using a vertex buffer if we cannot map it.
                colorBuffer->unmap();
                colorBuffer->release();
                delete colorBuffer;
                colorBuffer = 0;
                exit(0);
            } else {
                qDebug() << QString("colorBuffer allocated .......................................  size=%1  ID=%2 ").arg(colorBuffer->size()).arg(colorBuffer->bufferId());
                colorBuffer->unmap();
                colorBuffer->release();
            }
        } else {
            qDebug() << QString("Error:  We not can alloc colorBuffer ............ ");
            delete colorBuffer;
            colorBuffer = 0;
            exit(0);
        }
    }
}

//-----------------------------------------------------------------------------

void GLVertexBuffers::allocIndexBuffer(int iNumIndices)
{
    if(iNumIndices < 1)return;
    // Upload the indices into a server-side buffer.
//    indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
//    indexBuffer->setUsagePattern(vbodraw);
//    if (!indexBuffer->create()) {
//        delete indexBuffer;
//        indexBuffer = 0;
//        qDebug() << QString("Fail to create indexBuffer");
//        exit(0);
//    }
    Q_ASSERT(indexBuffer);

    if (indexBuffer) {
        qDebug() << QString("GLVertexBuffers::allocIndexBuffer     iNumIndices=%1  ").arg(iNumIndices);
        if(indexBuffer->bind()){
            indexBuffer->allocate(sizeof(IndexType) * iNumIndices);
            if (!indexBuffer->map(QGLBuffer::WriteOnly)) {
                // No point using a vertex buffer if we cannot map it.
                indexBuffer->unmap();
                indexBuffer->release();
                delete indexBuffer;
                indexBuffer = 0;
                exit(0);
            } else {
                qDebug() << QString("indexBuffer allocated .......................................  size=%1  ID=%2 ").arg(indexBuffer->size()).arg(indexBuffer->bufferId());
                indexBuffer->unmap();
                indexBuffer->release();
            }
        } else {
            qDebug() << QString("Error:  We not can alloc indexBuffer ............ ");
            delete indexBuffer;
            indexBuffer = 0;
            exit(0);
        }
    }

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** Set the indices memory to NUM_TRIANGLES_MAX
 * @brief GLVertexBuffers::setMemIndices
 * @param iNumIndices Triangles number
 */
void GLVertexBuffers::setMemIndices(int iNumIndices)
{
    Q_UNUSED(iNumIndices);
    /*
    if(iNumIndices < 1)return;
    int maxIndices = iNumIndices * 3;

    int tmem = 0;
#ifdef Q_WS_WIN
    if(indices != 0)tmem= _msize(indices);
#else
    if(indices != 0)tmem = (int)malloc_usable_size(indices);
#endif
    if(tmem <= 0)indices= 0;
    else if(indices != 0){
        free(indices);
        indices = 0;}
    indices = (IndexType *)malloc(maxIndices * sizeof(IndexType));
    memset(indices, 0, maxIndices * sizeof(IndexType));

#ifdef Q_WS_WIN
    tmem= _msize(indices);
#else
    tmem = (int)malloc_usable_size(indices);
#endif
    qDebug() << QString("GLVertexBuffers::setMemIndices     indices=%1 ").arg(tmem);
//*/
}

void GLVertexBuffers::setMemIndicesContourn(int iNumIndices)
{
    Q_UNUSED(iNumIndices);
/*
    if(iNumIndices < 1)return;
    int maxIndices = iNumIndices * 2;

    int tmem = 0;
#ifdef Q_WS_WIN
    if(indicesContourn != 0)tmem= _msize(indicesContourn);
#else
    if(indicesHideLines != 0)tmem = malloc_usable_size(indicesHideLines);
#endif
    if(tmem <= 0)indicesHideLines= 0;
    else if(indicesHideLines != 0){
        free(indicesHideLines);
        indicesHideLines = 0;}

    indicesHideLines = (IndexType *)malloc(maxIndices * sizeof(IndexType));
    memset(indicesHideLines, 0, maxIndices * sizeof(IndexType));

#ifdef Q_WS_WIN
    if(indicesContourn != 0)tmem= _msize(indicesContourn);
#else
    if(indicesHideLines != 0)tmem = malloc_usable_size(indicesHideLines);
#endif
    qDebug() << QString("GLVertexBuffers::setMemIndicesContourn     indicesContourn=%1 ").arg(tmem);
//*/
}

//-----------------------------------------------------------------------------

void GLVertexBuffers::freeVertexBuffer()
{
    if(vertexBuffer){
        vertexBuffer->destroy();
        vertexBuffer = 0;
    }

}

void GLVertexBuffers::freeColorBuffer()
{
    if(colorBuffer){
        colorBuffer->destroy();
        colorBuffer = 0;
    }
}

void GLVertexBuffers::freeAllBuffer()
{
	if (allBuffer) {
		allBuffer->destroy();
		allBuffer = 0;
	}

}

void GLVertexBuffers::freeIndexBuffer()
{
    if(indexBuffer){
        indexBuffer->destroy();
        indexBuffer = 0;
    }
}

//-----------------------------------------------------------------------------
/*
void GLVertexBuffers::fillIndices()
{
    if(m_iNumNurbs < 1)return;
    if(malloc_usable_size(indices) < 1)return;

    numIndices = 0;
    int subdivQuad= meshSize * meshSize;

//    int num = subdivQuad * m_iNumNurbs;
//    int maxIndices = num * 4;
//    qDebug() << QString("indices maxIndices=%1").arg(maxIndices);
//    if(indices){free(indices); indices= 0;}
//    indices = (IndexType *)malloc(maxIndices * sizeof(IndexType));

    for(int nNurbs=0; nNurbs<m_iNumNurbs; nNurbs++)
    {
        int iPosNurbs = nNurbs * subdivQuad;
        for (int sint = 0; sint < (meshSize - 1); ++sint) {
            for (int tint = 0; tint < (meshSize - 1); ++tint) {
                int corner1 = (sint + tint * meshSize) + iPosNurbs;
                int corner2 = (sint + (tint + 1) * meshSize) + iPosNurbs;
                int corner3 = ((sint + 1) + (tint + 1) * meshSize) + iPosNurbs;
                int corner4 = ((sint + 1) + tint * meshSize) + iPosNurbs;
                indices[numIndices++] = corner1;
                indices[numIndices++] = corner2;
                indices[numIndices++] = corner3;
                indices[numIndices++] = corner4;
//                KSHOW_TRACE(QString("GLVertexBuffers::fillIndices      nNurbs=%1  sint=%2 tint=%3   corners=%4 %5 %6 %7  numIndices=%8").arg(nNurbs).arg(sint).arg(tint).arg(corner1).arg(corner2).arg(corner3).arg(corner4).arg(numIndices));
            }
        }
    }
//    KSHOW_TRACE(QString("GLVertexBuffers::fillIndices      numIndices=%8").arg(numIndices));
}

void GLVertexBuffers::fillIndexContournBuffer(int iMode)
{
    if(iMode == 1 && m_iKnotSel.size() < 1)return;
    if(m_iNumNurbs < 1)return;
    if(malloc_usable_size( indicesContourn ) < 1)return;

    numIndicesContourn = 0;
    int indContourn = 0;
    int nlin = meshSize * 4 * m_iNumNurbs;
    int maxIndices = nlin * 2;
//    if(indicesContourn){free(indicesContourn); indicesContourn= 0;}
//    indicesContourn = (IndexType *)malloc(maxIndices * sizeof(IndexType));
//    memset(indicesContourn, 0, maxIndices * sizeof(IndexType));

    int iNurbsHor = 0;
    int iNurbsVer = 0;
    int iTotind = 0;
    int iTotalElements = m_iKnotSel.size();
    if(iMode == 0)iTotalElements = m_iNumNurbs;

    for(int nNurbs=0; nNurbs < m_iNumNurbs; nNurbs++)
    {
        int iNumNurbs = -1;
        int iBaseContourn = iNumNurbs * meshSize * 4;
        if(iMode == 1){
            for(int j = 0; j < iTotalElements; j++)
            {
                iNumNurbs = m_iKnotSel.at(j);
                if(iNumNurbs == nNurbs){
                    iBaseContourn = iNumNurbs * meshSize * 4;
                    indContourn = iBaseContourn;
                    j = iTotalElements;
                }
            }
        }
        if(indContourn >= 0){
            int iIndicesByNurbs = 0;
            for(int edge= 0; edge < 4; edge++){
                for(int ind= 0; ind < meshSize; ind++)
                {
                    if(ind < meshSize - 1){
                        if(numIndicesContourn < maxIndices)
                        {
                            indicesContourn[numIndicesContourn] = indContourn;
                            if(iMode == 1 && iBaseContourn < 0)indicesContourn[numIndicesContourn] = 0;
                            numIndicesContourn++;
                        }

                        if(numIndicesContourn < maxIndices)
                        {
                            indicesContourn[numIndicesContourn] = indContourn+1;
                            if(iMode == 1 && iBaseContourn < 0)indicesContourn[numIndicesContourn] = 0;
                            numIndicesContourn++;
                        }
                    }
                    indContourn++;
                    iIndicesByNurbs++;
                }
                iTotind++;
            }
        }

        iNurbsHor++;
        if(iNurbsHor >= m_iUsize){
            iNurbsVer++;
            iNurbsHor = 0;
        }
    }
//    KSHOW_TRACE(QString("................................ indContourn=%1 numIndicesContourn=%2 ").arg(indContourn).arg(numIndicesContourn));
}
//*/
//-----------------------------------------------------------------------------

void GLVertexBuffers::bindVertexBuffer()
{
    if (vertexBuffer) {
        vertexBuffer->bind();
        glVertexPointer(3, GL_FLOAT, 0, 0);
                    QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
                    for(int q=0; q<32; q++)//iNumIndices; q++)
                    {
                      //  if((q%3) == 0)qDebug(" \n");
                    //    if((q % 4) == 0)qDebug() << " \n";
                        qDebug() << QString("GLVertexBuffers::bindVertexBuffer    q=%0 vertex=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
                    }
                    vertexBuffer->unmap();
        vertexBuffer->release();
    }
}

void GLVertexBuffers::bindColorBuffer()
{
    if (colorBuffer) {
        colorBuffer->bind();
        glColorPointer(4, GL_BYTE, 0, 0);
//		glColorPointer(3, GL_BYTE, 0, 0);

                    QVector3Du *mappedV = (QVector3Du *)colorBuffer->map(QGLBuffer::WriteOnly);
                    for(int q=0; q<32; q++)
                    {
                    //    if((q % 4) == 0)qDebug() << " \n";
                        qDebug() << QString("GLVertexBuffers::bindColorBuffer    q=%0 color=%1 %2 %3").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
                    }
                    colorBuffer->unmap();
        colorBuffer->release();
    }
}

void GLVertexBuffers::bindIndexBuffer(int iNumIndices, bool bText, bool bNor, bool bCol)
{
    Q_ASSERT(indexBuffer);
 //   qDebug() << QString("GLVertexBuffers::bindIndexBuffer  iNumIndices=%1 indexBuffer=%2 ").arg(iNumIndices).arg((qlonglong)indexBuffer);

    if(indexBuffer){
        glEnableClientState(GL_VERTEX_ARRAY);
        if(bText)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(bNor)
            glEnableClientState(GL_NORMAL_ARRAY);

        if(bCol)glEnableClientState(GL_COLOR_ARRAY);
        indexBuffer->bind();
                                IndexType *mappedV1 = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
                                vertexBuffer->bind();
                                QVector3D *mappedV2 = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
                                for(int q=0; q<32; q++)//iNumIndices; q++)
                                {
                                    uint ind1 = mappedV1[q];
                      //              if((q%3) == 0)qDebug(" \n");
                                    qDebug() << QString("GLVertexBuffers::bindIndexBuffer  vertex  q=%0 ind1=%4 vertex=%1 %2 %3 ")
                                                .arg(q)
                                                .arg(mappedV2[ind1].x()).arg(mappedV2[ind1].y()).arg(mappedV2[ind1].z())
                                                .arg(ind1);

                                }
                                vertexBuffer->unmap();
                                vertexBuffer->release();
                                indexBuffer->unmap();

/*
                                mappedV1 = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
                                colorBuffer->bind();
                                QVector4Du *mappedV3 = (QVector4Du *)colorBuffer->map(QGLBuffer::WriteOnly);
                                for(int q=0; q<32; q++)//iNumIndices; q++)
                                {
                                    uint ind1 = mappedV1[q];
                      //              if((q%3) == 0)qDebug(" \n");
                                    qDebug() << QString("GLVertexBuffers::bindIndexBuffer  color  q=%0 ind1=%5 color=%1 %2 %3 %4 ")
                                                .arg(q)
                                                .arg(mappedV3[ind1].x()).arg(mappedV3[ind1].y()).arg(mappedV3[ind1].z()).arg(mappedV3[ind1].w())
                                                .arg(ind1);

                                }
                                colorBuffer->unmap();
                                colorBuffer->release();
                                indexBuffer->unmap();
//*/

        glDrawElements(GL_POINTS, iNumIndices, IndexTypeEnum, 0);
//        uint error = glGetError();
        indexBuffer->release();

        if(bCol)glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        if(bText)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(bNor)
            glDisableClientState(GL_NORMAL_ARRAY);
    }
}

void GLVertexBuffers::bindIndexSegmentsBuffer(int iNumIndices)
{
    if(indexBuffer){
        glEnableClientState(GL_VERTEX_ARRAY);
//        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        indexBuffer->bind();
        glDrawElements(GL_TRIANGLES, iNumIndices, IndexTypeEnum, 0);
//                           if(m_iID == 1){
//                                IndexType *mappedV = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
//                                for(int q=0; q<iNumIndices; q++)
//                                {
//                                    qDebug() << QString("GLVertexBuffers::bindIndexSegmentsBuffer     indexBuffer  q=%0 index=%1  ").arg(q).arg(mappedV[q]);
//                                    if((q%3) == 0)qDebug(" \n");
//                                }
//                                indexBuffer->unmap();
//                                qDebug() << QString("GLVertexBuffers::bindIndexSegmentsBuffer       numIndices=%1").arg(iNumIndices);
//                           }
        indexBuffer->release();
        glDisableClientState(GL_VERTEX_ARRAY);
//        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//        qDebug() << QString("GLVertexBuffers::bindIndexBuffer ____ ");
    }

}

void GLVertexBuffers::bindIndexBufferAll(int iNumIndices, char* mypcl, bool bText, bool bNor, bool bPrint)
{
	if (iNumIndices < 1)return;
	qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   iNumIndices=%1 ").arg(iNumIndices);
    if(indexBuffer){
        glEnableClientState(GL_VERTEX_ARRAY);
//        glEnableClientState(GL_COLOR_ARRAY);
        if(bText)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(bNor)
            glEnableClientState(GL_NORMAL_ARRAY);

        vertexBuffer->bind();
        glVertexPointer(3, GL_FLOAT, (3 * sizeof(float)) + 3, mypcl);
//        colorBuffer->bind();
////        glColorPointer(4, GL_BYTE, 0, 0);
//		glColorPointer(3, GL_BYTE, 3 * sizeof(char), mypcl + 3*sizeof(float));


        int iNum = iNumIndices * 3;
//        int iNum = 20 * 3;
        indexBuffer->bind();
                            if(bPrint){
                                int n = 0;
                                QVector3D pt1, pt2, pt3;
                                IndexType *mappedV1 = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
                                vertexBuffer->bind();
                                QVector3D *mappedV2 = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
                                float sum = 0;
                                if(mappedV1 != 0){
                                    for(int q=0; q<iNum; q++)
                                    {
                                        uint ind1 = mappedV1[q];
                                        if((q%3) == 0){
                                                    QTextStream out(stdout);
                                                    out << QString("\n_line\n%1,%2,%3\n%4,%5,%6\n%7,%8,%9\nC\n")
                                                                .arg(pt1.x()).arg(pt1.y()).arg(pt1.z())
                                                                .arg(pt2.x()).arg(pt2.y()).arg(pt2.z())
                                                                .arg(pt3.x()).arg(pt3.y()).arg(pt3.z())
                                                                << endl;
                                            sum += (pt2.x() - pt1.x())*(pt2.y() + pt1.y())*(pt2.z() + pt1.z());
                                            n=0;
                                        }
										if (mappedV2 != 0) {
											qDebug() << QString("GLVertexBuffers::bindIndexBufferAll    q=%0 ind1=%4 vertex= %1,%2,%3  n=%5")
												.arg(q)
												.arg(mappedV2[ind1].x()).arg(mappedV2[ind1].y()).arg(mappedV2[ind1].z())
												.arg(ind1).arg(n);
											if (n == 0)pt1 = QVector3D(mappedV2[ind1].x(), mappedV2[ind1].y(), mappedV2[ind1].z());
											if (n == 1)pt2 = QVector3D(mappedV2[ind1].x(), mappedV2[ind1].y(), mappedV2[ind1].z());
											if (n == 2)pt3 = QVector3D(mappedV2[ind1].x(), mappedV2[ind1].y(), mappedV2[ind1].z());
										}
                                        n++;
                                    }
                                }
                                vertexBuffer->unmap();
                                vertexBuffer->release();
                                indexBuffer->unmap();
                            }
//*/
//#define GL_TRIANGLES				0x0004
//#define GL_TRIANGLE_STRIP			0x0005
//#define GL_TRIANGLE_FAN			0x0006
		qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   aaaaaaaaaaaaa    iNumIndices=%1 ").arg(iNumIndices);

		glDrawElements(GL_POINTS, iNumIndices, IndexTypeEnum, 0);
		//glGetError();
//        glDrawElements(GL_TRIANGLES, iNum, IndexTypeEnum, (void*)0);//(GLvoid*)indices);
//        glDrawArrays(GL_TRIANGLE_STRIP, 0, iNumIndices);
        //uint error = glGetError();
		qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   bbbbbbbbbbbbb    iNumIndices=%1 ").arg(iNumIndices);
		indexBuffer->release();
		qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   ccccccccccccc    iNumIndices=%1 ").arg(iNumIndices);
		vertexBuffer->release();
//		colorBuffer->release();
		qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   ddddddddddddd    iNumIndices=%1 ").arg(iNumIndices);

/*
        indexBuffer->bind();
        IndexType *mappedV10 = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
        texBuffer->bind();
        QVector2D *mappedV3 = (QVector2D *)texBuffer->map(QGLBuffer::WriteOnly);
        for(int q=0; q<iNumIndices; q++)
        {
            uint ind1 = mappedV10[q];
            qDebug() << QString("GLVertexBuffers::bindIndexBuffer    q=%0 ind1=%1 tex=%2 %3   ")
                        .arg(q)
                        .arg(ind1)
                        .arg(mappedV3[ind1].x()).arg(mappedV3[ind1].y());
        }
        texBuffer->unmap();
        texBuffer->release();
        indexBuffer->unmap();
        indexBuffer->release();
//*/
/*        if(bNor){
            indexBuffer->bind();
            IndexType *mappedV11 = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
            normalBuffer->bind();
            QVector3D *mappedV4 = (QVector3D *)normalBuffer->map(QGLBuffer::WriteOnly);
            for(int q=0; q<iNumIndices; q++)
            {
                uint ind1 = mappedV11[q];
                qDebug() << QString("GLVertexBuffers::bindIndexBuffer  normals      q=%0 ind1=%1 normal=%2 %3 %4  ")
                            .arg(q)
                            .arg(ind1)
                            .arg(mappedV4[ind1].x()).arg(mappedV4[ind1].y()).arg(mappedV4[ind1].z());
            }
            normalBuffer->unmap();
            normalBuffer->release();
            indexBuffer->unmap();
            indexBuffer->release();
        }
//*/

        glDisableClientState(GL_VERTEX_ARRAY);
//        glDisableClientState(GL_COLOR_ARRAY);
        if(bText)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(bNor)
            glDisableClientState(GL_NORMAL_ARRAY);
		qDebug() << QString("GLVertexBuffers::bindIndexBufferAll   eeeeeeeeeeeee    iNumIndices=%1 ").arg(iNumIndices);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void GLVertexBuffers::writeAllBuffer(uchar *pcl, int iNumWrite, bool bPrint)
{
	if (!allBuffer)return;
	if (iNumWrite < 1)return;
//	int num = iNumWrite * 15;// (sizeof(QVector3D) + sizeof(QVector3Du));
	int num = 10000 * 16;// (sizeof(QVector3D) + sizeof(QVector3Du));
	uint offset = 0;

	qDebug() << QString("GLVertexBuffers::writeAllBuffer   iNumWrite=%1  pcl=%2 size=%3  QVector3D=%4 QVector3Du=%5 pcl_size=%6 ").arg(iNumWrite).arg((qlonglong)pcl).arg(num).arg(sizeof(QVector3D)).arg(sizeof(QVector4Du)).arg(_msize(pcl));
	allBuffer->bind();
	allBuffer->write(offset, pcl, num);

	if (bPrint) {
		num = 12;
		QVector4D *mappedV = (QVector4D *)allBuffer->map(QGLBuffer::WriteOnly);
//		QVector3Du *mappedU = (QVector3Du *)allBuffer->map(QGLBuffer::WriteOnly);
		for (int q = 0; q < num; q++)
		{
			if ((q % 3) == 0)qDebug() << " \n";
//			qDebug() << QString("GLVertexBuffers::writeAllBuffer     q=%0 vertexW=%1 %2 %3 color=%4 %5 %6 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
			qDebug() << QString("GLVertexBuffers::writeAllBuffer     q=%0 vertexW=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
		}//pcl
		allBuffer->unmap();
		qDebug() << QString("GLVertexBuffers::writeAllBuffer     iNumWrite=%1 \n").arg(iNumWrite);
	}
	allBuffer->release();
}

void GLVertexBuffers::writeVertexBuffer(QVector3D *posns, int iNumWrite, bool bPrint)
{
    if(!vertexBuffer)return;
    int num = iNumWrite;
    uint offset = 0;

//    KSHOW_TRACE(QString(" GLVertexBuffers::writeVertexBuffer            meshSize=%1 m_iNumNurbs=%2 ").arg(meshSize).arg(m_iNumNurbs));
	if (vertexBuffer->bind()) {
		vertexBuffer->write(offset, posns, num * sizeof(QVector3D));
		if (bPrint) {
			int iStart = STAR_PCL_DEBUG;
			int iEnd = STAR_PCL_DEBUG + 12;
			QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
			for (int q = iStart; q < iEnd; q++)
			{
				if ((q % 3) == 0)qDebug() << " \n";
				qDebug() << QString("GLVertexBuffers::writeVertexBuffer     q=%0 vertexW=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
			}//posns
			vertexBuffer->unmap();
			qDebug() << QString("GLVertexBuffers::writeVertexBuffer     iNumWrite=%1 \n").arg(iNumWrite);
		}
	}
    vertexBuffer->release();
}

void GLVertexBuffers::writeColorBuffer(QVector4Du *colors, int iNumWrite, bool bPrint)
{
    if(!colorBuffer)return;
    int num =  iNumWrite;
    uint offset = 0;

	//for (int m = 0; m < 30; m++) {
	//	qDebug() << QString("colors:  m=%1  colors=%2 %3 %4 %5").arg(m).arg(colors[m].x()).arg(colors[m].y()).arg(colors[m].z()).arg(colors[m].w());
	//}
	//qDebug() << QString("iNumWrite=%1").arg(iNumWrite);
	//for (int n = 0; n < iNumWrite; n++) {
	//	colors[n] = QVector4Du(0, 255, 0, 255);
	//}
	if (colorBuffer->bind()) {
		colorBuffer->write(offset, colors, num * sizeof(QVector4Du));
		if (bPrint) {
			//for (int m = 0; m < 30; m++) {
			//	qDebug() << QString("colors:  m=%1  colors=%2 %3 %4 %5").arg(m).arg(colors[m].x()).arg(colors[m].y()).arg(colors[m].z()).arg(colors[m].w());
			//}

			int iStart = STAR_PCL_DEBUG;
			int iEnd = STAR_PCL_DEBUG + 12;
			QVector4Du *mappedV = (QVector4Du *)colorBuffer->map(QGLBuffer::WriteOnly);
			if (mappedV) {
				for (int q = iStart; q < iEnd; q++)
				{
				//	if ((q % 3) == 0)qDebug() << " \n";
					qDebug() << QString("q=%0 color4= %1 %2 %3 %4 ").arg(q).arg(mappedV->x()).arg(mappedV->y()).arg(mappedV->z()).arg(mappedV->w());
				}
				qDebug() << " \n";
			}
			colorBuffer->unmap();
		}
		colorBuffer->release();
	}
}

void GLVertexBuffers::writeIndexBuffer(IndexType *indices, int iNumWrite, bool bPrint)
{
    if(!indexBuffer)return;
	if (iNumWrite < 1)return;
    int num = iNumWrite;// * 3;
    uint offset = 0;

//    KSHOW_TRACE(QString(" GLVertexBuffers::writeVertexBuffer            meshSize=%1 m_iNumNurbs=%2 ").arg(meshSize).arg(m_iNumNurbs));
    indexBuffer->bind();
    indexBuffer->write(offset, indices, num * sizeof(IndexType));
    if(bPrint){
		int iStart = STAR_PCL_DEBUG;
		int iEnd = STAR_PCL_DEBUG + 12;
		IndexType *mappedV = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
        for(int q=iStart; q<iEnd; q++)
        {
            //if((q % 3) == 0)qDebug() << " \n";
            qDebug() << QString("GLVertexBuffers::writeIndexBuffer      q=%0 index=%1 ").arg(q).arg(mappedV[q]);
        }
        indexBuffer->unmap();
        qDebug() << QString("GLVertexBuffers::writeIndexBuffer     iNumWrite=%1 \n").arg(iNumWrite);
    }
    indexBuffer->release();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void GLVertexBuffers::allocVertexBufferMem (int iNumIndices, int iNumBlocks)
{
    if(iNumIndices < 1)return;
    if(iNumBlocks < 1)return;

    posns = new QVector3D[iNumBlocks * iNumIndices];// * sizeof(QVector3D) * 3];
    memset(posns, 0, iNumBlocks * iNumIndices * sizeof(QVector3D));//sizeof(QVector3D) * 3);

    uint tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(posns);
//#else
//    tmem = (int)malloc_usable_size(posns);
//#endif
#ifdef Q_OS_LINUX
    tmem = (uint)malloc_usable_size(posns);
#else
    tmem= (uint)_msize(posns);
#endif
    qDebug() << QString("GLVertexBuffers::allocVertexBufferMem     m_iID=%6   posns=%1 iNumBlocks=%2 iNumIndices=%3 QVector3d=%4  total=%5 ").arg(tmem).arg(iNumBlocks).arg(iNumIndices).arg(sizeof(QVector3D)).arg(iNumBlocks * iNumIndices * sizeof(QVector3D) * 3).arg(m_iID);

}

void GLVertexBuffers::freeVertexBufferMem()
{
    if(posns){
        delete posns;
        posns = 0;
    }
}

void GLVertexBuffers::clearVertexBufferMem(int iNumTriangles, int iNumBlocks)
{
    if(iNumTriangles < 1)return;
    if(iNumBlocks < 1)return;
    if(!posns)return;

    int num = iNumBlocks * iNumTriangles * sizeof(QVector3D);
    memset(posns, 0, num);
	qDebug() << QString("GLVertexBuffers::clearVertexBufferMem    num=%1 ").arg(num);
}

void GLVertexBuffers::allocColorBufferMem(int iNumIndices, int iNumBlocks)
{
    if(iNumIndices < 1)return;
    if(iNumBlocks < 1)return;

	colors = new QVector4Du[iNumBlocks * iNumIndices];// *sizeof(QVector4Du)];
    memset(colors, 0, iNumBlocks * iNumIndices * sizeof(QVector4Du));

    int tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(colors);
//#else
//    tmem = (int)malloc_usable_size(colors);
//#endif
#ifdef Q_OS_LINUX
    tmem = (int)malloc_usable_size(colors);
#else
    tmem= (int)_msize(colors);
#endif
//    qDebug() << QString("GLVertexBuffers::allocColorBufferMem     colors=%1 ").arg(tmem);
    qDebug() << QString("GLVertexBuffers::allocColorBufferMem     m_iID=%6  colors=%1 iNumBlocks=%2 iNumIndices=%3 QVector4Du=%4  total=%5 ").arg(tmem).arg(iNumBlocks).arg(iNumIndices).arg(sizeof(QVector4Du)).arg(iNumBlocks * iNumIndices * sizeof(QVector4Du)).arg(m_iID);

}

void GLVertexBuffers::freeColorBufferMem()
{
    if(colors){
        delete colors;
        colors = 0;
    }
}

void GLVertexBuffers::clearColorBufferMem(int iNumTriangles, int iNumBlocks)
{
    if(iNumTriangles < 1)return;
    if(iNumBlocks < 1)return;
    if(!colors)return;

    int num = iNumBlocks * iNumTriangles * sizeof(QVector4Du);
    memset(colors, 0, num);
	qDebug() << QString("GLVertexBuffers::clearColorBufferMem    num=%1 ").arg(num);
}

void GLVertexBuffers::allocIndexBufferMem(int iNumIndices, int iNumBlocks)
{
    qDebug() << QString("GLVertexBuffers::allocIndexBufferMem  iNumIndices=%1 iNumBlocks=%2 ").arg(iNumIndices).arg(iNumBlocks);
    if(iNumIndices < 1)return;
    if(iNumBlocks < 1)return;

    indices = new IndexType[iNumBlocks * iNumIndices];
	memset(indices, 0, iNumBlocks * iNumIndices * sizeof(IndexType));


    int tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(indices);
//#else
//    tmem = (int)malloc_usable_size(indices);
//#endif
#ifdef Q_OS_LINUX
    tmem = (int)malloc_usable_size(indices);
#else
    tmem= (int)_msize(indices);
#endif
//    qDebug() << QString("GLVertexBuffers::allocIndexBufferMem     indices=%1 ").arg(tmem);
    qDebug() << QString("GLVertexBuffers::allocIndexBufferMem     m_iID=%6  indices=%1 iNumBlocks=%2 iNumIndices=%3 IndexType=%4  total=%5 ").arg(tmem).arg(iNumBlocks).arg(iNumIndices).arg(sizeof(IndexType)).arg(iNumBlocks * iNumIndices * sizeof(IndexType)).arg(m_iID);

}

void GLVertexBuffers::freeIndexBufferMem()
{
    if(indices){
        //delete indices;
        free(indices);
        indices = 0;
    }
}

void GLVertexBuffers::clearIndexBufferMem(int iNumIndices, int iNumBlocks)
{
    if(iNumIndices < 1)return;
    if(iNumBlocks < 1)return;
    if(!indices)return;

    int num = iNumBlocks * iNumIndices;// * sizeof(IndexType);
    memset(indices, 0, num);
}
/*
void GLVertexBuffers::allocAllBufferMem(int iNumIndices, int iNumBlocks)
{
	if (iNumIndices < 1)return;
	if (iNumBlocks < 1)return;

	posns = new QVector3D[iNumBlocks * iNumIndices];// * sizeof(QVector3D) * 3];
	memset(posns, 0, iNumBlocks * iNumIndices * sizeof(QVector3D));//sizeof(QVector3D) * 3);

	uint tmem = 0;
	//#ifdef Q_WS_WIN
	//    tmem= _msize(posns);
	//#else
	//    tmem = (int)malloc_usable_size(posns);
	//#endif
#ifdef Q_OS_LINUX
	tmem = (uint)malloc_usable_size(posns);
#else
	tmem = (uint)_msize(posns);
#endif
	qDebug() << QString("GLVertexBuffers::allocVertexBufferMem     m_iID=%6   posns=%1 iNumBlocks=%2 iNumIndices=%3 QVector3d=%4  total=%5 ").arg(tmem).arg(iNumBlocks).arg(iNumIndices).arg(sizeof(QVector3D)).arg(iNumBlocks * iNumIndices * sizeof(QVector3D) * 3).arg(m_iID);

}

void GLVertexBuffers::freeAllBufferMem()
{
	if (posns) {
		delete posns;
		posns = 0;
	}
}

void GLVertexBuffers::clearAllBufferMem(int iNumTriangles, int iNumBlocks)
{
	if (iNumTriangles < 1)return;
	if (iNumBlocks < 1)return;
	if (!posns)return;

	int num = iNumBlocks * iNumTriangles;// * sizeof(QVector3D) * 3;
	memset(posns, 0, num);
}
//*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void GLVertexBuffers::allocBuffers(int iNumVertices, int iNumIndices, int iMaxMem, int iNumBlocks)
{
    if(iNumVertices < 1)return;
    if(iNumIndices < 1) return;
    if(iNumBlocks < 1)return;
    qDebug() << QString("GLVertexBuffers::allocBuffers    iNumTriangles=%1 iNumIndices=%2  ******************************************** ").arg(iNumVertices).arg(iNumIndices);
    qDebug() << QString("Vertex memory=%1 ").arg(iNumIndices * sizeof(QVector3D) * 3);
//    setMemIndices(iNumTriangles);
//    setMemIndicesContourn(iNumTriangles);

    allocVertexBufferMem(iMaxMem, iNumBlocks);
    allocColorBufferMem(iMaxMem, iNumBlocks);
    allocIndexBufferMem(iMaxMem, iNumBlocks);

    qDebug() << QString("In RAM CPU memory created");
    allocVertexBuffer(iMaxMem);
    allocColorBuffer(iMaxMem);
    allocIndexBuffer(iMaxMem);
	allocAllBuffer(iMaxMem);

	int iPosBlock = 0;// 1 * BUFFERMEM;

    qDebug() << QString("In GPU memory created");
    // clear the Vertex Buffer
    clearVertexBufferMem(iNumVertices, iNumBlocks);
    writeVertexBuffer(posns, iNumVertices, false);

    // Clear the Color Buffer
    clearColorBufferMem(iNumVertices, iNumBlocks);
    writeColorBuffer(colors, iNumVertices, false);

    clearIndexBufferMem(iNumIndices, iNumBlocks);
    writeIndexBuffer(indices, iNumIndices, false);

    qDebug() << QString("GLVertexBuffers::allocBuffers     end ...........");
}

void GLVertexBuffers::freeBuffers()
{
    freeVertexBufferMem();
    freeColorBufferMem();
    freeIndexBufferMem();
    freeVertexBuffer();
    freeColorBuffer();
    freeIndexBuffer();
	freeAllBuffer();
}

void GLVertexBuffers::printSizes()
{
    qDebug() << QString("\n\nGLVertexBuffers::printSizes ------------- ID=%1 ---------------------------- ").arg(m_iID);
    uint tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(posns);
//#else
//    tmem = (int)malloc_usable_size(posns);
//#endif
#ifdef Q_OS_LINUX
    tmem = (int)malloc_usable_size(posns);
#else
    tmem= (int)_msize(posns);
#endif
    qDebug() << QString("GLVertexBuffers::printSizes     posns=%1 ").arg(tmem);

    tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(texcs);
//#else
//    tmem = (int)malloc_usable_size(texcs);
//#endif

    tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(colors);
//#else
//    tmem = (int)malloc_usable_size(colors);
//#endif
#ifdef Q_OS_LINUX
    tmem = (int)malloc_usable_size(colors);
#else
    tmem= (int)_msize(colors);
#endif
    qDebug() << QString("GLVertexBuffers::printSizes     colors=%1 ").arg(tmem);

    tmem = 0;
//#ifdef Q_WS_WIN
//    tmem= _msize(indices);
//#else
//    tmem = (int)malloc_usable_size(indices);
//#endif
#ifdef Q_OS_LINUX
    tmem = (int)malloc_usable_size(indices);
#else
    tmem= (int)_msize(indices);
#endif
    qDebug() << QString("GLVertexBuffers::printSizes     indices=%1 ").arg(tmem);


    Q_ASSERT(vertexBuffer);
    if (vertexBuffer) {
        if(vertexBuffer->bind()){
            if (vertexBuffer->map(QGLBuffer::WriteOnly)) {
                qDebug() << QString("vertexBuffer allocated size .......................................  size=%1  ").arg(vertexBuffer->size());
                vertexBuffer->unmap();
										int iStart = STAR_PCL_DEBUG;
										int iEnd = STAR_PCL_DEBUG + 12;
										QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
										if (mappedV) {
											for (int q = iStart; q < iEnd; q++)
											{
												//    if((q % 4) == 0)qDebug() << " \n";
												qDebug() << QString("q=%0 vertexP=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
											}
										}
                                        vertexBuffer->unmap();
				vertexBuffer->release();
            }
        }
    }
	Q_ASSERT(colorBuffer);
	if (colorBuffer) {
		if (colorBuffer->bind()) {
			if (colorBuffer->map(QGLBuffer::WriteOnly)) {
				qDebug() << QString("colorBuffer allocated size .......................................  size=%1  ").arg(colorBuffer->size());
				colorBuffer->unmap();
														int iStart = STAR_PCL_DEBUG;
														int iEnd = STAR_PCL_DEBUG + 12;
														QVector4Du *mappedV = (QVector4Du *)colorBuffer->map(QGLBuffer::WriteOnly);
														if (mappedV) {
															for (int q = iStart; q < iEnd; q++)
															{
																//    if((q % 4) == 0)qDebug() << " \n";
																qDebug() << QString("Color  q=%0 vertexP=%1 %2 %3 %4 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z()).arg(mappedV[q].w());
															}
														}
														colorBuffer->unmap();
				colorBuffer->release();
			}
		}
	}

    Q_ASSERT(indexBuffer);
    if (indexBuffer) {
        if(indexBuffer->bind()){
            if (indexBuffer->map(QGLBuffer::WriteOnly)) {
                qDebug() << QString("indexBuffer allocated size .......................................  size=%1  ").arg(indexBuffer->size());
                indexBuffer->unmap();
										int iStart = STAR_PCL_DEBUG;
										int iEnd = STAR_PCL_DEBUG + 12;
										IndexType *mappedV = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
										if (mappedV) {
											for (int q = iStart; q < iEnd; q++)
											{
												//if((q % 3) == 0)qDebug() << " \n";
												qDebug() << QString("Index      q=%0 index=%1 ").arg(q).arg(mappedV[q]);
											}
										}
										indexBuffer->unmap();
                indexBuffer->release();
            }
        }
    }
	Q_ASSERT(allBuffer);
	if (allBuffer) {
		if (allBuffer->bind()) {
			if (allBuffer->map(QGLBuffer::WriteOnly)) {
				qDebug() << QString("allBuffer allocated size .......................................  size=%1  ").arg(allBuffer->size());
				allBuffer->unmap();
				//                                    if(m_iID == 2){
				//                                        int num = 10;
				//                                        QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
				//                                        for(int q=0; q<num; q++)
				//                                        {
				//                                        //    if((q % 4) == 0)qDebug() << " \n";
				//                                            qDebug() << QString("q=%0 vertexP=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
				//                                        }
				//                                        vertexBuffer->unmap();
				//                                    }
				allBuffer->release();
			}
		}
	}
	qDebug() << QString("GLVertexBuffers::printSizes ------------------- end ---------------------- \n\n");

}

void GLVertexBuffers::printIndexSegmentsBuffer(int iNumWrite)
{
    if(!indexBuffer)return;
    int num = iNumWrite;

//    KSHOW_TRACE(QString(" GLVertexBuffers::writeVertexBuffer            meshSize=%1 m_iNumNurbs=%2 ").arg(meshSize).arg(m_iNumNurbs));
    indexBuffer->bind();
                        IndexType *mappedV = (IndexType *)indexBuffer->map(QGLBuffer::WriteOnly);
                        for(int q=0; q<num; q++)
                        {
                        //    if((q % 4) == 0)qDebug() << " \n";
                            qDebug() << QString("GLVertexBuffers::writeIndexSegmentsBuffer      q=%0 index=%1 ").arg(q).arg(mappedV[q]);
                        }
                        indexBuffer->unmap();
                        qDebug() << QString("GLVertexBuffers::writeIndexSegmentsBuffer     iNumWrite=%1 \n").arg(iNumWrite);
    indexBuffer->release();
}

void GLVertexBuffers::printVertexBuffer(int iNumWrite)
{
    if(!vertexBuffer)return;
    int num = iNumWrite;
    //uint offset = 0;

//    KSHOW_TRACE(QString(" GLVertexBuffers::writeVertexBuffer            meshSize=%1 m_iNumNurbs=%2 ").arg(meshSize).arg(m_iNumNurbs));
    vertexBuffer->bind();
                        QVector3D *mappedV = (QVector3D *)vertexBuffer->map(QGLBuffer::WriteOnly);
                        for(int q=0; q<num; q++)
                        {
                        //    if((q % 4) == 0)qDebug() << " \n";
                            qDebug() << QString("GLVertexBuffers::writeVertexBuffer     q=%0 vertexW=%1 %2 %3 ").arg(q).arg(mappedV[q].x()).arg(mappedV[q].y()).arg(mappedV[q].z());
                        }
                        vertexBuffer->unmap();
                        qDebug() << QString("GLVertexBuffers::writeVertexBuffer     iNumWrite=%1 \n").arg(iNumWrite);
    vertexBuffer->release();
}

