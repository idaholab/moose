/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
//  Hardening model base class.
//
#include "TensorMechanicsHardeningModel.h"

template <>
InputParameters
validParams<TensorMechanicsHardeningModel>()
{
  InputParameters params = validParams<GeneralUserObject>();
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
