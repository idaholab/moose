/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeStrainBase.h"

template<>
InputParameters validParams<ComputeStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("displacements", "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<Real>("temperature_ref", 273, "Reference temperature for thermal expansion in K");
  params.addCoupledVar("temperature", 273, "temperature in Kelvin");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addParam<Real>("thermal_expansion_coeff", 0, "Thermal expansion coefficient in 1/K");
  return params;
}

ComputeStrainBase::ComputeStrainBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_old(3),
    _grad_disp(3),
    _grad_disp_old(3),
    _T(coupledValue("temperature")),
    _T0(getParam<Real>("temperature_ref")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain"))
{
  if (isParamValid("displacements") == false)
    mooseError("The displacement variables for the Compute*Strain Materials must be provided in a string: displacements = 'disp_x disp_y'");
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of variables supplied in 'displacements' must match the mesh dimension.");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
    if (_fe_problem.isTransient())
    {
      _disp_old[i] = &coupledValueOld("displacements", i);
      _grad_disp_old[i] = &coupledGradientOld("displacements" ,i);
    }
    else
    {
      _disp_old[i] = &_zero;
      _grad_disp_old[i] = &_grad_zero;
    }
  }
  if (_ndisp < 3)
  {
    _disp[2] = &_zero;
    _disp_old[2] = &_zero;
    _grad_disp[2] = &_grad_zero;
    _grad_disp_old[2] = &_grad_zero;
  }
}

void
ComputeStrainBase::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
}


// DEPRECATED CONSTRUCTOR
ComputeStrainBase::ComputeStrainBase(const std::string & deprecated_name, InputParameters parameters) :
    DerivativeMaterialInterface<Material>(deprecated_name, parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _grad_disp(3),
    _grad_disp_old(3),
    _T(coupledValue("temperature")),
    _T0(getParam<Real>("temperature_ref")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain"))
{
  if (isParamValid("displacements") == false)
    mooseError("The displacement variables for the Compute*Strain Materials must be provided in a string: displacements = 'disp_x disp_y'");
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of variables supplied in 'displacements' must match the mesh dimension.");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
    if (_fe_problem.isTransient())
      _grad_disp_old[i] = &coupledGradientOld("displacements" ,i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
  if (_ndisp < 3)
  {
    _disp[2] = &_zero;
    _grad_disp[2] = &_grad_zero;
    _grad_disp_old[2] = &_grad_zero;
  }
}
