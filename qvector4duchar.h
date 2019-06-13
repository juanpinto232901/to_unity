/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVECTOR4DU_H
#define QVECTOR4DU_H

#include <QtGui/qtguiglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE


//class QMatrix4x4;
//class QVector2D;
//class QVector3D;

#ifndef QT_NO_VECTOR4DU

// Not uncomment this. It produces this error:
// GLVertexBuffers.obj:-1: error: LNK2019: unresolved external symbol "__declspec(dllimport) public: __cdecl QVector4Du::QVector4Du(void)" (__imp_??0QVector4Du@@QEAA@XZ) referenced in function "public: void __cdecl GLVertexBuffers::allocColorBufferMem(int,int)" (?allocColorBufferMem@GLVertexBuffers@@QEAAXHH@Z)
class /*Q_GUI_EXPORT*/ QVector4Du
{
public:
    Q_DECL_CONSTEXPR QVector4Du();
    explicit QVector4Du(Qt::Initialization) {}
    Q_DECL_CONSTEXPR QVector4Du(uchar xpos, uchar ypos, uchar zpos, uchar wpos);
//    Q_DECL_CONSTEXPR explicit QVector4D(const QPoint& point);
//    Q_DECL_CONSTEXPR explicit QVector4D(const QPointF& point);
//#ifndef QT_NO_VECTOR2D
//    QVector4D(const QVector2D& vector);
//    QVector4D(const QVector2D& vector, float zpos, float wpos);
//#endif
//#ifndef QT_NO_VECTOR3D
//    QVector4D(const QVector3D& vector);
//    QVector4D(const QVector3D& vector, float wpos);
//#endif

    bool isNull() const;

    Q_DECL_CONSTEXPR uchar x() const;
    Q_DECL_CONSTEXPR uchar y() const;
    Q_DECL_CONSTEXPR uchar z() const;
    Q_DECL_CONSTEXPR uchar w() const;

    void setX(uchar x);
    void setY(uchar y);
    void setZ(uchar z);
    void setW(uchar w);

    uchar &operator[](int i);
    uchar operator[](int i) const;

    float length() const;
    float lengthSquared() const; //In Qt 6 convert to inline and constexpr

    Q_REQUIRED_RESULT QVector4Du normalized() const;
    void normalize();

    QVector4Du &operator+=(const QVector4Du &vector);
    QVector4Du &operator-=(const QVector4Du &vector);
    QVector4Du &operator*=(float factor);
    QVector4Du &operator*=(const QVector4Du &vector);
    QVector4Du &operator/=(float divisor);
    inline QVector4Du &operator/=(const QVector4Du &vector);

    static float dotProduct(const QVector4Du& v1, const QVector4Du& v2); //In Qt 6 convert to inline and constexpr

    Q_DECL_CONSTEXPR friend inline bool operator==(const QVector4Du &v1, const QVector4Du &v2);
    Q_DECL_CONSTEXPR friend inline bool operator!=(const QVector4Du &v1, const QVector4Du &v2);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator+(const QVector4Du &v1, const QVector4Du &v2);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator-(const QVector4Du &v1, const QVector4Du &v2);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator*(float factor, const QVector4Du &vector);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator*(const QVector4Du &vector, float factor);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator*(const QVector4Du &v1, const QVector4Du& v2);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator-(const QVector4Du &vector);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator/(const QVector4Du &vector, float divisor);
    Q_DECL_CONSTEXPR friend inline const QVector4Du operator/(const QVector4Du &vector, const QVector4Du &divisor);

    Q_DECL_CONSTEXPR friend inline bool qFuzzyCompare(const QVector4Du& v1, const QVector4Du& v2);

//#ifndef QT_NO_VECTOR2D
//    QVector2D toVector2D() const;
//    QVector2D toVector2DAffine() const;
//#endif
//#ifndef QT_NO_VECTOR3D
//    QVector3D toVector3D() const;
//    QVector3D toVector3DAffine() const;
//#endif

//    Q_DECL_CONSTEXPR QPoint toPoint() const;
//    Q_DECL_CONSTEXPR QPointF toPointF() const;

    operator QVariant() const;

private:
    uchar v[4];

//    friend class QVector2D;
//    friend class QVector3D;
//#ifndef QT_NO_MATRIX4X4
//    friend QVector4D operator*(const QVector4D& vector, const QMatrix4x4& matrix);
//    friend QVector4D operator*(const QMatrix4x4& matrix, const QVector4D& vector);
//#endif
};

Q_DECLARE_TYPEINFO(QVector4Du, Q_PRIMITIVE_TYPE);

Q_DECL_CONSTEXPR inline QVector4Du::QVector4Du() : v{0, 0, 0, 0} {}

Q_DECL_CONSTEXPR inline QVector4Du::QVector4Du(uchar xpos, uchar ypos, uchar zpos, uchar wpos) : v{xpos, ypos, zpos, wpos} {}

//Q_DECL_CONSTEXPR inline QVector4D::QVector4D(const QPoint& point) : v{float(point.x()), float(point.y()), 0.0f, 0.0f} {}

//Q_DECL_CONSTEXPR inline QVector4D::QVector4D(const QPointF& point) : v{float(point.x()), float(point.y()), 0.0f, 0.0f} {}

//inline bool QVector4Du::isNull() const
//{
//    return qIsNull(v[0]) && qIsNull(v[1]) && qIsNull(v[2]) && qIsNull(v[3]);
//}

