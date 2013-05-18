// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

// -------------------------------------------------------------------------------------------------
//
// Math library
//
// Conventions:
//
// - Coordinate system is right-handed with positive y as up axis
// - All rotation angles are counter-clockwise when looking from the positive end of the rotation
//	 axis towards the origin
// - An unrotated view vector points along the negative z-axis
//
// -------------------------------------------------------------------------------------------------

#ifndef _utMath_H_
#define _utMath_H_

#include <cmath>


namespace Horde3D {

// Constants
namespace Math
{
	const unsigned int MaxUInt32 = 0xFFFFFFFF;
	const int MinInt32 = 0x80000000;
	const int MaxInt32 = 0x7FFFFFFF;
	const float MaxFloat = 3.402823466e+38F;
	const float MinPosFloat = 1.175494351e-38F;
	
	const float Pi = 3.141592654f;
	const float TwoPi = 6.283185307f;
	const float PiHalf = 1.570796327f;

	const float Epsilon = 0.000001f;
	const float ZeroEpsilon = 32.0f * MinPosFloat;  // Very small epsilon for checking against 0.0f
#ifdef __GNUC__
	const float NaN = __builtin_nanf("");
#else
	const float NaN = *(float *)&MaxUInt32;
#endif

	enum NoInitHint
	{
		NO_INIT
	};
};


// -------------------------------------------------------------------------------------------------
// General
// -------------------------------------------------------------------------------------------------

static inline float degToRad( float f ) 
{
	return f * 0.017453293f;
}

static inline float radToDeg( float f ) 
{
	return f * 57.29577951f;
}

static inline float minf( float a, float b )
{
	return a < b ? a : b;
}

static inline float maxf( float a, float b )
{
	return a > b ? a : b;
}

static inline float clamp( float f, float min, float max )
{
	return maxf(minf(f, max), min);;
}

static inline float fsel( float test, float a, float b )
{
	// Branchless selection
	return test >= 0.0f ? a : b;
}


// -------------------------------------------------------------------------------------------------
// Conversion
// -------------------------------------------------------------------------------------------------

static inline int ftoi_t( double val )
{
	// Float to int conversion using truncation
	
	return (int)val;
}

static inline int ftoi_r( double val )
{
	// Fast round (banker's round) using Sree Kotay's method
	// This function is much faster than a naive cast from float to int

	union
	{
		double dval;
		int ival[2];
	} u;

	u.dval = val + 6755399441055744.0;  // Magic number: 2^52 * 1.5;
	return u.ival[0];         // Needs to be [1] for big-endian
}


// -------------------------------------------------------------------------------------------------
// Vector
// -------------------------------------------------------------------------------------------------

class Vec3f
{
public:
	float x, y, z;
	
	
	// ------------
	// Constructors
	// ------------
	Vec3f() : x( 0.0f ), y( 0.0f ), z( 0.0f )
	{ 
	}

	explicit Vec3f( Math::NoInitHint )
	{
		// Constructor without default initialization
	}
	
	Vec3f( const float x, const float y, const float z ) : x( x ), y( y ), z( z ) 
	{
	}

	Vec3f( const Vec3f &v ) : x( v.x ), y( v.y ), z( v.z )
	{
	}

	// ------
	// Access
	// ------
	float &operator[]( unsigned int index )
	{
		return *(&x + index);
	}
	
	// -----------
	// Comparisons
	// -----------
	bool operator==( const Vec3f &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon && 
		        y > v.y - Math::Epsilon && y < v.y + Math::Epsilon &&
		        z > v.z - Math::Epsilon && z < v.z + Math::Epsilon);
	}

