//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedThermalConductivity.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("HeatConductionApp", HomogenizedThermalConductivity);

InputParameters
HomogenizedThermalConductivity::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Postprocessor for asymptotic expansion homogenization for thermal conductivity");
  params.addRequiredCoupledVar(
      "chi", "The characteristic functions used for homogenization of the thermal conductivity.");
  params.addRequiredParam<unsigned int>(
      "row",
      "The row index of the homogenized thermal conductivity tensor entry computed by this "
      "postprocessor.");
  params.addRequiredParam<unsigned int>(
      "col",
      "The column index of the homogenized thermal conductivity tensor entry computed by this "
      "postprocessor.");

  params.addParam<Real>("scale_factor", 1, "Scale factor");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient", "thermal_conductivity", "Property name of the diffusivity");
  params.addParam<bool>(
      "is_tensor", false, "True if the material property in diffusion_coefficient is a tensor");
  return params;
}

HomogenizedThermalConductivity::HomogenizedThermalConductivity(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _row(getParam<unsigned int>("row")),
    _col(getParam<unsigned int>("col")),
    _scale(getParam<Real>("scale_factor")),
    _dim(_mesh.dimension()),
    _diffusion_coefficient(!getParam<bool>("is_tensor")
                               ? &getMaterialProperty<Real>("diffusion_coefficient")
                               : nullptr),
    _tensor_diffusion_coefficient(getParam<bool>("is_tensor")
                                      ? &getMaterialProperty<RankTwoTensor>("diffusion_coefficient")
                                      : nullptr)
{
  if (_row >= _dim)
    paramError("row", "Must be smaller than mesh dimension (0, 1, 2 for 1D, 2D, 3D)");

  if (_col >= _dim)
    paramError("col", "Must be smaller than mesh dimension (0, 1, 2 for 1D, 2D, 3D)");

  if (coupledComponents("chi") != _dim)
    paramError("chi", "The number of entries must be identical to the mesh dimension.");

  _grad_chi.resize(_dim);
  for (unsigned int j = 0; j < _dim; ++j)
    _grad_chi[j] = &coupledGradient("chi", j);
}

void
HomogenizedThermalConductivity::initialize()
{
  _integral_value = 0.0;
  _volume = 0.0;
}

void
HomogenizedThermalConductivity::execute()
{
  _integral_value += computeIntegral();
  _volume += _current_elem_volume;
}

Real
HomogenizedThermalConductivity::getValue()
{
  return _integral_value / _volume;
}

void
HomogenizedThermalConductivity::finalize()
{
  gatherSum(_integral_value);
  gatherSum(_volume);
}

void
HomogenizedThermalConductivity::threadJoin(const UserObject & y)
{
  const HomogenizedThermalConductivity & pps =
      dynamic_cast<const HomogenizedThermalConductivity &>(y);

  _integral_value += pps._integral_value;
  _volume += pps._volume;
}

Real
HomogenizedThermalConductivity::computeQpIntegral()
{
  // the _row-th row of the thermal conductivity tensor
  RealVectorValue k_row;
  if (_diffusion_coefficient)
    k_row(_row) = (*_diffusion_coefficient)[_qp];
  else
    for (unsigned int j = 0; j < _dim; ++j)
      k_row(j) = (*_tensor_diffusion_coefficient)[_qp](_row, j);

  // initialize the dchi/dx tensor, but we only use _col-th column
  RealVectorValue M_col;
  M_col(_col) = 1.0;
  for (unsigned int i = 0; i < _dim; ++i)
    M_col(i) += (*_grad_chi[_col])[_qp](i);

  return _scale * k_row * M_col;
}
