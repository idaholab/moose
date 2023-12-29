//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Hardening model base class.
//
#include "TensorMechanicsHardeningModel.h"

InputParameters
TensorMechanicsHardeningModel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Hardening Model base class.  Override the virtual functions in your class");
  return params;
}

TensorMechanicsHardeningModel::TensorMechanicsHardeningModel(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
TensorMechanicsHardeningModel::initialize()
{
}

void
TensorMechanicsHardeningModel::execute()
{
}

void
TensorMechanicsHardeningModel::finalize()
{
}

Real TensorMechanicsHardeningModel::value(Real /*intnl*/) const { return 1.0; }

Real TensorMechanicsHardeningModel::derivative(Real /*intnl*/) const { return 0.0; }
