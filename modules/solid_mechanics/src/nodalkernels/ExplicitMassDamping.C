//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitMassDamping.h"
#include "MooseError.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

registerMooseObject("SolidMechanicsApp", ExplicitMassDamping);

InputParameters
ExplicitMassDamping::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription(
      "Adds Rayleigh mass damping, eta * M * v, to an ExplicitMixedOrder solid mechanics model.");
  params.addRangeCheckedParam<Real>(
      "eta",
      0.0,
      "eta >= 0.0",
      "Damping strength.  Mass damping helps to damp low-frequency, large-scale oscillations.  A "
      "reasonable value to use initially is eta = 2 * d * omega, where d is the damping fraction "
      "(typically 0.1) and omega is the lowest relevant frequency mode of the model.  For "
      "instance, if the model is can oscillate longitudinally, omega = (pi / model_length) * "
      "sqrt(youngs_modulus / density)");
  return params;
}

ExplicitMassDamping::ExplicitMassDamping(const InputParameters & parameters)
  : NodalKernel(parameters),
    _eta(getParam<Real>("eta")),
    _mass_matrix_lumped(initLumpedMass()),
    _u_dot_old(_var.nodalValueDotOld())
{
}

Real
ExplicitMassDamping::computeQpResidual()
{
  const auto dofnum = _variable->nodalDofIndex(); // dof for current var
  return _eta * _mass_matrix_lumped(dofnum) * _u_dot_old;
}

const NumericVector<Number> &
ExplicitMassDamping::initLumpedMass()
{
  const auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  if (nl.hasVector("mass_matrix_lumped"))
    return nl.getVector("mass_matrix_lumped");

  mooseError("Lumped mass matrix is missing. Make sure ExplicitMixedOrder is being used as the "
             "time integrator.");
}
