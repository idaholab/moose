//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHomogenizedThermalConductivity.h"

registerKokkosUserObject("HeatTransferApp", KokkosHomogenizedThermalConductivity);

InputParameters
KokkosHomogenizedThermalConductivity::validParams()
{
  InputParameters params = KokkosElementIntegralPostprocessor::validParams();
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
  // The homogenized conductivity is a volume average
  params.set<bool>("average") = true;
  params.suppressParameter<bool>("average");
  return params;
}

KokkosHomogenizedThermalConductivity::KokkosHomogenizedThermalConductivity(
    const InputParameters & parameters)
  : KokkosElementIntegralPostprocessor(parameters),
    _row(getParam<unsigned int>("row")),
    _col(getParam<unsigned int>("col")),
    _scale(getParam<Real>("scale_factor")),
    _dim(_subproblem.mesh().dimension()),
    _grad_chi(kokkosCoupledGradients("chi")),
    _is_tensor(getParam<bool>("is_tensor")),
    _diffusion_coefficient(!_is_tensor ? getKokkosMaterialProperty<Real>("diffusion_coefficient")
                                       : Moose::Kokkos::MaterialProperty<Real>()),
    _tensor_diffusion_coefficient(
        _is_tensor ? getKokkosMaterialProperty<Moose::Kokkos::Real33>("diffusion_coefficient")
                   : Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33>())
{
  if (_row >= _dim)
    paramError("row", "Must be smaller than mesh dimension (0, 1, 2 for 1D, 2D, 3D)");
  if (_col >= _dim)
    paramError("col", "Must be smaller than mesh dimension (0, 1, 2 for 1D, 2D, 3D)");
  if (coupledComponents("chi") != _dim)
    paramError("chi", "The number of entries must be identical to the mesh dimension.");
}
