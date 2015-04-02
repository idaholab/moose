#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "Moose.h"
#include "libmesh/libmesh.h"

namespace MathUtils
{
  inline Real round(Real x)
  {
    return ::round(x); // use round from math.h
  }

  inline Real sign(Real x)
  {
    return x >= 0.0 ? 1.0 : -1.0;
  }

  inline Real poly1Log(Real x, Real tol, int deriv)
  {
    Real c1 = 1.0/tol;
    Real c2 = std::log(tol) - 1.0;

    Real value=0.0;

    if (deriv == 0)
    {
      if (x < tol)
        value = c1*x + c2;
      else
        value = std::log(x);
    }
    else if (deriv == 1)
    {
      if (x < tol)
        value = c1;
      else
        value = 1.0/x;
    }
    else if (deriv == 2)
    {
      if (x < tol)
        value = 0.0;
      else
        value = -1.0/(x*x);
    }
    else if (deriv == 3)
    {
      if (x < tol)
        value = 0.0;
      else
        value = 2.0 / (x*x*x);
    }

    return value;
  }

  inline Real poly2Log(Real x, Real tol, int deriv)
  {
    Real c1=-0.5/(tol*tol);
    Real c2=2.0/tol;
    Real c3=std::log(tol) - 3.0/2.0;

    Real value=0.0;

    if (deriv == 0)
    {
      if (x<tol)
        value =  c1*x*x + c2*x + c3;
      else
        value =  std::log(x);
    }
    else if (deriv == 1)
    {
      if (x < tol)
        value = 2.0*c1*x + c2;
      else
        value = 1.0/x;
    }
    else if (deriv == 2)
    {
      if (x < tol)
        value = 2.0 * c1;
      else
        value = -1.0/(x*x);
    }
    else if (deriv == 3)
    {
      if (x < tol)
        value = 0.0;
      else
        value = 2.0 / (x*x*x);
    }

    return value;
  }

  inline Real poly3Log(Real x, Real tol, int order)
  {
    Real c1 = 1.0 / (3.0 * tol*tol*tol);
    Real c2 = -3.0 / (2.0 * tol*tol);
    Real c3 = 3.0 / tol;
    Real c4 = std::log(tol) - 11.0/6.0;

    Real value = 0.0;

    if (order == 0)
    {
      if (x < tol)
        value = c1 * x*x*x + c2 * x*x + c3 * x + c4;
      else
        value = std::log(x);
    }
    else if (order == 1)
    {
      if (x<tol)
        value = 3.0 * c1 * x*x + 2.0 * c2 * x + c3;
      else
        value = 1.0/x;
    }
    else if (order == 2)
    {
      if (x < tol)
        value = 6.0 * c1 * x + 2.0 * c2;
      else
        value = -1.0 / (x*x);
    }
    else if (order == 3)
    {
      if (x < tol)
        value = 6.0*c1;
      else
        value = 2.0 / (x*x*x);
    }
    return value;
  }

  inline Real poly4Log(Real x, Real tol, int order)
  {
    Real value = 0.0;

    if (order == 0)
    {
      if (x < tol)
        value =   std::log(tol)
                + (x-tol) / tol
                - (x-tol) * (x-tol) / (2.0 * tol*tol)
                + (x-tol) * (x-tol) * (x-tol) / (3.0 * tol*tol*tol)
                - (x-tol) * (x-tol) * (x-tol) * (x-tol) / (4.0 * tol*tol*tol*tol)
                + (x-tol) * (x-tol) * (x-tol) * (x-tol) * (x-tol) / (5.0 * tol*tol*tol*tol*tol)
                - (x-tol) * (x-tol) * (x-tol) * (x-tol) * (x-tol) * (x-tol) / (6.0 * tol*tol*tol*tol*tol*tol);
      else
        value = std::log(x);
    }
    else if (order == 1)
    {
      if (x < tol)
        value =   1.0 / tol
                - 2.0 * (x-tol)/(2.0*tol*tol)
                + 3.0 * (x-tol) * (x-tol) / (3.0 * tol*tol*tol)
                - 4.0 * (x-tol) * (x-tol) * (x-tol) / (4.0*tol*tol*tol*tol)
                + 5.0 * (x-tol) * (x-tol) * (x-tol) * (x-tol) / (5.0 * tol*tol*tol*tol*tol)
                - 6.0 * (x-tol) * (x-tol) * (x-tol) * (x-tol) * (x-tol) / (6.0 * tol*tol*tol*tol*tol*tol);
      else
        value = 1.0 / x;
    }
    else if (order == 2)
    {
      if (x < tol)
        value = -2.0 * 1.0 / (2.0*tol*tol)
                + 3.0 * 2.0 * (x-tol) / (3.0 * tol*tol*tol)
                - 4.0 * 3.0 * (x-tol) * (x-tol) / (4.0 * tol*tol*tol*tol)
                + 5.0 * 4.0 * (x-tol) * (x-tol) * (x-tol) / (5.0 * tol*tol*tol*tol*tol)
                - 6.0 * 5.0 * (x-tol) * (x-tol) * (x-tol) * (x-tol) / (6.0 * tol*tol*tol*tol*tol*tol);
      else
        value = -1.0 / (x*x);
    }
    else if (order == 3)
    {
      if (x < tol)
        value =   3.0 * 2.0 * 1.0 / (3.0 * tol*tol*tol)
                - 4.0 * 3.0 * 2.0 * (x-tol) / (4.0 * tol*tol*tol*tol)
                + 5.0 * 4.0 * 3.0 * (x-tol) * (x-tol) / (5.0 * tol*tol*tol*tol*tol)
                - 6.0 * 5.0 * 4.0 * (x-tol) * (x-tol) * (x-tol) / (6.0 * tol*tol*tol*tol*tol*tol);
      else
        value =  2.0 / (x*x*x);
    }
    return value;
  }

  /// \todo This can be done without std::pow!
  inline Real TaylorLog(Real x)
  {
    Real y = (x - 1.0) / (x + 1.0);
    Real val = 1.0;
    for (unsigned int i = 0; i < 5; ++i)
    {
      Real exponent = i + 2.0;
      val += 1.0 / (2.0 * (i + 1.0) + 1.0) * std::pow(y, exponent);
    }

    return val * 2.0 * y;
  }
}

#endif //MATHUTILS_H