Q_DECL_CONSTEXPR inline uchar QVector4Du::x() const { return v[0]; }
Q_DECL_CONSTEXPR inline uchar QVector4Du::y() const { return v[1]; }
Q_DECL_CONSTEXPR inline uchar QVector4Du::z() const { return v[2]; }
Q_DECL_CONSTEXPR inline uchar QVector4Du::w() const { return v[3]; }

inline void QVector4Du::setX(uchar aX) { v[0] = aX; }
inline void QVector4Du::setY(uchar aY) { v[1] = aY; }
inline void QVector4Du::setZ(uchar aZ) { v[2] = aZ; }
inline void QVector4Du::setW(uchar aW) { v[3] = aW; }

inline uchar &QVector4Du::operator[](int i)
{
    Q_ASSERT(uint(i) < 4u);
    return v[i];
}

inline uchar QVector4Du::operator[](int i) const
{
    Q_ASSERT(uint(i) < 4u);
    return v[i];
}

inline QVector4Du &QVector4Du::operator+=(const QVector4Du &vector)
{
    v[0] += vector.v[0];
    v[1] += vector.v[1];
    v[2] += vector.v[2];
    v[3] += vector.v[3];
    return *this;
}

inline QVector4Du &QVector4Du::operator-=(const QVector4Du &vector)
{
    v[0] -= vector.v[0];
    v[1] -= vector.v[1];
    v[2] -= vector.v[2];
    v[3] -= vector.v[3];
    return *this;
}

inline QVector4Du &QVector4Du::operator*=(float factor)
{
    v[0] *= factor;
    v[1] *= factor;
    v[2] *= factor;
    v[3] *= factor;
    return *this;
}

inline QVector4Du &QVector4Du::operator*=(const QVector4Du &vector)
{
    v[0] *= vector.v[0];
    v[1] *= vector.v[1];
    v[2] *= vector.v[2];
    v[3] *= vector.v[3];
    return *this;
}

inline QVector4Du &QVector4Du::operator/=(float divisor)
{
    v[0] /= divisor;
    v[1] /= divisor;
    v[2] /= divisor;
    v[3] /= divisor;
    return *this;
}

inline QVector4Du &QVector4Du::operator/=(const QVector4Du &vector)
{
    v[0] /= vector.v[0];
    v[1] /= vector.v[1];
    v[2] /= vector.v[2];
    v[3] /= vector.v[3];
    return *this;
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wfloat-equal")
QT_WARNING_DISABLE_GCC("-Wfloat-equal")
Q_DECL_CONSTEXPR inline bool operator==(const QVector4Du &v1, const QVector4Du &v2)
{
    return v1.v[0] == v2.v[0] && v1.v[1] == v2.v[1] && v1.v[2] == v2.v[2] && v1.v[3] == v2.v[3];
}

Q_DECL_CONSTEXPR inline bool operator!=(const QVector4Du &v1, const QVector4Du &v2)
{
    return v1.v[0] != v2.v[0] || v1.v[1] != v2.v[1] || v1.v[2] != v2.v[2] || v1.v[3] != v2.v[3];
}
QT_WARNING_POP

Q_DECL_CONSTEXPR inline const QVector4Du operator+(const QVector4Du &v1, const QVector4Du &v2)
{
    return QVector4Du(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3]);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator-(const QVector4Du &v1, const QVector4Du &v2)
{
    return QVector4Du(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3]);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator*(uchar factor, const QVector4Du &vector)
{
    return QVector4Du(vector.x() * factor, vector.y() * factor, vector.z() * factor, vector.w() * factor);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator*(const QVector4Du &vector, uchar factor)
{
    return QVector4Du(vector.x() * factor, vector.y() * factor, vector.z() * factor, vector.w() * factor);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator*(const QVector4Du &v1, const QVector4Du& v2)
{
    return QVector4Du(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3]);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator-(const QVector4Du &vector)
{
    return QVector4Du(-vector.v[0], -vector.v[1], -vector.v[2], -vector.v[3]);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator/(const QVector4Du &vector, uchar divisor)
{
    return QVector4Du(vector.x() / divisor, vector.y() / divisor, vector.z() / divisor, vector.w() / divisor);
}

Q_DECL_CONSTEXPR inline const QVector4Du operator/(const QVector4Du &vector, const QVector4Du &divisor)
{
    return QVector4Du(vector.v[0] / divisor.v[0], vector.v[1] / divisor.v[1], vector.v[2] / divisor.v[2], vector.v[3] / divisor.v[3]);
}

//Q_DECL_CONSTEXPR inline bool qFuzzyCompare(const QVector4D& v1, const QVector4D& v2)
//{
//    return qFuzzyCompare(v1.v[0], v2.v[0]) &&
//           qFuzzyCompare(v1.v[1], v2.v[1]) &&
//           qFuzzyCompare(v1.v[2], v2.v[2]) &&
//           qFuzzyCompare(v1.v[3], v2.v[3]);
//}

//Q_DECL_CONSTEXPR inline QPoint QVector4D::toPoint() const
//{
//    return QPoint(qRound(v[0]), qRound(v[1]));
//}

//Q_DECL_CONSTEXPR inline QPointF QVector4D::toPointF() const
//{
//    return QPointF(qreal(v[0]), qreal(v[1]));
//}

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QVector4Du &vector);
#endif

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QVector4Du &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QVector4Du &);
#endif

#endif

QT_END_NAMESPACE

#endif
