//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarQuadratureFunction.h"
#include "MFEMQuadratureFunctionCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarQuadratureFunction);

InputParameters
MFEMScalarQuadratureFunction::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMQuadratureFunction");
  params.registerSystemAttributeName("MFEMQuadratureFunction");
  params.addClassDescription(
      "Declares a scalar MFEM coefficient holding precomputed values of a source coefficient at "
      "quadrature points. Values are (re)projected lazily when the coefficient is used.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient", "Scalar coefficient to project onto the quadrature points.");
  params.addRequiredRangeCheckedParam<int>(
      "order",
      "order>=0",
      "Order of the quadrature rule the projected values are stored on. This must match the "
      "integration rule used by the objects consuming this coefficient.");
  MooseEnum updates("none time solve", "solve");
  params.addParam<MooseEnum>(
      "updates",
      updates,
      "When the stored values are re-projected from the source coefficient: 'none' projects "
      "exactly once, 'time' re-projects when the simulation time changes, 'solve' additionally "
      "re-projects whenever solution variables change (e.g. between nonlinear iterations).");
  return params;
}

MFEMScalarQuadratureFunction::MFEMScalarQuadratureFunction(const InputParameters & parameters)
  : MFEMObject(parameters),
    _qspace(&getMFEMProblem().mesh().getMFEMParMesh(), getParam<int>("order")),
    _qf(&_qspace)
{
  // Zero-initialize the storage; real values are projected lazily on first use.
  _qf = 0.0;

  const auto & updates = getParam<MooseEnum>("updates");
  const auto update_policy = updates == "none"
                                 ? MFEMQuadratureFunctionCoefficient::UpdatePolicy::NONE
                                 : updates == "time"
                                       ? MFEMQuadratureFunctionCoefficient::UpdatePolicy::TIME
                                       : MFEMQuadratureFunctionCoefficient::UpdatePolicy::SOLVE;
  getMFEMProblem().getCoefficients().declareScalar<MFEMQuadratureFunctionCoefficient>(
      name(), getScalarCoefficient("coefficient"), _qf, update_policy);
}

#endif
