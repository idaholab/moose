/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
//  This post processor returns the Interaction Integral
//
#include "InteractionIntegral.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<InteractionIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addCoupledVar("q", "The q function, aux variable");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp",
                       "The temperature (optional). Must be provided to correctly compute "
                       "stress intensity factors in models with thermal strain gradients.");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front corresponding to this q function");
  params.addParam<Real>(
      "K_factor", "Conversion factor between interaction integral and stress intensity factor K");
  params.addParam<unsigned int>("symmetry_plane",
                                "Account for a symmetry plane passing through "
                                "the plane of the crack, normal to the specified "
                                "axis (0=x, 1=y, 2=z)");
  params.addParam<bool>("t_stress", false, "Calculate T-stress");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

InteractionIntegral::InteractionIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _scalar_q(coupledValue("q")),
    _grad_of_scalar_q(coupledGradient("q")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_point_index(isParamValid("crack_front_point_index")),
    _crack_front_point_index(
        _has_crack_front_point_index ? getParam<unsigned int>("crack_front_point_index") : 0),
    _treat_as_2d(false),
    _stress(getMaterialPropertyByName<SymmTensor>("stress")),
    _strain(getMaterialPropertyByName<SymmTensor>("elastic_strain")),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(parameters.get<SubProblem *>("_subproblem")->mesh().dimension() == 3
                     ? coupledGradient("disp_z")
                     : _grad_zero),
    _has_temp(isCoupled("temp")),
    _grad_temp(_has_temp ? coupledGradient("temp") : _grad_zero),
    _aux_stress(getMaterialProperty<ColumnMajorMatrix>("aux_stress")),
    _aux_grad_disp(getMaterialProperty<ColumnMajorMatrix>("aux_grad_disp")),
    _current_instantaneous_thermal_expansion_coef(
        hasMaterialProperty<Real>("current_instantaneous_thermal_expansion_coef")
            ? &getMaterialProperty<Real>("current_instantaneous_thermal_expansion_coef")
            : NULL),
    _K_factor(getParam<Real>("K_factor")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _t_stress(getParam<bool>("t_stress")),
    _poissons_ratio(getParam<Real>("poissons_ratio"))
{
  if (_has_temp && !_current_instantaneous_thermal_expansion_coef)
    mooseError("To include thermal strain term in interaction integral, must both couple "
               "temperature in DomainIntegral block and compute thermal expansion property in "
               "material model using compute_InteractionIntegral = true.");
}

void
InteractionIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();

  if (_treat_as_2d)
  {
    if (_has_crack_front_point_index)
      mooseWarning(
          "crack_front_point_index ignored because CrackFrontDefinition is set to treat as 2D");
  }
  else
  {
    if (!_has_crack_front_point_index)
      mooseError("crack_front_point_index must be specified in qFunctionJIntegral3D");
  }
}

Real
InteractionIntegral::getValue()
{
  gatherSum(_integral_value);

  if (_t_stress && !_treat_as_2d)
    _integral_value +=
        _poissons_ratio *
        _crack_front_definition->getCrackFrontTangentialStrain(_crack_front_point_index);

  return _K_factor * _integral_value;
}

Real
InteractionIntegral::computeQpIntegral()
{

  RealVectorValue grad_q = _grad_of_scalar_q[_qp];

  // In the crack front coordinate system, the crack direction is (1,0,0)
  RealVectorValue crack_direction(0.0);
  crack_direction(0) = 1.0;

  ColumnMajorMatrix aux_du;
  aux_du(0, 0) = _aux_grad_disp[_qp](0, 0);
  aux_du(0, 1) = _aux_grad_disp[_qp](0, 1);
  aux_du(0, 2) = _aux_grad_disp[_qp](0, 2);

  ColumnMajorMatrix stress;
  stress(0, 0) = _stress[_qp].xx();
  stress(0, 1) = _stress[_qp].xy();
  stress(0, 2) = _stress[_qp].xz();
  stress(1, 0) = _stress[_qp].xy();
  stress(1, 1) = _stress[_qp].yy();
  stress(1, 2) = _stress[_qp].yz();
  stress(2, 0) = _stress[_qp].xz();
  stress(2, 1) = _stress[_qp].yz();
  stress(2, 2) = _stress[_qp].zz();

  ColumnMajorMatrix strain;
  strain(0, 0) = _strain[_qp].xx();
  strain(0, 1) = _strain[_qp].xy();
  strain(0, 2) = _strain[_qp].xz();
  strain(1, 0) = _strain[_qp].xy();
  strain(1, 1) = _strain[_qp].yy();
  strain(1, 2) = _strain[_qp].yz();
  strain(2, 0) = _strain[_qp].xz();
  strain(2, 1) = _strain[_qp].yz();
  strain(2, 2) = _strain[_qp].zz();

  ColumnMajorMatrix grad_disp;
  grad_disp(0, 0) = _grad_disp_x[_qp](0);
  grad_disp(0, 1) = _grad_disp_x[_qp](1);
  grad_disp(0, 2) = _grad_disp_x[_qp](2);
  grad_disp(1, 0) = _grad_disp_y[_qp](0);
  grad_disp(1, 1) = _grad_disp_y[_qp](1);
  grad_disp(1, 2) = _grad_disp_y[_qp](2);
  grad_disp(2, 0) = _grad_disp_z[_qp](0);
  grad_disp(2, 1) = _grad_disp_z[_qp](1);
  grad_disp(2, 2) = _grad_disp_z[_qp](2);

  // Rotate stress, strain, displacement and temperature to crack front coordinate system
  RealVectorValue grad_q_cf =
      _crack_front_definition->rotateToCrackFrontCoords(grad_q, _crack_front_point_index);
  ColumnMajorMatrix grad_disp_cf =
      _crack_front_definition->rotateToCrackFrontCoords(grad_disp, _crack_front_point_index);
  ColumnMajorMatrix stress_cf =
      _crack_front_definition->rotateToCrackFrontCoords(stress, _crack_front_point_index);
  ColumnMajorMatrix strain_cf =
      _crack_front_definition->rotateToCrackFrontCoords(strain, _crack_front_point_index);
  RealVectorValue grad_temp_cf =
      _crack_front_definition->rotateToCrackFrontCoords(_grad_temp[_qp], _crack_front_point_index);

  ColumnMajorMatrix dq;
  dq(0, 0) = crack_direction(0) * grad_q_cf(0);
  dq(0, 1) = crack_direction(0) * grad_q_cf(1);
  dq(0, 2) = crack_direction(0) * grad_q_cf(2);

  // Calculate interaction integral terms

  // Term1 = stress * x1-derivative of aux disp * dq
  ColumnMajorMatrix tmp1 = dq * stress_cf;
  Real term1 = aux_du.doubleContraction(tmp1);

  // Term2 = aux stress * x1-derivative of disp * dq
  ColumnMajorMatrix tmp2 = dq * _aux_stress[_qp];
  Real term2 = grad_disp_cf(0, 0) * tmp2(0, 0) + grad_disp_cf(1, 0) * tmp2(0, 1) +
               grad_disp_cf(2, 0) * tmp2(0, 2);

  // Term3 = aux stress * strain * dq_x   (= stress * aux strain * dq_x)
  Real term3 = dq(0, 0) * _aux_stress[_qp].doubleContraction(strain_cf);

  // Term4 (thermal strain term) = q * aux_stress * alpha * dtheta_x
  // - the term including the derivative of alpha is not implemented
  Real term4 = 0.0;
  if (_has_temp)
  {
    Real aux_stress_trace =
        _aux_stress[_qp](0, 0) + _aux_stress[_qp](1, 1) + _aux_stress[_qp](2, 2);
    term4 = _scalar_q[_qp] * aux_stress_trace *
            (*_current_instantaneous_thermal_expansion_coef)[_qp] * grad_temp_cf(0);
  }

  Real q_avg_seg = 1.0;
  if (!_crack_front_definition->treatAs2D())
  {
    q_avg_seg =
        (_crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_point_index) +
         _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_point_index)) /
        2.0;
  }

  Real eq = term1 + term2 - term3 + term4;

  if (_has_symmetry_plane)
    eq *= 2.0;

  return eq / q_avg_seg;
}