	bool operator!=( const Vec3f &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon || 
		        y < v.y - Math::Epsilon || y > v.y + Math::Epsilon ||
		        z < v.z - Math::Epsilon || z > v.z + Math::Epsilon);
	}
	
	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec3f operator-() const
	{
		return Vec3f( -x, -y, -z );
	}

	Vec3f operator+( const Vec3f &v ) const
	{
		return Vec3f( x + v.x, y + v.y, z + v.z );
	}

	Vec3f &operator+=( const Vec3f &v )
	{
		return *this = *this + v;
	}

	Vec3f operator-( const Vec3f &v ) const 
	{
		return Vec3f( x - v.x, y - v.y, z - v.z );
	}

	Vec3f &operator-=( const Vec3f &v )
	{
		return *this = *this - v;
	}

	Vec3f operator*( const float f ) const
	{
		return Vec3f( x * f, y * f, z * f );
	}

	Vec3f &operator*=( const float f )
	{
		return *this = *this * f;
	}

	Vec3f operator/( const float f ) const
	{
		const float invF = 1.0f / f;
		return Vec3f( x * invF, y * invF, z * invF );
	}

	Vec3f &operator/=( const float f )
	{
		return *this = *this / f;
	}

	// ----------------
	// Special products
	// ----------------
	float dot( const Vec3f &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3f cross( const Vec3f &v ) const
	{
		return Vec3f( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	// ----------------
	// Other operations
	// ----------------
	float length() const 
	{
		return sqrtf( x * x + y * y + z * z );
	}

	Vec3f normalized() const
	{
		const float invLen = 1.0f / length();
		return Vec3f( x * invLen, y * invLen, z * invLen );
	}

	void normalize()
	{
		const float invLen = 1.0f / length();
		x *= invLen;
		y *= invLen;
		z *= invLen;
	}

	/*void fromRotation( float angleX, float angleY )
	{
		x = cosf( angleX ) * sinf( angleY ); 
		y = -sinf( angleX );
		z = cosf( angleX ) * cosf( angleY );
	}*/

	Vec3f toRotation() const
	{
		// Assumes that the unrotated view vector is (0, 0, -1)
		Vec3f v;
		
		if( y != 0.0f ) v.x = atan2f( y, sqrtf( x*x + z*z ) );
		if( x != 0.0f || z != 0.0f ) v.y = atan2f( -x, -z );

		return v;
	}

	Vec3f lerp( const Vec3f &v, float f ) const
	{
		return Vec3f( x + (v.x - x) * f, y + (v.y - y) * f, z + (v.z - z) * f ); 
	}
};


class Vec4f
{
public:
	
	float x, y, z, w;


	Vec4f() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f )
	{
	}
	
	explicit Vec4f( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}

	explicit Vec4f( Vec3f v ) : x( v.x ), y( v.y ), z( v.z ), w( 1.0f )
	{
	}

	Vec4f operator+( const Vec4f &v ) const
	{
		return Vec4f( x + v.x, y + v.y, z + v.z, w + v.w );
	}

	Vec4f operator-() const
	{
		return Vec4f( -x, -y, -z, -w );
	}
	
	Vec4f operator*( const float f ) const
	{
		return Vec4f( x * f, y * f, z * f, w * f );
	}
};


// -------------------------------------------------------------------------------------------------
// Quaternion
// -------------------------------------------------------------------------------------------------

class Quaternion
{
public:	
	
	float x, y, z, w;

	// ------------
	// Constructors
	// ------------
	Quaternion() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) 
	{ 
	}
	
	explicit Quaternion( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}
	
	Quaternion( const float eulerX, const float eulerY, const float eulerZ )
	{
		const Quaternion roll( sinf( eulerX * 0.5f ), 0.0f, 0.0f, cosf( eulerX * 0.5f ) );
		const Quaternion pitch( 0.0f, sinf( eulerY * 0.5f ), 0.0f, cosf( eulerY * 0.5f ) );
		const Quaternion yaw( 0.0f, 0.0f, sinf( eulerZ * 0.5f ), cosf( eulerZ * 0.5f ) );
	
		// Order: y * x * z
		*this = pitch * roll * yaw;
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Quaternion operator*( const Quaternion &q ) const
	{
		return Quaternion(
			y * q.z - z * q.y + q.x * w + x * q.w,
			z * q.x - x * q.z + q.y * w + y * q.w,
			x * q.y - y * q.x + q.z * w + z * q.w,
			w * q.w - (x * q.x + y * q.y + z * q.z) );
	}

	Quaternion &operator*=( const Quaternion &q )
	{
		return *this = *this * q;
	}

	// ----------------
	// Other operations
	// ----------------

	Quaternion slerp( const Quaternion &q, const float t ) const
	{
		// Spherical linear interpolation between two quaternions
		// Note: SLERP is not commutative
		
		Quaternion q1( q );

        // Calculate cosine
        float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

        // Use the shortest path
        if( cosTheta < Math::ZeroEpsilon )
		{
			cosTheta = -cosTheta; 
			q1.x = -q.x; q1.y = -q.y;
			q1.z = -q.z; q1.w = -q.w;
        }

        // Initialize with linear interpolation
		float scale0 = 1.0f - t, scale1 = t;
		
		// Use spherical interpolation only if the quaternions are not very close
		if( (1.0f - 0.001f) > cosTheta )
		{
			// SLERP
			const float theta = acosf( cosTheta );
			const float invSinTheta = 1.0f / sinf( theta );
			scale0 = sinf( (1.0f - t) * theta ) * invSinTheta;
			scale1 = sinf( t * theta ) * invSinTheta;
		} 
		
		// Calculate final quaternion
		return Quaternion( x * scale0 + q1.x * scale1, y * scale0 + q1.y * scale1,
		                   z * scale0 + q1.z * scale1, w * scale0 + q1.w * scale1 );
	}

	Quaternion nlerp( const Quaternion &q, const float t ) const
	{
		// Normalized linear quaternion interpolation
		// Note: NLERP is faster than SLERP and commutative but does not yield constant velocity

		Quaternion qt;
		const float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;
		
		// Use the shortest path and interpolate linearly
		if( cosTheta < Math::ZeroEpsilon )
			qt = Quaternion( x + (-q.x - x) * t, y + (-q.y - y) * t,
							 z + (-q.z - z) * t, w + (-q.w - w) * t );
		else
			qt = Quaternion( x + (q.x - x) * t, y + (q.y - y) * t,
							 z + (q.z - z) * t, w + (q.w - w) * t );

		// Return normalized quaternion
		const float invLen = 1.0f / sqrtf( qt.x * qt.x + qt.y * qt.y + qt.z * qt.z + qt.w * qt.w );
		return Quaternion( qt.x * invLen, qt.y * invLen, qt.z * invLen, qt.w * invLen );
	}

	Quaternion inverted() const
	{
		const float len = x * x + y * y + z * z + w * w;
		if( len > Math::ZeroEpsilon )
        {
            float invLen = 1.0f / len;
            return Quaternion( -x * invLen, -y * invLen, -z * invLen, w * invLen );
		}
		else return Quaternion();
	}
};


// -------------------------------------------------------------------------------------------------
// Matrix
// -------------------------------------------------------------------------------------------------

class Matrix4f
{
public:
	
	union
	{
		float c[4][4];	// Column major order for OpenGL: c[column][row]
		float x[16];
	};
	
	// --------------
	// Static methods
	// --------------
	static Matrix4f TransMat( float x, float y, float z )
	{
		Matrix4f m;

		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;

		return m;
	}

	static Matrix4f ScaleMat( float x, float y, float z )
	{
		Matrix4f m;
		
		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;

		return m;
	}

	static Matrix4f RotMat( float x, float y, float z )
	{
		// Rotation order: YXZ [* Vector]
		return Matrix4f( Quaternion( x, y, z ) );
	}

	static Matrix4f RotMat( Vec3f axis, float angle )
	{
		const float halfAngle = angle * 0.5f;
		axis = axis * sinf( halfAngle );
		return Matrix4f( Quaternion( axis.x, axis.y, axis.z, cosf( halfAngle ) ) );
	}

	static Matrix4f PerspectiveMat( float l, float r, float b, float t, float n, float f )
	{
		Matrix4f m;

		const float invRsubL = 1.0f / (r - l);
		const float invTsubB = 1.0f / (t - b);
		const float invFsubN = 1.0f / (f - n);

		m.x[0] = 2.0f * n * invRsubL;
		m.x[5] = 2.0f * n * invTsubB;
		m.x[8] = (r + l) * invRsubL;
		m.x[9] = (t + b) * invTsubB;
		m.x[10] = -(f + n) * invFsubN;
		m.x[11] = -1.0f;
		m.x[14] = -2.0f * f * n * invFsubN;
		m.x[15] = 0.0f;

		return m;
	}

	static Matrix4f OrthoMat( float l, float r, float b, float t, float n, float f )
	{
		Matrix4f m;

		const float invRsubL = 1.0f / (r - l);
		const float invTsubB = 1.0f / (t - b);
		const float invFsubN = 1.0f / (f - n);

		m.x[0] = 2.0f * invRsubL;
		m.x[5] = 2.0f * invTsubB;
		m.x[10] = -2.0f * invFsubN;
		m.x[12] = -(r + l) * invRsubL;
		m.x[13] = -(t + b) * invTsubB;;
		m.x[14] = -(f + n) * invFsubN;

		return m;
	}

	static void fastMult43( Matrix4f &dst, const Matrix4f &m1, const Matrix4f &m2 )
	{
		// Note: dst may not be the same as m1 or m2

		float *dstx = dst.x;
		const float *m1x = m1.x;
		const float *m2x = m2.x;
		
		dstx[0] = m1x[0] * m2x[0] + m1x[4] * m2x[1] + m1x[8] * m2x[2];
		dstx[1] = m1x[1] * m2x[0] + m1x[5] * m2x[1] + m1x[9] * m2x[2];
		dstx[2] = m1x[2] * m2x[0] + m1x[6] * m2x[1] + m1x[10] * m2x[2];
		dstx[3] = 0.0f;

		dstx[4] = m1x[0] * m2x[4] + m1x[4] * m2x[5] + m1x[8] * m2x[6];
		dstx[5] = m1x[1] * m2x[4] + m1x[5] * m2x[5] + m1x[9] * m2x[6];
		dstx[6] = m1x[2] * m2x[4] + m1x[6] * m2x[5] + m1x[10] * m2x[6];
		dstx[7] = 0.0f;

		dstx[8] = m1x[0] * m2x[8] + m1x[4] * m2x[9] + m1x[8] * m2x[10];
		dstx[9] = m1x[1] * m2x[8] + m1x[5] * m2x[9] + m1x[9] * m2x[10];
		dstx[10] = m1x[2] * m2x[8] + m1x[6] * m2x[9] + m1x[10] * m2x[10];
		dstx[11] = 0.0f;

		dstx[12] = m1x[0] * m2x[12] + m1x[4] * m2x[13] + m1x[8] * m2x[14] + m1x[12] * m2x[15];
		dstx[13] = m1x[1] * m2x[12] + m1x[5] * m2x[13] + m1x[9] * m2x[14] + m1x[13] * m2x[15];
		dstx[14] = m1x[2] * m2x[12] + m1x[6] * m2x[13] + m1x[10] * m2x[14] + m1x[14] * m2x[15];
		dstx[15] = 1.0f;
	}

	// ------------
	// Constructors
	// ------------
	Matrix4f()
	{
		c[0][0] = 1.0f; c[1][0] = 0.0f; c[2][0] = 0.0f; c[3][0] = 0.0f;
		c[0][1] = 0.0f; c[1][1] = 1.0f; c[2][1] = 0.0f; c[3][1] = 0.0f;
		c[0][2] = 0.0f; c[1][2] = 0.0f; c[2][2] = 1.0f; c[3][2] = 0.0f;
		c[0][3] = 0.0f; c[1][3] = 0.0f; c[2][3] = 0.0f; c[3][3] = 1.0f;
	}

	explicit Matrix4f( Math::NoInitHint )
	{
		// Constructor without default initialization
	}

	Matrix4f( const float *floatArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = floatArray16[i * 4 + j];
			}
		}
	}

	Matrix4f( const Quaternion &q )
	{
		// Calculate coefficients
		float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		float xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
		float yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
		float wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

		c[0][0] = 1.0f - (yy + zz);  c[1][0] = xy - wz;	
		c[2][0] = xz + wy;           c[3][0] = 0.0f;
		c[0][1] = xy + wz;           c[1][1] = 1.0f - (xx + zz);
		c[2][1] = yz - wx;           c[3][1] = 0.0f;
		c[0][2] = xz - wy;           c[1][2] = yz + wx;
		c[2][2] = 1.0f - (xx + yy);  c[3][2] = 0.0f;
		c[0][3] = 0.0f;              c[1][3] = 0.0f;
		c[2][3] = 0.0f;              c[3][3] = 1.0f;
	}

	// ----------
	// Matrix sum
	// ----------
	Matrix4f operator+( const Matrix4f &m ) const 
	{
		Matrix4f mf( Math::NO_INIT );
		
		mf.x[0] = x[0] + m.x[0];
		mf.x[1] = x[1] + m.x[1];
		mf.x[2] = x[2] + m.x[2];
		mf.x[3] = x[3] + m.x[3];
		mf.x[4] = x[4] + m.x[4];
		mf.x[5] = x[5] + m.x[5];
		mf.x[6] = x[6] + m.x[6];
		mf.x[7] = x[7] + m.x[7];
		mf.x[8] = x[8] + m.x[8];
		mf.x[9] = x[9] + m.x[9];
		mf.x[10] = x[10] + m.x[10];
		mf.x[11] = x[11] + m.x[11];
		mf.x[12] = x[12] + m.x[12];
		mf.x[13] = x[13] + m.x[13];
		mf.x[14] = x[14] + m.x[14];
		mf.x[15] = x[15] + m.x[15];

		return mf;
	}

	Matrix4f &operator+=( const Matrix4f &m )
	{
		return *this = *this + m;
	}

	// ---------------------
	// Matrix multiplication
	// ---------------------
	Matrix4f operator*( const Matrix4f &m ) const 
	{
		Matrix4f mf( Math::NO_INIT );
		
		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	Matrix4f operator*( const float f ) const
	{
		Matrix4f m( *this );
		
		m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
		m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
		m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
		m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

		return m;
	}

	// ----------------------------
	// Vector-Matrix multiplication
	// ----------------------------
	Vec3f operator*( const Vec3f &v ) const
	{
		return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
	}

	Vec4f operator*( const Vec4f &v ) const
	{
		return Vec4f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + v.w * c[3][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + v.w * c[3][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + v.w * c[3][2],
		              v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + v.w * c[3][3] );
	}

	Vec3f mult33Vec( const Vec3f &v ) const
	{
		return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
	}
	
	// ---------------
	// Transformations
	// ---------------
	void translate( const float x, const float y, const float z )
	{
		*this = TransMat( x, y, z ) * *this;
	}

	void scale( const float x, const float y, const float z )
	{
		*this = ScaleMat( x, y, z ) * *this;
	}

	void rotate( const float x, const float y, const float z )
	{
		*this = RotMat( x, y, z ) * *this;
	}

	// ---------------
	// Other
	// ---------------

	Matrix4f transposed() const
	{
		Matrix4f m( *this );
		
		for( unsigned int y = 0; y < 4; ++y )
		{
			for( unsigned int x = y + 1; x < 4; ++x ) 
			{
				const float tmp = m.c[x][y];
				m.c[x][y] = m.c[y][x];
				m.c[y][x] = tmp;
			}
		}

		return m;
	}

	float determinant() const
	{
//		return 
//			c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
//			c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
//			c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
//			c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
//			c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
//			c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];

		const float c0312 = c[0][3]*c[1][2];
		const float c0213 = c[0][2]*c[1][3];
		const float c0311 = c[0][3]*c[1][1];
		const float c0113 = c[0][1]*c[1][3];
		const float c0211 = c[0][2]*c[1][1];
		const float c0112 = c[0][1]*c[1][2];
		const float c0310 = c[0][3]*c[1][0];
		const float c0013 = c[0][0]*c[1][3];
		const float c0210 = c[0][2]*c[1][0];
		const float c0012 = c[0][0]*c[1][2];
		const float c0110 = c[0][1]*c[1][0];
		const float c0011 = c[0][0]*c[1][1];

		const float c0312subc0213 = c0312 - c0213;
		const float c0311subc0113 = c0311 - c0113;
		const float c0211subc0112 = c0211 - c0112;
		const float c0310subc0013 = c0310 - c0013;
		const float c0210subc0012 = c0210 - c0012;
		const float c0110subc0011 = c0110 - c0011;

		const float res30 = (c0312subc0213*c[2][1] + c0211subc0112*c[2][3] - c0311subc0113*c[2][2])*c[3][0];
		const float res31 = (c0310subc0013*c[2][2] - c0210subc0012*c[2][3] - c0312subc0213*c[2][0])*c[3][1];
		const float res32 = (c0311subc0113*c[2][0] + c0110subc0011*c[2][3] - c0310subc0013*c[2][1])*c[3][2];
		const float res33 = (c0210subc0012*c[2][1] - c0211subc0112*c[2][0] - c0110subc0011*c[2][2])*c[3][3];

		return (res30 + res31) + (res32 + res33);
	}

	Matrix4f inverted() const
	{
		Matrix4f m( Math::NO_INIT );

		float d = determinant();
		if( fabsf(d) <= Math::ZeroEpsilon ) return m;
		d = 1.0f / d;
		
//		m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
//		m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
//		m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
//		m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
//		m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
//		m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
//		m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
//		m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
//		m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
//		m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
//		m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
//		m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
//		m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
//		m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
//		m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
//		m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);

		const float c1223 = c[1][2]*c[2][3];
		const float c1322 = c[1][3]*c[2][2];
		const float c1321 = c[1][3]*c[2][1];
		const float c1123 = c[1][1]*c[2][3];
		const float c1221 = c[1][2]*c[2][1];
		const float c1122 = c[1][1]*c[2][2];
		const float c2231 = c[2][2]*c[3][1];
		const float c2331 = c[2][3]*c[3][1];
		const float c2132 = c[2][1]*c[3][2];
		const float c2332 = c[2][3]*c[3][2];
		const float c2133 = c[2][1]*c[3][3];
		const float c2233 = c[2][2]*c[3][3];
		const float c0213 = c[0][2]*c[1][3];
		const float c0312 = c[0][3]*c[1][2];
		const float c0311 = c[0][3]*c[1][1];
		const float c0113 = c[0][1]*c[1][3];
		const float c0211 = c[0][2]*c[1][1];
		const float c0112 = c[0][1]*c[1][2];
		const float c2032 = c[2][0]*c[3][2];
		const float c2033 = c[2][0]*c[3][3];
		const float c2330 = c[2][3]*c[3][0];
		const float c2230 = c[2][2]*c[3][0];
		const float c0310 = c[0][3]*c[1][0];
		const float c0013 = c[0][0]*c[1][3];
		const float c0210 = c[0][2]*c[1][0];
		const float c0012 = c[0][0]*c[1][2];
		const float c2031 = c[2][0]*c[3][1];
		const float c2130 = c[2][1]*c[3][0];
		const float c0110 = c[0][1]*c[1][0];
		const float c0011 = c[0][0]*c[1][1];

		const float c1223subc1322 = c1223 - c1322;
		const float c1321subc1123 = c1321 - c1123;
		const float c1122subc1221 = c1122 - c1221;
		const float c2231subc2132 = c2231 - c2132;
		const float c2133subc2331 = c2133 - c2331;
		const float c2332subc2233 = c2332 - c2233;
		const float c0213subc0312 = c0213 - c0312;
		const float c0311subc0113 = c0311 - c0113;
		const float c0112subc0211 = c0112 - c0211;

		m.c[0][0] = d * (c1223subc1322*c[3][1] + c1321subc1123*c[3][2] + c1122subc1221*c[3][3]);
		m.c[0][1] = d * (c2231subc2132*c[0][3] + c2133subc2331*c[0][2] + c2332subc2233*c[0][1]);
		m.c[0][2] = d * (c0213subc0312*c[3][1] + c0311subc0113*c[3][2] + c0112subc0211*c[3][3]);
		m.c[0][3] = d * (c1321subc1123*c[0][2] + c1223subc1322*c[0][1] - c1122subc1221*c[0][3]);
		m.c[1][0] = d * (c2332subc2233*c[1][0] - c1223subc1322*c[3][0] - c[1][3]*c2032 + c[1][2]*c2033);
		m.c[1][1] = d * ((c2330 - c2033)*c[0][2] + (c2032 - c2230)*c[0][3] - c2332subc2233*c[0][0]);
		m.c[1][2] = d * ((c0013 - c0310)*c[3][2] + (c0210 - c0012)*c[3][3] - c0213subc0312*c[3][0]);
		m.c[1][3] = d * (c0213subc0312*c[2][0] + c0310*c[2][2] - c0210*c[2][3] + c1223subc1322*c[0][0]);
		m.c[2][0] = d * (c[1][3]*c2031 - c[1][1]*c2033 + c2133subc2331*c[1][0] - c1321subc1123*c[3][0]);
		m.c[2][1] = d * ((c2130 - c2031)*c[0][3] + (c2033 - c2330)*c[0][1] - c2133subc2331*c[0][0]);
		m.c[2][2] = d * ((c0310 - c0013)*c[3][1] + (c0011 - c0110)*c[3][3] - c0311subc0113*c[3][0]);
		m.c[2][3] = d * (c0311subc0113*c[2][0] - c0310*c[2][1] + c0110*c[2][3] + c1321subc1123*c[0][0]);
		m.c[3][0] = d * (c[1][1]*c2032 - c1122subc1221*c[3][0] - c[1][2]*c2031 + c2231subc2132*c[1][0]);
		m.c[3][1] = d * ((c2230 - c2032)*c[0][1] + (c2031 - c2130)*c[0][2] - c2231subc2132*c[0][0]);
		m.c[3][2] = d * ((c0012 - c0210)*c[3][1] + (c0110 - c0011)*c[3][2] - c0112subc0211*c[3][0]);
		m.c[3][3] = d * (c0112subc0211*c[2][0] + c0210*c[2][1] - c0110*c[2][2] + c1122subc1221*c[0][0]);

		return m;
	}

	void decompose( Vec3f &trans, Vec3f &rot, Vec3f &scale ) const
	{
		// Getting translation is trivial
		trans = Vec3f( c[3][0], c[3][1], c[3][2] );

		// Scale is length of columns
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( fabsf(scale.x) <= Math::ZeroEpsilon || fabsf(scale.y) <= Math::ZeroEpsilon || fabsf(scale.z) <= Math::ZeroEpsilon ) return;

		// Detect negative scale with determinant and flip one arbitrary axis
		if( determinant() < Math::ZeroEpsilon ) scale.x = -scale.x;

		// Combined rotation matrix YXZ
		//
		// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]   Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]  Cos[x]*Sin[y]	
		// Cos[x]*Sin[z]                        Cos[x]*Cos[z]                       -Sin[x]
		// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]  Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]  Cos[x]*Cos[y]

		const float invScaleX = 1.0f / scale.x;
		const float invScaleY = 1.0f / scale.y;
		const float invScaleZ = 1.0f / scale.z;
		
		rot.x = asinf( -c[2][1] * invScaleZ );
		
		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		const float f = fabsf( c[2][1] * invScaleZ );
		if( f > 0.999f && f < 1.001f )
		{
			// Pin arbitrarily one of y or z to zero
			// Mathematical equivalent of gimbal lock
			rot.y = 0.0f;
			
			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
			rot.z = atan2f( -c[1][0] * invScaleY, c[0][0] * invScaleX );
		}
		// Standard case
		else
		{
			rot.y = atan2f( c[2][0] * invScaleZ, c[2][2] * invScaleZ );
			rot.z = atan2f( c[0][1] * invScaleX, c[1][1] * invScaleY );
		}
	}

	
	void setCol( unsigned int col, const Vec4f& v ) 
	{
		x[col * 4 + 0] = v.x;
		x[col * 4 + 1] = v.y;
		x[col * 4 + 2] = v.z;
		x[col * 4 + 3] = v.w;
	}

	Vec4f getCol( unsigned int col ) const
	{
		return Vec4f( x[col * 4 + 0], x[col * 4 + 1], x[col * 4 + 2], x[col * 4 + 3] );
	}

	Vec4f getRow( unsigned int row ) const
	{
		return Vec4f( x[row + 0], x[row + 4], x[row + 8], x[row + 12] );
	}

	Vec3f getTrans() const
	{
		return Vec3f( c[3][0], c[3][1], c[3][2] );
	}
	
	Vec3f getScale() const
	{
		Vec3f scale;
		// Scale is length of columns
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );
		return scale;
	}
};


