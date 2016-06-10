#include <stdlib.h>
#include <math.h>

#include "track.h"

/**
 * pow2()
 * @param: x
 * @return: x^2
 * @brief: Just an interface for pow in math library. If in future we want to
 * use another optimize formula, we would just have to change the implementation
 * of this function.
 */
double pow2 (const double x);

/**
 * root2()
 * @param: x
 * @return: sqrt(x)
 * @brief: Just an interface for sqrt in math library. If in future we want to
 * use another optimize formula, we would just have to change the implementation
 * of this function.
 */
double root2 (double x);

/**
 * track()
 *
 * See matlab file and report.
 *
 */
int track (const double T1, const double T2, Point* p)
{
  const double e = 4.0; /* distance between the captor and the origin in [cm] */
  const double v = 0.0340; /* Velocity of the signal in [cm/(10^-6)s] */

  /* Check input */
  if (p == NULL || T1 <= 0 || T2 <= 0)
  {
     p->x = 0.0;
     p->y = 0.0;
     p->z = 0.0;
     return 1;
  }

  double d1 = T1 * v;
  double d2 = T2 * v;

  if (d1 == d2)
  {
     p->x = 0.0;
     p->y = (pow2(d1) - pow2(e)) / (2 * d1);

     if (p->y < 0)
     {
        p->x = 0.0;
        p->y = 0.0;
        p->z = 0.0;
        return 2;
     }

     return 0;
  }

  double m = pow2(e) - pow2(d1);
  double n = pow2(e) - pow2(d2);

  double a = (n / pow2(d2)) - (m / pow2(d1));
  double b = (-1.0) * e * ((n / pow2(d2)) + (m / pow2(d1)));
  double c = (pow2(n) / (4.0 * pow2(d2))) - (pow2(m) / (4.0 * pow2(d1)));

  double delta = pow2(b) - 4.0 * a * c;

  if (delta < 0)
  {
     p->x = 0.0;
     p->y = 0.0;
     p->z = 0.0;
     return 3;
  }

  p->x = (-b + root2(delta)) / (2.0 * a);

  double tmp = (((m * pow2(p->x) + m * e * p->x + (pow2(m)) / 4.0)) / pow2(d1));

  if (tmp < 0)
  {
     p->x = 0.0;
     p->y = 0.0;
     p->z = 0.0;
     return 4;
  }

  p->y = root2(tmp);

  if (p->y < 0)
  {
     p->x = 0.0;
     p->y = 0.0;
     p->z = 0.0;
     return 2;
  }

  return 0;
}

double root2 (double x)
{

   return sqrt(x);
}

double pow2 (const double x)
{
   return x * x;
}

