// Handy geometry classes from Physics for Game Developers
// Altered to produce a double precision version

#ifndef _MYMATH
#define _MYMATH

#include <math.h>

#define MYMATH_ABS(a) ((a) >= 0 ? (a) : -(a))

// wis  - namespace to avoid naming problems
namespace pgd
{

//------------------------------------------------------------------------//
// Misc. Constants
//------------------------------------------------------------------------//

double	const	pi	= M_PI;
double	const	tol	= 0.0000001;		// double type tolerance 


//------------------------------------------------------------------------//
// Misc. Functions
//------------------------------------------------------------------------//
inline	double	DegreesToRadians(double deg);
inline	double	RadiansToDegrees(double rad);

inline	double	DegreesToRadians(double deg)
{
	return deg * pi / 180.0;
}

inline	double	RadiansToDegrees(double rad)
{	
	return rad * 180.0 / pi;
}

//------------------------------------------------------------------------//
// Vector Class and vector functions
//------------------------------------------------------------------------//
class Vector {
public:
	double x;
	double y;
	double z;

	Vector(void);
	Vector(double xi, double yi, double zi);

	double Magnitude(void);
	void  Normalize(void);
	void  Reverse(void);

	Vector& operator+=(Vector u);	// vector addition
	Vector& operator-=(Vector u);	// vector subtraction
	Vector& operator*=(double s);	// scalar multiply
	Vector& operator/=(double s);	// scalar divide

	Vector operator-(void);

};

inline	Vector operator+(Vector u, Vector v);
inline	Vector operator-(Vector u, Vector v);
inline	Vector operator^(Vector u, Vector v);
inline	double operator*(Vector u, Vector v);
inline	Vector operator*(double s, Vector u);
inline	Vector operator*(Vector u, double s);
inline	Vector operator/(Vector u, double s);
inline	double TripleScalarProduct(Vector u, Vector v, Vector w);

inline Vector::Vector(void)
{
	x = 0;
	y = 0;
	z = 0;
}

inline Vector::Vector(double xi, double yi, double zi)
{
	x = xi;
	y = yi;
	z = zi;
}

inline	double Vector::Magnitude(void)
{
	return (double) sqrt(x*x + y*y + z*z);
}

inline	void  Vector::Normalize(void)
{
	double m = (double) sqrt(x*x + y*y + z*z);
	if(m <= tol) m = 1;
	x /= m;
	y /= m;
	z /= m;	

	if (MYMATH_ABS(x) < tol) x = 0.0;
	if (MYMATH_ABS(y) < tol) y = 0.0;
	if (MYMATH_ABS(z) < tol) z = 0.0;
}

inline	void  Vector::Reverse(void)
{
	x = -x;
	y = -y;
	z = -z;
}

inline Vector& Vector::operator+=(Vector u)
{
	x += u.x;
	y += u.y;
	z += u.z;
	return *this;
}

inline	Vector& Vector::operator-=(Vector u)
{
	x -= u.x;
	y -= u.y;
	z -= u.z;
	return *this;
}

inline	Vector& Vector::operator*=(double s)
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

inline	Vector& Vector::operator/=(double s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

inline	Vector Vector::operator-(void)
{
	return Vector(-x, -y, -z);
}


inline	Vector operator+(Vector u, Vector v)
{
	return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline	Vector operator-(Vector u, Vector v)
{
	return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
}

// Vector cross product (u cross v)
inline	Vector operator^(Vector u, Vector v)
{
	return Vector(	u.y*v.z - u.z*v.y,
					-u.x*v.z + u.z*v.x,
					u.x*v.y - u.y*v.x );
}

// Vector dot product
inline	double operator*(Vector u, Vector v)
{
	return (u.x*v.x + u.y*v.y + u.z*v.z);
}

inline	Vector operator*(double s, Vector u)
{
	return Vector(u.x*s, u.y*s, u.z*s);
}

inline	Vector operator*(Vector u, double s)
{
	return Vector(u.x*s, u.y*s, u.z*s);
}

inline	Vector operator/(Vector u, double s)
{
	return Vector(u.x/s, u.y/s, u.z/s);
}

// triple scalar product (u dot (v cross w))
inline	double TripleScalarProduct(Vector u, Vector v, Vector w)
{
	return double(	(u.x * (v.y*w.z - v.z*w.y)) +
					(u.y * (-v.x*w.z + v.z*w.x)) +
					(u.z * (v.x*w.y - v.y*w.x)) );
	//return u*(v^w);

}



//------------------------------------------------------------------------//
// Matrix Class and matrix functions
//------------------------------------------------------------------------//

class Matrix3x3 {
public:
	// elements eij: i -> row, j -> column
	double	e11, e12, e13, e21, e22, e23, e31, e32, e33;	

	Matrix3x3(void);
	Matrix3x3(	double r1c1, double r1c2, double r1c3, 
				double r2c1, double r2c2, double r2c3, 
				double r3c1, double r3c2, double r3c3 );

	double	det(void);
	Matrix3x3	Transpose(void);
	Matrix3x3	Inverse(void);

	Matrix3x3& operator+=(Matrix3x3 m);
	Matrix3x3& operator-=(Matrix3x3 m);
	Matrix3x3& operator*=(double s);
	Matrix3x3& operator/=(double s);
};

inline	Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2);
inline	Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2);
inline	Matrix3x3 operator/(Matrix3x3 m, double s);
inline	Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2);
inline	Matrix3x3 operator*(Matrix3x3 m, double s);
inline	Matrix3x3 operator*(double s, Matrix3x3 m);
inline	Vector operator*(Matrix3x3 m, Vector u);
inline	Vector operator*(Vector u, Matrix3x3 m);





