//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGapConductanceConstraint.h"

registerKokkosMortarConstraint("HeatTransferApp", KokkosGapConductanceConstraint);

InputParameters
KokkosGapConductanceConstraint::validParams()
{
  InputParameters params = MortarConstraint::validParams();
  params.addClassDescription("Computes the heat flux through a gap of constant conductance with a "
                             "mortar Lagrange multiplier.");
  params.addRequiredParam<Real>("k", "Gap conductance");
  params.addParam<Real>("min_gap",
                        1e-6,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");
  return params;
}

KokkosGapConductanceConstraint::KokkosGapConductanceConstraint(const InputParameters & parameters)
  : MortarConstraint(parameters), _k(getParam<Real>("k")), _min_gap(getParam<Real>("min_gap"))
{
}
