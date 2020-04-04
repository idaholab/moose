//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StretchBasedFailureCriterionPD.h"

registerMooseObject("PeridynamicsApp", StretchBasedFailureCriterionPD);

InputParameters
StretchBasedFailureCriterionPD::validParams()
{
  InputParameters params = BondStatusBasePD::validParams();
  params.addClassDescription("Class for bond stretch failure criterion in bond-based model and "
                             "ordinary state-based model");

  return params;
}

StretchBasedFailureCriterionPD::StretchBasedFailureCriterionPD(const InputParameters & parameters)
  : BondStatusBasePD(parameters),
    _mechanical_stretch(getMaterialProperty<Real>("mechanical_stretch"))
{
}

Real
StretchBasedFailureCriterionPD::computeFailureCriterionValue()
{
  return _mechanical_stretch[0] - _critical_val[0];
}
