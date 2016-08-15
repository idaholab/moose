/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergenceRZTensors.h"
#include "Assembly.h"
#include "ElasticityTensorTools.h"

template<>
InputParameters validParams<StressDivergenceRZTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addClassDescription ("Calculate stress divergence for an axisymmetric problem in cylinderical coordinates.");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z; note in this kernel disp_x refers to the radial displacement and disp_y refers to the axial displacement.)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceRZTensors::StressDivergenceRZTensors(const InputParameters & parameters) :
    StressDivergenceTensors(parameters)
{
}

Real
StressDivergenceRZTensors::computeQpResidual()
{
  mooseAssert(_assembly.coordSystem() == Moose::COORD_RZ,
              "The coordinate system in the Problem block must be set to RZ for Axisymmetric geometries.");

  Real div = 0;
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp](0,0) +
        + ( _test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](2,2) +
        + _grad_test[_i][_qp](1) * _stress[_qp](0,1); // stress_{rz}
  }
  else if (_component == 1)
  {
    div = _grad_test[_i][_qp](1) * _stress[_qp](1,1) +
        + _grad_test[_i][_qp](0) * _stress[_qp](1,0);  // stress_{zr}
  }
  else
    mooseError("Invalid component for this AxisymmetricRZ problem.");

  return div;
}

Real
StressDivergenceRZTensors::computeQpJacobian()
{
  return calculateJacobian(_component, _component);
}

Real
StressDivergenceRZTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    if (jvar == _disp_var[i])
      return calculateJacobian( _component, i);
  }

  if (_temp_coupled && jvar == _temp_var)
    return 0.0;

  return 0;
}

Real
StressDivergenceRZTensors::calculateJacobian(unsigned int ivar, unsigned int jvar)
{
  RealGradient test, test_z, phi, phi_z;

  if (ivar == 0)  //Case grad_test for x, requires contributions from stress_xx, stress_xy, and stress_zz
  {
    test(0) = _grad_test[_i][_qp](0);
    test(1) = _grad_test[_i][_qp](1);
    test_z(2) = _test[_i][_qp] / _q_point[_qp](0);
  }
  else // Case grad_test for y
  {
    test(0) = _grad_test[_i][_qp](0);
    test(1) = _grad_test[_i][_qp](1);
  }

  if (jvar == 0)
  {
    phi(0) = _grad_phi[_j][_qp](0);
    phi(1) = _grad_phi[_j][_qp](1);
    phi_z(2) = _phi[_j][_qp] / _q_point[_qp](0);
  }
  else
  {
    phi(0) = _grad_phi[_j][_qp](0);
    phi(1) = _grad_phi[_j][_qp](1);
  }

  if (ivar == 0 && jvar == 0)  // Case when both phi and test are functions of x and z; requires four terms
  {
    const Real first_sum = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, jvar, test, phi); //test_x and phi_x
    const Real second_sum = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], 2, 2, test_z, phi_z); //test_z and phi_z
    const Real mixed_sum1 = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, 2, test, phi_z); //test_x and phi_z
    const Real mixed_sum2 = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], 2, jvar, test_z, phi); //test_z and phi_x

    return first_sum + second_sum + mixed_sum1 + mixed_sum2;
  }
  else if (ivar == 0 && jvar == 1)
  {
    const Real first_sum = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, jvar, test, phi); //test_x and phi_y
    const Real mixed_sum2 = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], 2, jvar, test_z, phi); //test_z and phi_y

    return first_sum + mixed_sum2;
  }
  else if (ivar == 1 && jvar == 0)
  {
    const Real second_sum = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, jvar, test, phi); //test_y and phi_x
    const Real mixed_sum1 = ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, 2, test, phi_z); //test_y and phi_z

    return second_sum + mixed_sum1;
  }
  else if (ivar == 1 && jvar == 1)
    return ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], ivar, jvar, test, phi); //test_y and phi_y

  else
    mooseError("Invalid component in Jacobian Calculation");
}