inline	Matrix3x3::Matrix3x3(void)
{
	e11 = 0;
	e12 = 0;
	e13 = 0;
	e21 = 0;
	e22 = 0;
	e23 = 0;
	e31 = 0;
	e32 = 0;
	e33 = 0;
}

inline	Matrix3x3::Matrix3x3(	double r1c1, double r1c2, double r1c3, 
								double r2c1, double r2c2, double r2c3, 
								double r3c1, double r3c2, double r3c3 )
{
	e11 = r1c1;
	e12 = r1c2;
	e13 = r1c3;
	e21 = r2c1;
	e22 = r2c2;
	e23 = r2c3;
	e31 = r3c1;
	e32 = r3c2;
	e33 = r3c3;
}

inline	double	Matrix3x3::det(void)
{
	return	e11*e22*e33 - 
			e11*e32*e23 + 
			e21*e32*e13 - 
			e21*e12*e33 + 
			e31*e12*e23 - 
			e31*e22*e13;	
}

inline	Matrix3x3	Matrix3x3::Transpose(void)
{
	return Matrix3x3(e11,e21,e31,e12,e22,e32,e13,e23,e33);
}

inline	Matrix3x3	Matrix3x3::Inverse(void)
{
	double	d = e11*e22*e33 - 
				e11*e32*e23 + 
				e21*e32*e13 - 
				e21*e12*e33 + 
				e31*e12*e23 - 
				e31*e22*e13;

	if (d == 0) d = 1;

	return	Matrix3x3(	(e22*e33-e23*e32)/d,
						-(e12*e33-e13*e32)/d,
						(e12*e23-e13*e22)/d,
						-(e21*e33-e23*e31)/d,
						(e11*e33-e13*e31)/d,
						-(e11*e23-e13*e21)/d,
						(e21*e32-e22*e31)/d,
						-(e11*e32-e12*e31)/d,
						(e11*e22-e12*e21)/d );	
}

inline	Matrix3x3& Matrix3x3::operator+=(Matrix3x3 m)
{
	e11 += m.e11;
	e12 += m.e12;
	e13 += m.e13;
	e21 += m.e21;
	e22 += m.e22;
	e23 += m.e23;
	e31 += m.e31;
	e32 += m.e32;
	e33 += m.e33;
	return *this;
}

inline	Matrix3x3& Matrix3x3::operator-=(Matrix3x3 m)
{
	e11 -= m.e11;
	e12 -= m.e12;
	e13 -= m.e13;
	e21 -= m.e21;
	e22 -= m.e22;
	e23 -= m.e23;
	e31 -= m.e31;
	e32 -= m.e32;
	e33 -= m.e33;
	return *this;
}

inline	Matrix3x3& Matrix3x3::operator*=(double s)
{
	e11 *= s;
	e12 *= s;
	e13 *= s;
	e21 *= s;
	e22 *= s;
	e23 *= s;
	e31 *= s;
	e32 *= s;
	e33 *= s;
	return *this;
}

