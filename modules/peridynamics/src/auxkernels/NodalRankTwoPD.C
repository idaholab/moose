//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalRankTwoPD.h"
#include "PeridynamicsMesh.h"
#include "RankTwoScalarTools.h"

registerMooseObject("PeridynamicsApp", NodalRankTwoPD);

template <>
InputParameters
validParams<NodalRankTwoPD>()
{
  InputParameters params = validParams<AuxKernelBasePD>();
  params.addClassDescription(
      "Class for computing and outputing components and scalar quantities "
      "of nodal rank two strain and stress tensors for bond-based and ordinary "
      "state-based peridynamic models");

  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable names for the displacements");
  params.addParam<VariableName>("temperature", "Nonlinear variable name for the temperature");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");
  params.addParam<bool>("plane_stress", false, "Plane stress problem or not");
  params.addParam<Real>("youngs_modulus", 0.0, "Material constant: Young's modulus");
  params.addParam<Real>("poissons_ratio", 0.0, "Material constant: Poisson's ratio");
  params.addParam<Real>(
      "thermal_expansion_coeff", 0.0, "Value of material thermal expansion coefficient");
  params.addParam<Real>("stress_free_temperature", 0.0, "Stress free temperature");
  params.addRequiredParam<std::string>(
      "rank_two_tensor",
      "Parameter to set which rank two tensor: total_strain, mechanical_strain or stress");
  params.addRequiredParam<std::string>("output_type", "Type of output: component or scalar");
  params.addParam<MooseEnum>(
      "scalar_type", RankTwoScalarTools::scalarOptions(), "Type of scalar output");
  params.addParam<unsigned int>(
      "index_i", 0, "The index i of ij for the tensor to output (0, 1, 2)");
  params.addParam<unsigned int>(
      "index_j", 0, "The index j of ij for the tensor to output (0, 1, 2)");
  params.addParam<Point>("point1",
                         Point(0, 0, 0),
                         "Start point for axis used to calculate some direction dependent material "
                         "tensor scalar quantities");
  params.addParam<Point>("point2",
                         Point(1, 0, 0),
                         "End point for axis used to calculate some direction dependent material "
                         "tensor scalar quantities");

  return params;
}

NodalRankTwoPD::NodalRankTwoPD(const InputParameters & parameters)
  : AuxKernelBasePD(parameters),
    _has_temp(isParamValid("temperature")),
    _temp_var(_has_temp
                  ? &_subproblem.getStandardVariable(_tid, getParam<VariableName>("temperature"))
                  : NULL),
    _bond_status_var(_subproblem.getStandardVariable(_tid, "bond_status")),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _plane_stress(getParam<bool>("plane_stress")),
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _alpha(getParam<Real>("thermal_expansion_coeff")),
    _temp_ref(getParam<Real>("stress_free_temperature")),
    _rank_two_tensor(getParam<std::string>("rank_two_tensor")),
    _output_type(getParam<std::string>("output_type")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2"))
{
  if (!_var.isNodal())
    mooseError("NodalRankTwoPD operates on nodal variable!");

  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));
  if (nl_vnames.size() != _dim)
    mooseError("Number of displacements should be the same as problem dimension!");
  for (unsigned int i = 0; i < _dim; ++i)
    _disp_var.push_back(&_subproblem.getStandardVariable(_tid, nl_vnames[i]));

  if (_rank_two_tensor == "stress" && !isParamValid("youngs_modulus") &&
      !isParamValid("poissons_ratio"))
    mooseError("Both Young's modulus and Poisson's ratio must be provided for stress calculation");

  std::vector<Real> iso_const(2);
  iso_const[0] =
      _youngs_modulus * _poissons_ratio / ((1.0 + _poissons_ratio) * (1.0 - 2.0 * _poissons_ratio));
  iso_const[1] = _youngs_modulus / (2.0 * (1.0 + _poissons_ratio));

  // fill elasticity tensor
  _Cijkl.fillFromInputVector(iso_const, RankFourTensor::symmetric_isotropic);

  if (_has_temp && !isParamValid("thermal_expansion_coeff"))
    mooseError("thermal_expansion_coeff is required when temperature is coupled");

  if (_output_type == "component" && !(isParamValid("index_i") || isParamValid("index_j")))
    mooseError("The output_type is 'component', but no 'index_i' and 'index_j' are provided!");

  if (_output_type == "scalar" && !isParamValid("scalar_type"))
    mooseError("The output_type is 'scalar', but no 'scalar_type' is provided!");
}

