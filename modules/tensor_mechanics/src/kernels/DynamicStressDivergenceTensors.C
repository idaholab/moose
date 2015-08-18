/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DynamicStressDivergenceTensors.h"

#include "Material.h"

template<>
InputParameters validParams<DynamicStressDivergenceTensors>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Rayleigh damping and HHT time integration term to be used along with Stress divergence kernel in Tensor Mechanics module");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("displacements", "The string of displacements suitable for the problem statement");
  params.addCoupledVar("disp_x", "Depricated: the x displacement");
  params.addCoupledVar("disp_y", "Depricated: the y displacement");
  params.addCoupledVar("disp_z", "Depricated: the z displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<Real>("zeta", 0, "zeta parameter for the Rayleigh damping");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}


DynamicStressDivergenceTensors::DynamicStressDivergenceTensors(const InputParameters & parameters) :
    Kernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialPropertyByName<ElasticityTensorR4>(_base_name + "Jacobian_mult")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_var(3),
    _temp_coupled(isCoupled("temp")),
    _temp_var(_temp_coupled ? coupled("temp") : 0),
    _zeta(getParam<Real>("zeta")),
    _alpha(getParam<Real>("alpha"))
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
DynamicStressDivergenceTensors::computeQpResidual()
{
  /**
  *This kernel needs to be used only if Rayleigh damping needs to be added to the problem thorugh the stiffness dependent damping parameter _zeta.
  * The residual of _zeta*K*vel = _zeta*d/dt (Div sigma) = _zeta*(Div sigma - Div sigma_old)/dt is required
  */
  if ((_dt > 0))
    return _stress[_qp].row(_component) * _grad_test[_i][_qp]*(_alpha+_zeta/_dt)-(_alpha+_zeta/_dt)*_stress_old[_qp].row(_component)* _grad_test[_i][_qp];
  else
    return 0;
}

Real
DynamicStressDivergenceTensors::computeQpJacobian()
{
  if (_dt > 0)
    return _Jacobian_mult[_qp].elasticJacobian(_component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp])*(_alpha+_zeta/_dt);
  else
    return 0;
}

Real
DynamicStressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
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
  {
    if (_dt > 0)
      return _Jacobian_mult[_qp].elasticJacobian(_component, coupled_component, _grad_test[_i][_qp], _grad_phi[_j][_qp])*(_alpha+_zeta/_dt);
    else
      return 0;
   }
  if (_temp_coupled && jvar == _temp_var)
  {
    return 0.0;
  }

  return 0;
}
