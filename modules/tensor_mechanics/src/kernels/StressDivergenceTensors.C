/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergenceTensors.h"
#include "Material.h"
#include "MooseMesh.h"
#include "ElasticityTensorTools.h"

template<>
InputParameters validParams<StressDivergenceTensors>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Stress divergence kernel (used by the TensorMechanics action)");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("displacements", "The string of displacements suitable for the problem statement");
  params.addCoupledVar("disp_x", "Depricated: the x displacement");
  params.addCoupledVar("disp_y", "Depricated: the y displacement");
  params.addCoupledVar("disp_z", "Depricated: the z displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}


StressDivergenceTensors::StressDivergenceTensors(const InputParameters & parameters) :
    Kernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>(_base_name + "Jacobian_mult")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_var(3),
    _temp_coupled(isCoupled("temp")),
    _temp_var(_temp_coupled ? coupled("temp") : 0)
{
  if (_ndisp)
  {
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      _disp[i] = &coupledValue("displacements", i);
      _disp_var[i] = coupled("displacements", i);
    }
  }

  //This code is for the depricated version that allows three unique displacement variables
  else if ((_ndisp == 0) && isParamValid("disp_x"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of displacement variable names, e.g. displacements = 'disp_x disp_y' in the input file.");
    _disp[0] = &coupledValue("disp_x");
    _disp_var[0] = coupled("disp_x");
    ++_ndisp;
    if (isParamValid("disp_y"))
    {
      _disp[1] = &coupledValue("disp_y");
      _disp_var[1] = coupled("disp_y");
      ++_ndisp;
      if (isParamValid("disp_z"))
      {
        _disp[2] = &coupledValue("disp_z");
        _disp_var[2] = coupled("disp_z");
        ++_ndisp;
      }
    }
  }

  else
    mooseError("The input file should specify a string of displacement names; these names should match the Variable block names.");

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension.");
}

Real
StressDivergenceTensors::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}

Real
StressDivergenceTensors::computeQpJacobian()
{
  return ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], _component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
}

Real
StressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;
  bool active(false);

  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
    {
      coupled_component = i;
      active = true;
    }

  if (active)
    return ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], _component, coupled_component,
                                          _grad_test[_i][_qp], _grad_phi[_j][_qp]);

  if (_temp_coupled && jvar == _temp_var)
  {
    //return _d_stress_dT[_qp].rowDot(_component, _grad_test[_i][_qp]) * _phi[_j][_qp];
    return 0.0;
  }

  return 0;
}
