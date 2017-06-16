/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFractureBulkRateBase.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<PFFractureBulkRateBase>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  params.addRequiredParam<Real>("width", "Width of the smooth crack representation");
  params.addRequiredParam<Real>(
      "viscosity", "Viscosity parameter, which reflects the transition right at crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "gc", "Material property which provides the maximum stress/crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "G0", "Material property name with undamaged strain energy driving damage (G0_pos)");
  params.addParam<MaterialPropertyName>(
      "dG0_dstrain", "Material property name with derivative of G0_pos with strain");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

PFFractureBulkRateBase::PFFractureBulkRateBase(const InputParameters & parameters)
  : Kernel(parameters),
    _gc_prop(getMaterialProperty<Real>("gc")),
    _G0_pos(getMaterialProperty<Real>("G0")),
    _dG0_pos_dstrain(
        isParamValid("dG0_dstrain") ? &getMaterialProperty<RankTwoTensor>("dG0_dstrain") : NULL),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _width(getParam<Real>("width")),
    _viscosity(getParam<Real>("viscosity"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}
