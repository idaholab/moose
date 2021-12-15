//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceRSphericalTensors.h"
#include "ElasticityTensorTools.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceRSphericalTensors);

InputParameters
StressDivergenceRSphericalTensors::validParams()
{
  InputParameters params = StressDivergenceTensors::validParams();
  params.addClassDescription(
      "Calculate stress divergence for a spherically symmetric 1D problem in polar coordinates.");
  params.set<unsigned int>("component") = 0;
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceRSphericalTensors::StressDivergenceRSphericalTensors(
    const InputParameters & parameters)
  : StressDivergenceTensors(parameters)
{
  if (_component != 0)
    mooseError("Invalid component for this 1D RSpherical problem.");
}

void
StressDivergenceRSphericalTensors::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RSPHERICAL)
    mooseError("The coordinate system in the Problem block must be set to RSPHERICAL for 1D "
               "spherically symmetric geometries.");
}

Real
StressDivergenceRSphericalTensors::computeQpResidual()
{
  return _grad_test[_i][_qp](0) * _stress[_qp](0, 0) +               // stress_{rr} part 1
         +(_test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](1, 1) + // stress_{\theta \theta}
         +(_test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](2, 2);  // stress_{\phi \phi}
}

Real
StressDivergenceRSphericalTensors::computeQpJacobian()
{
  return calculateJacobian(_component, _component);
}

Real
StressDivergenceRSphericalTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
      return calculateJacobian(_component, i);

  return 0.0;
}

Real
StressDivergenceRSphericalTensors::calculateJacobian(unsigned int libmesh_dbg_var(ivar),
                                                     unsigned int libmesh_dbg_var(jvar))
{
  mooseAssert(ivar == 0 && jvar == 0,
              "Invalid component in Jacobian Calculation"); // Only nonzero case for a 1D simulation

  const Real test = _grad_test[_i][_qp](0);
  const Real test_r1 = _test[_i][_qp] / _q_point[_qp](0);
  // const Real test_r2 = test_r1;

  const Real phi = _grad_phi[_j][_qp](0);
  const Real phi_r1 = _phi[_j][_qp] / _q_point[_qp](0);
  const Real phi_r2 = phi_r1;

  const Real term1 = test * _Jacobian_mult[_qp](0, 0, 0, 0) * phi;
  const Real term2 = test * _Jacobian_mult[_qp](0, 0, 1, 1) * phi_r1; // same as term3
  // const Real term3 = test * _Jacobian_mult[_qp](0, 0, 2, 2) * phi_r2;

  const Real term4 = test_r1 * _Jacobian_mult[_qp](1, 1, 0, 0) * phi;    // same as term7
  const Real term5 = test_r1 * _Jacobian_mult[_qp](1, 1, 1, 1) * phi_r1; // same as term9
  const Real term6 = test_r1 * _Jacobian_mult[_qp](1, 1, 2, 2) * phi_r2; // same as term8

  // const Real term7 = test_r2 * _Jacobian_mult[_qp](2, 2, 0, 0) * phi;
  // const Real term8 = test_r2 * _Jacobian_mult[_qp](2, 2, 1, 1) * phi_r1;
  // const Real term9 = test_r2 * _Jacobian_mult[_qp](2, 2, 2, 2) * phi_r2;

  return term1 + 2 * (term2 + term4 + term5 + term6);
}
