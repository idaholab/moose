//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBroadbridgeWhite.h"
#include "libmesh/utility.h"

namespace PorousFlowBroadbridgeWhite
{
Real
LambertW(Real z)
{
  /* Lambert W function.
     Was ~/C/LambertW.c written K M Briggs Keith dot Briggs at bt dot com 97 May 21.
     Revised KMB 97 Nov 20; 98 Feb 11, Nov 24, Dec 28; 99 Jan 13; 00 Feb 23; 01 Apr 09

     Computes Lambert W function, principal branch.
     See LambertW1.c for -1 branch.

     Returned value W(z) satisfies W(z)*exp(W(z))=z
     test data...
        W(1)= 0.5671432904097838730
        W(2)= 0.8526055020137254914
        W(20)=2.2050032780240599705
     To solve (a+b*R)*exp(-c*R)-d=0 for R, use
     R=-(b*W(-exp(-a*c/b)/b*d*c)+a*c)/b/c

     Test:
       gcc -DTESTW LambertW.c -o LambertW -lm && LambertW
     Library:
       gcc -O3 -c LambertW.c

     Modified trivially by Andy to use MOOSE things
  */
  mooseAssert(z > 0, "LambertW function in RichardsSeff1BWsmall called with negative argument");

  const Real eps = 4.0e-16; //, em1=0.3678794411714423215955237701614608;
  Real p, e, t, w;

  /* Uncomment this stuff is you ever need to call with a negative argument
  if (z < -em1)
    mooseError("LambertW: bad argument ", z, "\n");

  if (0.0 == z)
    return 0.0;
  if (z < -em1+1e-4)
  {
    // series near -em1 in sqrt(q)
    Real q=z+em1,r=std::sqrt(q),q2=q*q,q3=q2*q;
    return
     -1.0
     +2.331643981597124203363536062168*r
     -1.812187885639363490240191647568*q
     +1.936631114492359755363277457668*r*q
     -2.353551201881614516821543561516*q2
     +3.066858901050631912893148922704*r*q2
     -4.175335600258177138854984177460*q3
     +5.858023729874774148815053846119*r*q3
      -8.401032217523977370984161688514*q3*q;  // error approx 1e-16
  }
  */
  /* initial approx for iteration... */
  if (z < 1.0)
  {
    /* series near 0 */
    p = std::sqrt(2.0 * (2.7182818284590452353602874713526625 * z + 1.0));
    w = -1.0 + p * (1.0 + p * (-0.333333333333333333333 + p * 0.152777777777777777777777));
  }
  else
    w = std::log(z); /* asymptotic */
  if (z > 3.0)
    w -= std::log(w); /* useful? */
  for (unsigned int i = 0; i < 10; ++i)
  {
    /* Halley iteration */
    e = std::exp(w);
    t = w * e - z;
    p = w + 1.0;
    t /= e * p - 0.5 * (p + 1.0) * t / p;
    w -= t;
    if (std::abs(t) < eps * (1.0 + std::abs(w)))
      return w; /* rel-abs error */
  }
  /* should never get here */
  mooseError("LambertW: No convergence at z= ", z, "\n");
}

Real
effectiveSaturation(Real pressure, Real c, Real sn, Real ss, Real las)
{
  if (pressure >= 0.0)
    return 1.0;
  const Real x = (c - 1.0) * std::exp(c - 1.0 - c * pressure / las);
  const Real th = c / (1.0 + LambertW(x)); // use branch 0 for positive x
  return sn + (ss - sn) * th;
}

Real
dEffectiveSaturation(Real pressure, Real c, Real sn, Real ss, Real las)
{
  if (pressure >= 0.0)
    return 0.0;
  const Real x = (c - 1.0) * std::exp(c - 1.0 - c * pressure / las);
  const Real lamw = LambertW(x);
  return (ss - sn) * Utility::pow<2>(c) / las * lamw / Utility::pow<3>(1.0 + lamw);
}

Real
d2EffectiveSaturation(Real pressure, Real c, Real sn, Real ss, Real las)
{
  if (pressure >= 0.0)
    return 0.0;
  const Real x = (c - 1.0) * std::exp(c - 1.0 - c * pressure / las);
  const Real lamw = LambertW(x);
  return -(ss - sn) * Utility::pow<3>(c) / Utility::pow<2>(las) * lamw * (1.0 - 2.0 * lamw) /
         Utility::pow<5>(1.0 + lamw);
}

Real
relativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks)
{
  if (s <= sn)
    return kn;

  if (s >= ss)
    return ks;

  const Real coef = (ks - kn) * (c - 1.0);
  const Real th = (s - sn) / (ss - sn);
  const Real krel = kn + coef * Utility::pow<2>(th) / (c - th);
  return krel;
}

Real
dRelativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks)
{
  if (s <= sn)
    return 0.0;

  if (s >= ss)
    return 0.0;

  const Real coef = (ks - kn) * (c - 1.0);
  const Real th = (s - sn) / (ss - sn);
  const Real krelp = coef * (2.0 * th / (c - th) + Utility::pow<2>(th) / Utility::pow<2>(c - th));
  return krelp / (ss - sn);
}

Real
d2RelativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks)
{
  if (s <= sn)
    return 0.0;

  if (s >= ss)
    return 0.0;

  const Real coef = (ks - kn) * (c - 1.0);
  const Real th = (s - sn) / (ss - sn);
  const Real krelpp = coef * (2.0 / (c - th) + 4.0 * th / Utility::pow<2>(c - th) +
                              2.0 * Utility::pow<2>(th) / Utility::pow<3>(c - th));
  return krelpp / Utility::pow<2>(ss - sn);
}
}