inline	Matrix3x3& Matrix3x3::operator/=(double s)
{
	e11 /= s;
	e12 /= s;
	e13 /= s;
	e21 /= s;
	e22 /= s;
	e23 /= s;
	e31 /= s;
	e32 /= s;
	e33 /= s;
	return *this;
}

inline	Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2)
{
	return	Matrix3x3(	m1.e11+m2.e11,
						m1.e12+m2.e12,
						m1.e13+m2.e13,
						m1.e21+m2.e21,
						m1.e22+m2.e22,
						m1.e23+m2.e23,
						m1.e31+m2.e31,
						m1.e32+m2.e32,
						m1.e33+m2.e33);
}

inline	Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2)
{
	return	Matrix3x3(	m1.e11-m2.e11,
						m1.e12-m2.e12,
						m1.e13-m2.e13,
						m1.e21-m2.e21,
						m1.e22-m2.e22,
						m1.e23-m2.e23,
						m1.e31-m2.e31,
						m1.e32-m2.e32,
						m1.e33-m2.e33);
}

inline	Matrix3x3 operator/(Matrix3x3 m, double s)
{	
	return	Matrix3x3(	m.e11/s,
						m.e12/s,
						m.e13/s,
						m.e21/s,
						m.e22/s,
						m.e23/s,
						m.e31/s,
						m.e32/s,
						m.e33/s);
}

inline	Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2)
{
	return Matrix3x3(	m1.e11*m2.e11 + m1.e12*m2.e21 + m1.e13*m2.e31,
						m1.e11*m2.e12 + m1.e12*m2.e22 + m1.e13*m2.e32,
						m1.e11*m2.e13 + m1.e12*m2.e23 + m1.e13*m2.e33,
						m1.e21*m2.e11 + m1.e22*m2.e21 + m1.e23*m2.e31,
						m1.e21*m2.e12 + m1.e22*m2.e22 + m1.e23*m2.e32,
						m1.e21*m2.e13 + m1.e22*m2.e23 + m1.e23*m2.e33,
						m1.e31*m2.e11 + m1.e32*m2.e21 + m1.e33*m2.e31,
						m1.e31*m2.e12 + m1.e32*m2.e22 + m1.e33*m2.e32,
						m1.e31*m2.e13 + m1.e32*m2.e23 + m1.e33*m2.e33 );
}

inline	Matrix3x3 operator*(Matrix3x3 m, double s)
{
	return	Matrix3x3(	m.e11*s,
						m.e12*s,
						m.e13*s,
						m.e21*s,
						m.e22*s,
						m.e23*s,
						m.e31*s,
						m.e32*s,
						m.e33*s);
}

inline	Matrix3x3 operator*(double s, Matrix3x3 m)
{
	return	Matrix3x3(	m.e11*s,
						m.e12*s,
						m.e13*s,
						m.e21*s,
						m.e22*s,
						m.e23*s,
						m.e31*s,
						m.e32*s,
						m.e33*s);
}

inline	Vector operator*(Matrix3x3 m, Vector u)
{
	return Vector(	m.e11*u.x + m.e12*u.y + m.e13*u.z,
					m.e21*u.x + m.e22*u.y + m.e23*u.z,
					m.e31*u.x + m.e32*u.y + m.e33*u.z);					
}

inline	Vector operator*(Vector u, Matrix3x3 m)
{
	return Vector(	u.x*m.e11 + u.y*m.e21 + u.z*m.e31,
					u.x*m.e12 + u.y*m.e22 + u.z*m.e32,
					u.x*m.e13 + u.y*m.e23 + u.z*m.e33);
}

//------------------------------------------------------------------------//
// Quaternion Class and Quaternion functions
//------------------------------------------------------------------------//

class Quaternion {
public:
	double	n;	// number (scalar) part
	Vector	v;	// vector part: v.x, v.y, v.z

	Quaternion(void);
	Quaternion(double e0, double e1, double e2, double e3);

	double	Magnitude(void);
	Vector	GetVector(void);
	double	GetScalar(void);
	Quaternion	operator+=(Quaternion q);
	Quaternion	operator-=(Quaternion q);
	Quaternion operator*=(double s);
	Quaternion operator/=(double s);
	Quaternion	operator~(void) const { return Quaternion(n, -v.x, -v.y, -v.z);}
};

