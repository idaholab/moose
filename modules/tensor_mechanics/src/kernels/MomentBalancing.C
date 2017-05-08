/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MomentBalancing.h"

// MOOSE includes
#include "ElasticityTensorTools.h"
#include "Material.h"
#include "MooseVariable.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

template <>
InputParameters
validParams<MomentBalancing>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component<3",
      "An integer corresponding to the direction the variable this "
      "kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  params.addRequiredCoupledVar("displacements", "The 3 displacement variables");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

MomentBalancing::MomentBalancing(const InputParameters & parameters)
  : Kernel(parameters),
    _stress(getMaterialProperty<RankTwoTensor>("stress" +
                                               getParam<std::string>("appended_property_name"))),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(
        "Jacobian_mult" + getParam<std::string>("appended_property_name"))),
    _component(getParam<unsigned int>("component")),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc_var(_nrots),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp)
{
  if (_nrots != 3)
    mooseError("MomentBalancing: This Kernel is only defined for 3-dimensional simulations so 3 "
               "Cosserat rotation variables are needed");
  for (unsigned i = 0; i < _nrots; ++i)
    _wc_var[i] = coupled("Cosserat_rotations", i);

  if (_ndisp != 3)
    mooseError("MomentBalancing: This Kernel is only defined for 3-dimensional simulations so 3 "
               "displacement variables are needed");
  for (unsigned i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  // Following check is necessary to ensure the correct Jacobian is calculated
  if (_wc_var[_component] != _var.number())
    mooseError("MomentBalancing: The variable for this Kernel must be equal to the Cosserat "
               "rotation variable defined by the \"component\" and the \"Cosserat_rotations\" "
               "parameters");
}

Real
MomentBalancing::computeQpResidual()
{
  Real the_sum = 0.0;
  for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      the_sum += RankThreeTensor::leviCivita(_component, j, k) * _stress[_qp](j, k);
  return _test[_i][_qp] * the_sum;
}

Real
MomentBalancing::computeQpJacobian()
{
  return ElasticityTensorTools::momentJacobianWC(
      _Jacobian_mult[_qp], _component, _component, _test[_i][_qp], _phi[_j][_qp]);
}

Real
MomentBalancing::computeQpOffDiagJacobian(unsigned int jvar)
{
  // What does 2D look like here?
  for (unsigned v = 0; v < _ndisp; ++v)
    if (jvar == _disp_var[v])
      return ElasticityTensorTools::momentJacobian(
          _Jacobian_mult[_qp], _component, v, _test[_i][_qp], _grad_phi[_j][_qp]);

  // What does 2D look like here?
  for (unsigned v = 0; v < _nrots; ++v)
    if (jvar == _wc_var[v])
      return ElasticityTensorTools::momentJacobianWC(
          _Jacobian_mult[_qp], _component, v, _test[_i][_qp], _phi[_j][_qp]);

  return 0.0;
}
