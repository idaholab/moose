//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMQuadratureFunctionBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMQuadratureFunctionBase::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMQuadratureFunction");
  params.registerSystemAttributeName("MFEMQuadratureFunction");
  params.addRequiredRangeCheckedParam<int>(
      "order",
      "order>=0",
      "Order of the quadrature rule the projected values are stored on. This must match the "
      "integration rule used by the objects consuming this coefficient.");
  MooseEnum updates(MFEMQuadratureFunctionCoefficientBase::getUpdatePolicyOptions(), "NONLINEAR");
  params.addParam<MooseEnum>(
      "updates",
      updates,
      "When the stored values are re-projected from the source coefficient: 'none' projects "
      "exactly once, 'time' re-projects when the simulation time changes, 'nonlinear' additionally "
      "re-projects whenever solution variables change (i.e. on each nonlinear iteration).");
  return params;
}

MFEMQuadratureFunctionBase::MFEMQuadratureFunctionBase(const InputParameters & parameters)
  : MFEMObject(parameters),
    _qspace(&getMFEMProblem().mesh().getMFEMParMesh(), getParam<int>("order"))
{
}

MFEMQuadratureFunctionCoefficientBase::UpdatePolicy
MFEMQuadratureFunctionBase::updatePolicy() const
{
  return getParam<MooseEnum>("updates")
      .getEnum<MFEMQuadratureFunctionCoefficientBase::UpdatePolicy>();
}

#endif