inline	Quaternion operator+(Quaternion q1, Quaternion q2);
inline	Quaternion operator-(Quaternion q1, Quaternion q2);
inline	Quaternion operator*(Quaternion q1, Quaternion q2);
inline	Quaternion operator*(Quaternion q, double s);
inline	Quaternion operator*(double s, Quaternion q);
inline	Quaternion operator*(Quaternion q, Vector v);
inline	Quaternion operator*(Vector v, Quaternion q);
inline	Quaternion operator/(Quaternion q, double s);
inline	double QGetAngle(Quaternion q);
inline	Vector QGetAxis(Quaternion q);
inline	Quaternion QRotate(Quaternion q1, Quaternion q2);
inline	Vector	QVRotate(Quaternion q, Vector v);
inline	Quaternion	MakeQFromEulerAngles(double x, double y, double z);
inline	Vector	MakeEulerAnglesFromQ(Quaternion q);
inline	Quaternion	MakeQFromAxis(double x, double y, double z, double angle);

inline	Quaternion::Quaternion(void)
{
	n = 0;
	v.x = 0;
	v.y =  0;
	v.z = 0;
}

inline	Quaternion::Quaternion(double e0, double e1, double e2, double e3)
{
	n = e0;
	v.x = e1;
	v.y = e2;
	v.z = e3;
}

inline	double	Quaternion::Magnitude(void)
{
	return (double) sqrt(n*n + v.x*v.x + v.y*v.y + v.z*v.z);
}

inline	Vector	Quaternion::GetVector(void)
{
	return Vector(v.x, v.y, v.z);
}

inline	double	Quaternion::GetScalar(void)
{
	return n;
}

inline	Quaternion	Quaternion::operator+=(Quaternion q)
{
	n += q.n;
	v.x += q.v.x;
	v.y += q.v.y;
	v.z += q.v.z;
	return *this;
}

inline	Quaternion	Quaternion::operator-=(Quaternion q)
{
	n -= q.n;
	v.x -= q.v.x;
	v.y -= q.v.y;
	v.z -= q.v.z;
	return *this;
}

inline	Quaternion Quaternion::operator*=(double s)
{
	n *= s;
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return *this;
}

inline	Quaternion Quaternion::operator/=(double s)
{
	n /= s;
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return *this;
}

/*inline	Quaternion	Quaternion::operator~()
{
	return Quaternion(n, -v.x, -v.y, -v.z);
}*/

inline	Quaternion operator+(Quaternion q1, Quaternion q2)
{
	return	Quaternion(	q1.n + q2.n,
							q1.v.x + q2.v.x,
							q1.v.y + q2.v.y,
							q1.v.z + q2.v.z);
}

inline	Quaternion operator-(Quaternion q1, Quaternion q2)
{
	return	Quaternion(	q1.n - q2.n,
							q1.v.x - q2.v.x,
							q1.v.y - q2.v.y,
							q1.v.z - q2.v.z);
}

inline	Quaternion operator*(Quaternion q1, Quaternion q2)
{
	return	Quaternion(	q1.n*q2.n - q1.v.x*q2.v.x - q1.v.y*q2.v.y - q1.v.z*q2.v.z,
							q1.n*q2.v.x + q1.v.x*q2.n + q1.v.y*q2.v.z - q1.v.z*q2.v.y,
							q1.n*q2.v.y + q1.v.y*q2.n + q1.v.z*q2.v.x - q1.v.x*q2.v.z,
							q1.n*q2.v.z + q1.v.z*q2.n + q1.v.x*q2.v.y - q1.v.y*q2.v.x);							
}

inline	Quaternion operator*(Quaternion q, double s)
{
	return	Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
}

inline	Quaternion operator*(double s, Quaternion q)
{
	return	Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
}

