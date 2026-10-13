//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ADComputeGlobalStrain.h"
#include "GlobalStrainPeriodicDirUserObject.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", ADComputeGlobalStrain);

InputParameters
ADComputeGlobalStrain::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription(
      "Builds an AD global-strain tensor from separate diagonal and off-diagonal scalar "
      "variables.");

  params.addParam<std::string>(
      "base_name",
      "Optional parameter that permits multiple mechanics material systems on one block");

  params.addRequiredCoupledVar(
      "diagonal_global_strain",
      "Scalar variable containing active diagonal global-strain components");
  params.addCoupledVar(
      "off_diagonal_global_strain",
      "Scalar variable containing active off-diagonal components in the order xy, xz, yz");
  params.addRequiredCoupledVar("displacements", "The displacement variables");
  params.addRequiredParam<UserObjectName>(
      "global_strain_uo", "UserObject that supplies the translated-periodic directions");

  // Retain all QP evaluations while validating the AD formulation.
  params.set<MooseEnum>("constant_on") = "NONE";
  return params;
}

ADComputeGlobalStrain::ADComputeGlobalStrain(const InputParameters & parameters)
  : ADMaterial(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _diagonal_global_strain(adCoupledScalarValue("diagonal_global_strain")),
    _off_diagonal_global_strain(isCoupledScalar("off_diagonal_global_strain")
                                    ? &adCoupledScalarValue("off_diagonal_global_strain")
                                    : nullptr),
    _number_of_diagonal_components(
        static_cast<unsigned int>(coupledScalarOrder("diagonal_global_strain"))),
    _number_of_off_diagonal_components(
        _off_diagonal_global_strain
            ? static_cast<unsigned int>(coupledScalarOrder("off_diagonal_global_strain"))
            : 0),
    _global_strain(declareADProperty<RankTwoTensor>(_base_name + "global_strain")),
    _periodicity_uo(getUserObject<GlobalStrainPeriodicDirUserObject>("global_strain_uo")),
    _periodic_dir(_periodicity_uo.getPeriodicDirections()),
    _diagonal_components(),
    _off_diagonal_components(),
    _dim(_mesh.dimension()),
    _ndisp(coupledComponents("displacements"))
{
  if (_ndisp != _dim)
    paramError("displacements",
               "ADComputeGlobalStrain requires one displacement variable per mesh dimension; ",
               _dim,
               " are required but ",
               _ndisp,
               " were supplied.");

  assignComponentIndices();
  validateScalarVariables();
}

void
ADComputeGlobalStrain::assignComponentIndices()
{
  _diagonal_components.clear();
  _off_diagonal_components.clear();

  // Diagonal ordering: xx, yy, zz, with nonperiodic entries removed.
  for (unsigned int i = 0; i < _dim; ++i)
    if (_periodic_dir(i))
      _diagonal_components.emplace_back(i, i);

  // Off-diagonal ordering is deliberately fixed as xy, xz, yz.
  // A shear is active whenever either associated direction is periodic.
  if (_dim >= 2 && (_periodic_dir(0) || _periodic_dir(1)))
    _off_diagonal_components.emplace_back(0, 1);

  if (_dim >= 3 && (_periodic_dir(0) || _periodic_dir(2)))
    _off_diagonal_components.emplace_back(0, 2);

  if (_dim >= 3 && (_periodic_dir(1) || _periodic_dir(2)))
    _off_diagonal_components.emplace_back(1, 2);
}

void
ADComputeGlobalStrain::validateScalarVariables() const
{
  if (_diagonal_components.empty())
    paramError("global_strain_uo", "At least one translated-periodic direction is required.");

  if (_number_of_diagonal_components != _diagonal_components.size())
    paramError("diagonal_global_strain",
               "The diagonal scalar variable contains ",
               _number_of_diagonal_components,
               " component(s), but the periodic directions require ",
               _diagonal_components.size(),
               ".");

  if (_off_diagonal_components.empty())
  {
    if (_off_diagonal_global_strain)
      paramError("off_diagonal_global_strain",
                 "No off-diagonal global-strain component is active; omit this variable.");
  }
  else if (!_off_diagonal_global_strain)
    paramError("off_diagonal_global_strain",
               "The periodic directions require ",
               _off_diagonal_components.size(),
               " off-diagonal global-strain component(s).");
  else if (_number_of_off_diagonal_components != _off_diagonal_components.size())
    paramError("off_diagonal_global_strain",
               "The off-diagonal scalar variable contains ",
               _number_of_off_diagonal_components,
               " component(s), but the periodic directions require ",
               _off_diagonal_components.size(),
               ". Components are ordered xy, xz, yz after inactive entries are removed.");
}

void
ADComputeGlobalStrain::initQpStatefulProperties()
{
  _global_strain[_qp].zero();
}

void
ADComputeGlobalStrain::fillGlobalStrainTensor(ADRankTwoTensor & strain) const
{
  strain.zero();

  for (unsigned int component = 0; component < _diagonal_components.size(); ++component)
  {
    const auto & indices = _diagonal_components[component];
    strain(indices.first, indices.second) = _diagonal_global_strain[component];
  }

  for (unsigned int component = 0; component < _off_diagonal_components.size(); ++component)
  {
    const auto & indices = _off_diagonal_components[component];
    const ADReal value = (*_off_diagonal_global_strain)[component];

    strain(indices.first, indices.second) = value;
    strain(indices.second, indices.first) = value;
  }
}

void
ADComputeGlobalStrain::computeQpProperties()
{
  fillGlobalStrainTensor(_global_strain[_qp]);
}