// -------------------------------------------------------------------------------------------------
// Plane
// -------------------------------------------------------------------------------------------------

class Plane
{
public:
	Vec3f normal; 
	float dist;

	// ------------
	// Constructors
	// ------------
	Plane() 
	{ 
		normal.x = 0.0f; normal.y = 0.0f; normal.z = 0.0f; dist = 0.0f; 
	};

	explicit Plane( const float a, const float b, const float c, const float d )
	{
		normal = Vec3f( a, b, c );
		const float invLen = 1.0f / normal.length();
		normal *= invLen;	// Normalize
		dist = d * invLen;
	}

	Plane( const Vec3f &v0, const Vec3f &v1, const Vec3f &v2 )
	{
		normal = v1 - v0;
		normal = normal.cross( v2 - v0 );
		normal.normalize();
		dist = -normal.dot( v0 );
	}

	// ----------------
	// Other operations
	// ----------------
	float distToPoint( const Vec3f &v ) const
	{
		return normal.dot( v ) + dist;
	}
};


// -------------------------------------------------------------------------------------------------
// Intersection
// -------------------------------------------------------------------------------------------------

inline bool rayTriangleIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, 
                                     const Vec3f &vert0, const Vec3f &vert1, const Vec3f &vert2,
                                     Vec3f &intsPoint )
{
	// Idea: Tomas Moeller and Ben Trumbore
	// in Fast, Minimum Storage Ray/Triangle Intersection 
	
	// Find vectors for two edges sharing vert0
	const Vec3f edge1 = vert1 - vert0;
	const Vec3f edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	const Vec3f pvec = rayDir.cross( edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	const float det = edge1.dot( pvec );


	// *** Culling branch ***
	/*if( det < Math::Epsilon )return false;

	// Calculate distance from vert0 to ray origin
	Vec3f tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	float u = tvec.dot( pvec );
	if (u < 0 || u > det ) return false;

	// Prepare to test V parameter
	Vec3f qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	float v = rayDir.dot( qvec );
	if (v < 0 || u + v > det ) return false;

	// Calculate t, scale parameters, ray intersects triangle
	float t = edge2.dot( qvec ) / det;*/


	// *** Non-culling branch ***
	if( det > -Math::Epsilon && det < Math::Epsilon ) return 0;
	const float inv_det = 1.0f / det;

	// Calculate distance from vert0 to ray origin
	const Vec3f tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	const float u = tvec.dot( pvec ) * inv_det;
	if( u < Math::ZeroEpsilon || u > 1.0f ) return 0;

	// Prepare to test V parameter
	const Vec3f qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	const float v = rayDir.dot( qvec ) * inv_det;
	if( v < Math::ZeroEpsilon || u + v > 1.0f ) return 0;

	// Calculate t, ray intersects triangle
	const float t = edge2.dot( qvec ) * inv_det;


	// Calculate intersection point and test ray length and direction
	intsPoint = rayOrig + rayDir * t;
	const Vec3f vec = intsPoint - rayOrig;
	if( vec.dot( rayDir ) < Math::ZeroEpsilon || vec.length() > rayDir.length() ) return false;

	return true;
}


inline bool rayAABBIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, 
                                 const Vec3f &mins, const Vec3f &maxs )
{
	// SLAB based optimized ray/AABB intersection routine
	// Idea taken from http://ompf.org/ray/
	
	const float invRayDirX = 1.0f / rayDir.x;
	const float invRayDirY = 1.0f / rayDir.y;
	const float invRayDirZ = 1.0f / rayDir.z;

	float l1 = (mins.x - rayOrig.x) * invRayDirX;
	float l2 = (maxs.x - rayOrig.x) * invRayDirX;
	float lmin = minf( l1, l2 );
	float lmax = maxf( l1, l2 );

	l1 = (mins.y - rayOrig.y) * invRayDirY;
	l2 = (maxs.y - rayOrig.y) * invRayDirY;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );
		
	l1 = (mins.z - rayOrig.z) * invRayDirZ;
	l2 = (maxs.z - rayOrig.z) * invRayDirZ;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );

	if( (lmax >= Math::ZeroEpsilon) & (lmax >= lmin) )
	{
		// Consider length
		const Vec3f rayDest = rayOrig + rayDir;
		const Vec3f rayMins( minf( rayDest.x, rayOrig.x), minf( rayDest.y, rayOrig.y ), minf( rayDest.z, rayOrig.z ) );
		const Vec3f rayMaxs( maxf( rayDest.x, rayOrig.x), maxf( rayDest.y, rayOrig.y ), maxf( rayDest.z, rayOrig.z ) );
		return 
			(rayMins.x < maxs.x) && (rayMaxs.x > mins.x) &&
			(rayMins.y < maxs.y) && (rayMaxs.y > mins.y) &&
			(rayMins.z < maxs.z) && (rayMaxs.z > mins.z);
	}
	else
		return false;
}


inline float nearestDistToAABB( const Vec3f &pos, const Vec3f &mins, const Vec3f &maxs )
{
	const Vec3f center = (mins + maxs) * 0.5f;
	const Vec3f extent = (maxs - mins) * 0.5f;
	
	Vec3f nearestVec;
	nearestVec.x = maxf( 0.0f, fabsf( pos.x - center.x ) - extent.x );
	nearestVec.y = maxf( 0.0f, fabsf( pos.y - center.y ) - extent.y );
	nearestVec.z = maxf( 0.0f, fabsf( pos.z - center.z ) - extent.z );
	
	return nearestVec.length();
}

}
#endif // _utMath_H_