inline	Quaternion operator*(Quaternion q, Vector v)
{
	return	Quaternion(	-(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
							q.n*v.x + q.v.y*v.z - q.v.z*v.y,
							q.n*v.y + q.v.z*v.x - q.v.x*v.z,
							q.n*v.z + q.v.x*v.y - q.v.y*v.x);
}

inline	Quaternion operator*(Vector v, Quaternion q)
{
	return	Quaternion(	-(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
							q.n*v.x + q.v.z*v.y - q.v.y*v.z,
							q.n*v.y + q.v.x*v.z - q.v.z*v.x,
							q.n*v.z + q.v.y*v.x - q.v.x*v.y);
}

inline	Quaternion operator/(Quaternion q, double s)
{
	return	Quaternion(q.n/s, q.v.x/s, q.v.y/s, q.v.z/s);
}

inline	double QGetAngle(Quaternion q)
{
	return	(double) (2*acos(q.n));
}

inline	Vector QGetAxis(Quaternion q)
{
	Vector v;
	double m;

	v = q.GetVector();
	m = v.Magnitude();
	
	if (m <= tol)
		return Vector();
	else
		return v/m;	
}

inline	Quaternion QRotate(Quaternion q1, Quaternion q2)
{
	return	q1*q2*(~q1);
}

inline	Vector	QVRotate(Quaternion q, Vector v)
{
	Quaternion t;


	t = q*v*(~q);

	return	t.GetVector();
}

inline	Quaternion	MakeQFromEulerAngles(double x, double y, double z)
{
	Quaternion	q;
	double	roll = DegreesToRadians(x);
	double	pitch = DegreesToRadians(y);
	double	yaw = DegreesToRadians(z);
	
	double	cyaw, cpitch, croll, syaw, spitch, sroll;
	double	cyawcpitch, syawspitch, cyawspitch, syawcpitch;

	cyaw = cos(0.5 * yaw);
	cpitch = cos(0.5 * pitch);
	croll = cos(0.5 * roll);
	syaw = sin(0.5 * yaw);
	spitch = sin(0.5 * pitch);
	sroll = sin(0.5 * roll);

	cyawcpitch = cyaw*cpitch;
	syawspitch = syaw*spitch;
	cyawspitch = cyaw*spitch;
	syawcpitch = syaw*cpitch;

	q.n = (double) (cyawcpitch * croll + syawspitch * sroll);
	q.v.x = (double) (cyawcpitch * sroll - syawspitch * croll); 
	q.v.y = (double) (cyawspitch * croll + syawcpitch * sroll);
	q.v.z = (double) (syawcpitch * croll - cyawspitch * sroll);

	return q;
}

inline	Vector	MakeEulerAnglesFromQ(Quaternion q)
{
	double	r11, r21, r31, r32, r33, r12, r13;
	double	q00, q11, q22, q33;
	double	tmp;
	Vector	u;

	q00 = q.n * q.n;
	q11 = q.v.x * q.v.x;
	q22 = q.v.y * q.v.y;
	q33 = q.v.z * q.v.z;

	r11 = q00 + q11 - q22 - q33;
	r21 = 2 * (q.v.x*q.v.y + q.n*q.v.z);
	r31 = 2 * (q.v.x*q.v.z - q.n*q.v.y);
	r32 = 2 * (q.v.y*q.v.z + q.n*q.v.x);
	r33 = q00 - q11 - q22 + q33;

	tmp = MYMATH_ABS(r31);
	if(tmp > 0.999999999999)
	{
		r12 = 2 * (q.v.x*q.v.y - q.n*q.v.z);
		r13 = 2 * (q.v.x*q.v.z + q.n*q.v.y);

		u.x = RadiansToDegrees(0.0); //roll
		u.y = RadiansToDegrees((double) (-(pi/2) * r31/tmp)); // pitch
		u.z = RadiansToDegrees((double) atan2(-r12, -r31*r13)); // yaw
		return u;
	}

	u.x = RadiansToDegrees((double) atan2(r32, r33)); // roll
	u.y = RadiansToDegrees((double) asin(-r31));		 // pitch
	u.z = RadiansToDegrees((double) atan2(r21, r11)); // yaw
	return u;
	

}

// wis  - new routine to make a Quaternion from an axis and a rotation angle in radians
inline	Quaternion	MakeQFromAxis(double x, double y, double z, double angle)
{
    Quaternion	q;
    
    Vector v(x, y, z);
    v.Normalize();

    double sin_a = sin( angle / 2 );
    double cos_a = cos( angle / 2 );

    q.v.x    = v.x * sin_a;
    q.v.y    = v.y * sin_a;
    q.v.z    = v.z * sin_a;
    q.n    = cos_a;

    return q;
}

}



#endif
