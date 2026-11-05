//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ADGlobalDisplacementAux.h"
#include "GlobalStrainPeriodicDirUserObject.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", ADGlobalDisplacementAux);

InputParameters
ADGlobalDisplacementAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes affine displacement from split diagonal/off-diagonal global-strain variables and "
      "optionally adds the periodic displacement solution.");

  params.addRequiredCoupledVar(
      "diagonal_global_strain",
      "Scalar variable containing active diagonal global-strain components");
  params.addCoupledVar(
      "off_diagonal_global_strain",
      "Scalar variable containing active off-diagonal components in the order xy, xz, yz");
  params.addRequiredCoupledVar("displacements", "The nonlinear displacement variables");
  params.addRequiredParam<unsigned int>("component",
                                        "Displacement component produced by this AuxKernel");
  params.addParam<bool>(
      "output_global_displacement", false, "Output only the affine global displacement");
  params.addRequiredParam<UserObjectName>(
      "global_strain_uo", "UserObject that supplies the translated-periodic directions");
  params.addParam<Point>("reference_point",
                         Point(0, 0, 0),
                         "Coordinate of the center or fixed point of the simulation");

  params.set<ExecFlagEnum>("execute_on") = EXEC_PRE_DISPLACE;
  return params;
}

ADGlobalDisplacementAux::ADGlobalDisplacementAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _diagonal_global_strain(coupledScalarValue("diagonal_global_strain")),
    _off_diagonal_global_strain(isCoupledScalar("off_diagonal_global_strain")
                                    ? &coupledScalarValue("off_diagonal_global_strain")
                                    : nullptr),
    _number_of_diagonal_components(
        static_cast<unsigned int>(coupledScalarOrder("diagonal_global_strain"))),
    _number_of_off_diagonal_components(
        _off_diagonal_global_strain
            ? static_cast<unsigned int>(coupledScalarOrder("off_diagonal_global_strain"))
            : 0),
    _component(getParam<unsigned int>("component")),
    _output_global_disp(getParam<bool>("output_global_displacement")),
    _periodicity_uo(getUserObject<GlobalStrainPeriodicDirUserObject>("global_strain_uo")),
    _periodic_dir(_periodicity_uo.getPeriodicDirections()),
    _diagonal_components(),
    _off_diagonal_components(),
    _ref_point(getParam<Point>("reference_point")),
    _dim(_mesh.dimension()),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements"))
{
  if (!isNodal())
    paramError("variable", "ADGlobalDisplacementAux must be used on a nodal auxiliary variable.");

  if (_component >= _dim)
    paramError("component", "Component ", _component, " does not exist for a ", _dim, "D problem.");

  if (_ndisp != _dim)
    paramError("displacements",
               "ADGlobalDisplacementAux requires one displacement variable per mesh dimension; ",
               _dim,
               " are required but ",
               _ndisp,
               " were supplied.");

  assignComponentIndices();
  validateScalarVariables();
}

void
ADGlobalDisplacementAux::assignComponentIndices()
{
  _diagonal_components.clear();
  _off_diagonal_components.clear();

  for (unsigned int i = 0; i < _dim; ++i)
    if (_periodic_dir(i))
      _diagonal_components.emplace_back(i, i);

  if (_dim >= 2 && (_periodic_dir(0) || _periodic_dir(1)))
    _off_diagonal_components.emplace_back(0, 1);

  if (_dim >= 3 && (_periodic_dir(0) || _periodic_dir(2)))
    _off_diagonal_components.emplace_back(0, 2);

  if (_dim >= 3 && (_periodic_dir(1) || _periodic_dir(2)))
    _off_diagonal_components.emplace_back(1, 2);
}

void
ADGlobalDisplacementAux::validateScalarVariables() const
{
  if (_number_of_diagonal_components != _diagonal_components.size())
    paramError("diagonal_global_strain",
               "The diagonal scalar variable contains ",
               _number_of_diagonal_components,
               " component(s), but ",
               _diagonal_components.size(),
               " are required.");

  if (_off_diagonal_components.empty())
  {
    if (_off_diagonal_global_strain)
      paramError("off_diagonal_global_strain",
                 "No off-diagonal component is active; omit this variable.");
  }
  else if (!_off_diagonal_global_strain)
    paramError("off_diagonal_global_strain",
               "The periodic directions require an off-diagonal scalar variable with ",
               _off_diagonal_components.size(),
               " component(s).");
  else if (_number_of_off_diagonal_components != _off_diagonal_components.size())
    paramError("off_diagonal_global_strain",
               "The off-diagonal scalar variable contains ",
               _number_of_off_diagonal_components,
               " component(s), but ",
               _off_diagonal_components.size(),
               " are required.");
}

void
ADGlobalDisplacementAux::fillGlobalStrainTensor(RankTwoTensor & strain) const
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
    const Real value = (*_off_diagonal_global_strain)[component];

    strain(indices.first, indices.second) = value;
    strain(indices.second, indices.first) = value;
  }
}

Real
ADGlobalDisplacementAux::computeValue()
{
  RankTwoTensor strain;
  fillGlobalStrainTensor(strain);

  const RealVectorValue global_disp = strain * ((*_current_node) - _ref_point);

  if (_output_global_disp)
    return global_disp(_component);

  return global_disp(_component) + (*_disp[_component])[_qp];
}
