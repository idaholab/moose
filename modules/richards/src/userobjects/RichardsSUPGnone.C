//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  User object to implement no SUPG
//
#include "RichardsSUPGnone.h"

registerMooseObject("RichardsApp", RichardsSUPGnone);

InputParameters
RichardsSUPGnone::validParams()
{
  InputParameters params = RichardsSUPG::validParams();
  params.addClassDescription("User object for no SUPG");
  return params;
}

RichardsSUPGnone::RichardsSUPGnone(const InputParameters & parameters) : RichardsSUPG(parameters) {}

RealVectorValue RichardsSUPGnone::velSUPG(RealTensorValue /*perm*/,
                                          RealVectorValue /*gradp*/,
                                          Real /*density*/,
                                          RealVectorValue /*gravity*/) const
{
  return RealVectorValue();
}

RealTensorValue RichardsSUPGnone::dvelSUPG_dgradp(RealTensorValue /*perm*/) const
{
  return RealTensorValue();
}

RealVectorValue RichardsSUPGnone::dvelSUPG_dp(RealTensorValue /*perm*/,
                                              Real /*density_prime*/,
                                              RealVectorValue /*gravity*/) const
{
  return RealVectorValue();
}

RealVectorValue
RichardsSUPGnone::bb(RealVectorValue /*vel*/,
                     int /*dimen*/,
                     RealVectorValue /*xi_prime*/,
                     RealVectorValue /*eta_prime*/,
                     RealVectorValue /*zeta_prime*/) const
{
  return RealVectorValue();
}

// following is d(bb*bb)/d(gradp)
RealVectorValue RichardsSUPGnone::dbb2_dgradp(RealVectorValue /*vel*/,
                                              RealTensorValue /*dvel_dgradp*/,
                                              RealVectorValue /*xi_prime*/,
                                              RealVectorValue /*eta_prime*/,
                                              RealVectorValue /*zeta_prime*/) const
{
  return RealVectorValue();
}

// following is d(bb*bb)/d(p)
Real RichardsSUPGnone::dbb2_dp(RealVectorValue /*vel*/,
                               RealVectorValue /*dvel_dp*/,
                               RealVectorValue /*xi_prime*/,
                               RealVectorValue /*eta_prime*/,
                               RealVectorValue /*zeta_prime*/) const
{
  return 0;
}

Real RichardsSUPGnone::tauSUPG(RealVectorValue /*vel*/,
                               Real /*traceperm*/,
                               RealVectorValue /*b*/) const
{
  return 0;
}

RealVectorValue RichardsSUPGnone::dtauSUPG_dgradp(RealVectorValue /*vel*/,
                                                  RealTensorValue /*dvel_dgradp*/,
                                                  Real /*traceperm*/,
                                                  RealVectorValue /*b*/,
                                                  RealVectorValue /*db2_dgradp*/) const
{
  return RealVectorValue();
}

Real RichardsSUPGnone::dtauSUPG_dp(RealVectorValue /*vel*/,
                                   RealVectorValue /*dvel_dp*/,
                                   Real /*traceperm*/,
                                   RealVectorValue /*b*/,
                                   Real /*db2_dp*/) const
{
  return 0;
}

bool
RichardsSUPGnone::SUPG_trivial() const
{
  return true;
}
