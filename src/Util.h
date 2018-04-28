/*
 *  Util.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  All the routines I can't think of a better place for
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <math.h>

#include <dm.h>

#define SQUARE(a) ((a) * (a))
#define THROWIFZERO(a) if ((a) == 0) throw 0
#define THROWIF(a) if ((a) != 0) throw 0
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#define ODD(n) ((n) & 1)

class Util {
public:

    // calculate cross product (vector product)
    inline static void CrossProduct3x1(const double *a, const double *b, double *c)
    {
        c[0] = a[1] * b[2] - a[2] * b[1];
        c[1] = a[2] * b[0] - a[0] * b[2];
        c[2] = a[0] * b[1] - a[1] * b[0];
    };

    // calculate dot product (scalar product)
    inline static double DotProduct3x1(const double *a, const double *b)
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    };

    // calculate length of vector
    inline static double Magnitude3x1(const double *a)
    {
        return sqrt(SQUARE(a[0]) + SQUARE(a[1]) + SQUARE(a[2]));
    };

    // calculate distance between two points
    inline static double Distance3x1(const double *a, const double *b)
    {
        return sqrt(SQUARE(a[0] - b[0]) + SQUARE(a[1] - b[1]) + SQUARE(a[2] - b[2]));
    };

    // calculate unit vector
    inline static void Unit3x1(double *a)
    {
        double len = sqrt(SQUARE(a[0]) + SQUARE(a[1]) + SQUARE(a[2]));
        // default fixup for zero length vectors
        if (ABS(len) < 1e-30)
        {
            a[0] = 1;
            a[1] = 0;
            a[2] = 0;
        }
        else
        {
            a[0] /= len;
            a[1] /= len;
            a[2] /= len;
        }
    };

    // c = a + b vectors
    inline static void Add3x1(const double *a, const double *b, double *c)
    {
        c[0] = a[0] + b[0];
        c[1] = a[1] + b[1];
        c[2] = a[2] + b[2];
    };

    // c = a - b vectors
    inline static void Subtract3x1(const double *a, const double *b, double *c)
    {
        c[0] = a[0] - b[0];
        c[1] = a[1] - b[1];
        c[2] = a[2] - b[2];
    };

    // c = scalar * a
    inline static void ScalarMultiply3x1(const double scalar, const double *a, double *c)
    {
        c[0] = a[0] * scalar;
        c[1] = a[1] * scalar;
        c[2] = a[2] * scalar;
    };

    // b = a
    inline static void Copy3x1(const double *a, double *b)
    {
        b[0] = a[0];
        b[1] = a[1];
        b[2] = a[2];
    };


    inline static void MDHTransform(double a, double alpha, double d, double theta,
                                    double *location)
    {
        double internal[3];

        // translate back to origin

        internal[0] = location[0] - a;
        internal[1] = location[1];
        internal[2] = location[2] - d;

        // rotation code from dmMDHLink::rtxToInboard

        double calpha = cos(alpha);
        double salpha = sin(alpha);
        double ctheta = cos(theta);
        double stheta = sin(theta);
        double temp1;

        // z planar rotation:
        location[0] = internal[0]*ctheta - internal[1]*stheta;
        temp1 = internal[1]*ctheta + internal[0]*stheta;

        // x planar rotation
        location[1] = temp1*calpha - internal[2]*salpha;
        location[2] = internal[2]*calpha + temp1*salpha;

        // translate back to original position

        location[0] += a;
        location[2] += d;
    };

    inline static void ZPlaneRotate(double theta,
                                    double *location)
    {
        double internal[3];

        // get a local copy

        internal[0] = location[0];
        internal[1] = location[1];
        // internal[2] = location[2];
        
        // rotation code

        double ctheta = cos(theta);
        double stheta = sin(theta);

        // z planar rotation:
        location[0] = internal[0]*ctheta - internal[1]*stheta;
        location[1] = internal[1]*ctheta + internal[0]*stheta;
    };

    inline static bool OutRange(double v, double l, double h)
    {
        if (v < l)
            return true;
        if (v > h)
            return true;
        return false;
    }

    // convert a link local coordinate to a world coordinate

    inline static void
        ConvertLocalToWorldP(const dmABForKinStruct *m, const CartesianVector local, CartesianVector world)
    {
        for (int j = 0; j < 3; j++)
        {

            world[j] = m->p_ICS[j] +
            m->R_ICS[j][0] * local[0] +
            m->R_ICS[j][1] * local[1] +
            m->R_ICS[j][2] * local[2];
        }
    }

    // convert a link local coordinate to a world coordinate
    // don't know if this is actually corect for the rotations (are they
    // global or local?)

    inline static void
        ConvertLocalToWorldV(const dmABForKinStruct *m, const SpatialVector local, SpatialVector world)
    {
        for (int j = 0; j < 3; j++)
        {
            world[j] =
            m->R_ICS[j][0] * local[0] +
            m->R_ICS[j][1] * local[1] +
            m->R_ICS[j][2] * local[2];

            world[j + 3] =
                m->R_ICS[j][0] * local[0 + 3] +
                m->R_ICS[j][1] * local[1 + 3] +
                m->R_ICS[j][2] * local[2 + 3];
        }
    }

    inline static double Double(char *buf)
    {
        return strtod(buf, 0);
    }

    inline static double Double(unsigned char *buf)
    {
        return strtod((char *)buf, 0);
    }

    inline static void Double(char *buf, int n, double *d)
    {
        char *ptr = buf;
        for (int i = 0; i < n; i++)
            d[i] = strtod(ptr, &ptr);
    }

    inline static void Double(unsigned char *buf, int n, double *d)
    {
        char *ptr = (char *)buf;
        for (int i = 0; i < n; i++)
            d[i] = strtod(ptr, &ptr);
    }

    inline static int Int(char *buf)
    {
        return (int)strtol(buf, 0, 10);
    }

    inline static int Int(unsigned char *buf)
    {
        return (int)strtol((char *)buf, 0, 10);
    }

    inline static void Int(char *buf, int n, int *d)
    {
        char *ptr = buf;
        for (int i = 0; i < n; i++)
            d[i] = (int)strtol(ptr, &ptr, 10);
    }
    
    inline static void Int(unsigned char *buf, int n, int *d)
    {
        char *ptr = (char *)buf;
        for (int i = 0; i < n; i++)
            d[i] = (int)strtol(ptr, &ptr, 10);
    }

    // Note modifies string
    inline static bool Bool(char *buf)
    {
        Strip(buf);
        if (strcasecmp(buf, "false") == 0) return false;
        if (strcasecmp(buf, "true") == 0) return true;
        if (strtol(buf, 0, 10) != 0) return true;
        return false;
    }

    // Note modifies string
    inline static bool Bool(unsigned char *buf)
    {
        Strip((char *)buf);
        if (strcasecmp((char *)buf, "false") == 0) return false;
        if (strcasecmp((char *)buf, "true") == 0) return true;
        if (strtol((char *)buf, 0, 10) != 0) return true;
        return false;
    }

    // strip out beginning and ending whitespace
    // Note modifies string
    inline static void Strip(char *str)
    {
        char *p1, *p2;

        if (*str == 0) return;

        // heading whitespace
        if (*str <= ' ')
        {
            p1 = str;
            while (*p1)
            {
                if (*p1 > ' ') break;
                p1++;
            }
            p2 = str;
            while (*p1)
            {
                *p2 = *p1;
                p1++;
                p2++;
            }
            *p2 = 0;
        }

        if (*str == 0) return;

        // tailing whitespace
        p1 = str;
        while (*p1)
        {
            p1++;
        }
        p1--;
        while (*p1 <= ' ')
        {
            p1--;
        }
        p1++;
        *p1 = 0;

        return;
    }
    
    // return the index of a matching item in a sorted array
    template <class T> inline static int BinarySearchMatch
        (T array[ ], int listlen, T item)
    {
            int first = 0;
            int last = listlen-1;
            int mid;
            while (first <= last)
            {
                mid = (first + last) / 2;
                if (array[mid] < item) first = mid + 1;
                else if (array[mid] > item) last = mid - 1;
                else return mid;
            }

            return -1;
    }

    // return the index of a matching item in a sorted array
    // special case when I'm searching for a range rather than an exact match
    // returns the index of array[index] <= item < array[index+1]
    // UNTESTED!!!!
    template <class T> inline static int BinarySearchRange
        (T array[ ], int listlen, T item)
    {
            int first = 0;
            int last = listlen-1;
            int mid;
            while (first <= last)
            {
                mid = (first + last) / 2;
                if (array[mid + 1] <= item) first = mid + 1;
                else if (array[mid] > item) last = mid - 1;
                else return mid;
            }
            return -1;
    }

    // calculate the tangent intercept of a line from point (x, y) to a circle
    // of radius r and centred on the origin. The anticlockwise return value is the
    // angle of the radius pointing to the intercept when the vector from the point to
    // the intercept acts in an anticlockwise direction.
    // returns zero on success, non-zero on failure.
    static int PointToCircleTangentIntercept(double x, double y, double r,
                                      double *anticlockwise, double *clockwise);

    
#ifdef USE_OPENGL
    static void DrawAxes();
    static void DrawCOM(double x, double y, double z);
#endif

};



#endif                   // __UTIL_H__