Real
NodalRankTwoPD::computeValue()
{
  Real val = 0.0;
  RankTwoTensor tensor;
  Point p = Point(0, 0, 0);
  if (_rank_two_tensor == "total_strain")
    tensor = computeNodalTotalStrain();
  else if (_rank_two_tensor == "mechanical_strain")
    tensor = computeNodalMechanicalStrain();
  else if (_rank_two_tensor == "stress")
    tensor = computeNodalStress();
  else
    mooseError("NodalRankTwoPD Error: Pass valid rank two tensor: total_strain, "
               "mechanical_strain or stress");

  if (_output_type == "component")
    val = RankTwoScalarTools::component(tensor, _i, _j);
  else if (_output_type == "scalar")
    val = RankTwoScalarTools::getQuantity(tensor, _scalar_type, _point1, _point2, _q_point[_qp], p);
  else
    mooseError("NodalRankTwoPD Error: Pass valid output type - component or scalar");

  return val;
}

RankTwoTensor
NodalRankTwoPD::computeNodalTotalStrain()
{
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_node->id());
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_node->id());
  Real horizon = _pdmesh.getHorizon(_current_node->id());

  // calculate the shape tensor and prepare the deformation gradient tensor
  RankTwoTensor shape, dgrad, delta(RankTwoTensor::initIdentity);
  shape.zero();
  dgrad.zero();
  if (_dim == 2)
    shape(2, 2) = dgrad(2, 2) = 1.0;

  RealGradient origin_vector(3), current_vector(3);
  origin_vector = 0.0;
  current_vector = 0.0;

  for (unsigned int j = 0; j < neighbors.size(); ++j)
  {
    Node * node_j = _pdmesh.nodePtr(neighbors[j]);
    Real vol_j = _pdmesh.getPDNodeVolume(neighbors[j]);
    for (unsigned int k = 0; k < _dim; ++k)
    {
      origin_vector(k) = (*node_j)(k) - (*_pdmesh.nodePtr(_current_node->id()))(k);
      current_vector(k) = origin_vector(k) + _disp_var[k]->getNodalValue(*node_j) -
                          _disp_var[k]->getNodalValue(*_current_node);
    }
    Real origin_length = origin_vector.norm();
    Real bond_status_ij = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[j]));
    for (unsigned int k = 0; k < _dim; ++k)
      for (unsigned int l = 0; l < _dim; ++l)
      {
        shape(k, l) +=
            vol_j * horizon / origin_length * origin_vector(k) * origin_vector(l) * bond_status_ij;
        dgrad(k, l) +=
            vol_j * horizon / origin_length * current_vector(k) * origin_vector(l) * bond_status_ij;
      }
  }

  // finalize the deformation gradient tensor
  dgrad *= shape.inverse();

  // the green-lagrange strain tensor
  RankTwoTensor total_strain = 0.5 * (dgrad.transpose() * dgrad - delta);

  if (_scalar_out_of_plane_strain_coupled)
    total_strain(2, 2) = _scalar_out_of_plane_strain[0];
  else if (_plane_stress)
  {
    if (_has_temp)
    {
      Real mstrain00 =
          (total_strain(0, 0) - _alpha * (_temp_var->getNodalValue(*_current_node) - _temp_ref));
      Real mstrain11 =
          (total_strain(1, 1) - _alpha * (_temp_var->getNodalValue(*_current_node) - _temp_ref));
      Real mstrain22 =
          -(_Cijkl(2, 2, 0, 0) * mstrain00 + _Cijkl(2, 2, 1, 1) * mstrain11) / _Cijkl(2, 2, 2, 2);
      total_strain(2, 2) =
          mstrain22 + _alpha * (_temp_var->getNodalValue(*_current_node) - _temp_ref);
    }
    else
      total_strain(2, 2) =
          -(_Cijkl(2, 2, 0, 0) * total_strain(0, 0) + _Cijkl(2, 2, 1, 1) * total_strain(1, 1)) /
          _Cijkl(2, 2, 2, 2);
  }

  return total_strain;
}

RankTwoTensor
NodalRankTwoPD::computeNodalMechanicalStrain()
{
  RankTwoTensor total_strain = computeNodalTotalStrain();
  RankTwoTensor delta(RankTwoTensor::initIdentity);
  RankTwoTensor thermal_strain;

  if (_has_temp)
    thermal_strain = _alpha * (_temp_var->getNodalValue(*_current_node) - _temp_ref) * delta;

  RankTwoTensor mechanical_strain = total_strain - thermal_strain;

  return mechanical_strain;
}

RankTwoTensor
NodalRankTwoPD::computeNodalStress()
{
  return _Cijkl * computeNodalMechanicalStrain();
}
