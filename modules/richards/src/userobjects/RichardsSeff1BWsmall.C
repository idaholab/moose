//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "Broadbridge-White" form of effective saturation for Kn small (P Broadbridge and I White
//  ``Constant rate rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water
//  Resources Research 24 (1988) 145-154)
//
#include "RichardsSeff1BWsmall.h"
#include "libmesh/utility.h"

registerMooseObject("RichardsApp", RichardsSeff1BWsmall);

InputParameters
RichardsSeff1BWsmall::validParams()
{
  InputParameters params = RichardsSeff::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "Sn",
      "Sn >= 0",
      "Low saturation.  This must be < Ss, and non-negative.  This is BW's "
      "initial effective saturation, below which effective saturation never goes "
      "in their simulations/models.  If Kn=0 then Sn is the immobile saturation.  "
      "This form of effective saturation is only correct for Kn small.");
  params.addRangeCheckedParam<Real>(
      "Ss",
      1.0,
      "Ss <= 1",
      "High saturation.  This must be > Sn and <= 1.  Effective saturation "
      "where porepressure = 0.  Effective saturation never exceeds this "
      "value in BW's simulations/models.");
  params.addRequiredRangeCheckedParam<Real>(
      "C", "C > 1", "BW's C parameter.  Must be > 1.  Typical value would be 1.05.");
  params.addRequiredRangeCheckedParam<Real>("las",
                                            "las > 0",
                                            "BW's lambda_s parameter multiplied "
                                            "by (fluiddensity*gravity).  Must be "
                                            "> 0.  Typical value would be 1E5");
  params.addClassDescription("Broadbridge-white form of effective saturation for negligable Kn.  "
                             "Then porepressure = -las*( (1-th)/th - (1/c)Ln((C-th)/((C-1)th))), "
                             "for th = (Seff - Sn)/(Ss - Sn).  A Lambert-W function must be "
                             "evaluated to express Seff in terms of porepressure, which can be "
                             "expensive");
  return params;
}

RichardsSeff1BWsmall::RichardsSeff1BWsmall(const InputParameters & parameters)
  : RichardsSeff(parameters),
    _sn(getParam<Real>("Sn")),
    _ss(getParam<Real>("Ss")),
    _c(getParam<Real>("C")),
    _las(getParam<Real>("las"))
{
  if (_ss <= _sn)
    mooseError("In BW effective saturation Sn set to ",
               _sn,
               " and Ss set to ",
               _ss,
               " but these must obey Ss > Sn");
}

Real
RichardsSeff1BWsmall::LambertW(const Real z) const
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

     Modified trially by Andy to use MOOSE things
  */
  mooseAssert(z > 0, "LambertW function in RichardsSeff1BWsmall called with negative argument");

  int i;
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
  for (i = 0; i < 10; i++)
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
RichardsSeff1BWsmall::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real pp = (*p[0])[qp];
  if (pp >= 0)
    return 1.0;

  Real x = (_c - 1.0) * std::exp(_c - 1 - _c * pp / _las);
  Real th = _c / (1.0 + LambertW(x)); // use branch 0 for positive x
  return _sn + (_ss - _sn) * th;
}

void
RichardsSeff1BWsmall::dseff(std::vector<const VariableValue *> p,
                            unsigned int qp,
                            std::vector<Real> & result) const
{
  result[0] = 0.0;

  Real pp = (*p[0])[qp];
  if (pp >= 0)
    return;

  Real x = (_c - 1) * std::exp(_c - 1.0 - _c * pp / _las);
  Real lamw = LambertW(x);
  result[0] = Utility::pow<2>(_c) / _las * lamw / Utility::pow<3>(1 + lamw);
}

void
RichardsSeff1BWsmall::d2seff(std::vector<const VariableValue *> p,
                             unsigned int qp,
                             std::vector<std::vector<Real>> & result) const
{
  result[0][0] = 0.0;

  Real pp = (*p[0])[qp];
  if (pp >= 0)
    return;

  Real x = (_c - 1) * std::exp(_c - 1 - _c * pp / _las);
  Real lamw = LambertW(x);
  result[0][0] = -Utility::pow<3>(_c) / Utility::pow<2>(_las) * lamw * (1.0 - 2.0 * lamw) /
                 Utility::pow<5>(1 + lamw);
}
