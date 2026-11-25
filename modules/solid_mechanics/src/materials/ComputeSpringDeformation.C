//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSpringDeformation.h"

#include "MooseMesh.h"

registerMooseObject("SolidMechanicsApp", ComputeSpringDeformation);

InputParameters
ComputeSpringDeformation::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the local frame of a two-node spring element and the "
                             "translation and rotation of its second node relative to its first "
                             "node in that frame.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables, in the order x, y, z (the family is written for three-"
      "dimensional models; a plane model holds its third displacement). The deformation of the "
      "spring is "
      "the displacement of node 1 minus the displacement of node 0, expressed in the local frame; "
      "a positive first component is an elongation along local axis 1.");
  params.addCoupledVar(
      "rotations",
      "The three rotation variables, in the order x, y, z. Give them when a rotational spring law "
      "acts on the element: the relative rotation of node 1 to node 0 is then reported in the "
      "local frame.");
  params.addParam<RealVectorValue>(
      "local_axis_1",
      "Direction of local axis 1 of the spring in global components, any length. Without it "
      "the axis runs from node 0 to node 1. This is the axial direction of a translational "
      "law and the normal direction of the Mohr-Coulomb law. A spring between coincident "
      "positions is modeled as a short element (the mesh library rejects an element of zero "
      "length), and this parameter then sets its axis, since the length enters no law.");
  params.addParam<RealVectorValue>(
      "local_axis_2",
      "Direction whose projection on the plane normal to axis 1 is local axis 2, any length. "
      "Without it the global Y axis is projected; when axis 1 is along Y, the global axis least "
      "aligned with it is projected instead. Axis 3 completes the right-handed frame.");
  params.addParam<std::string>(
      "base_name",
      "Prefix of the property names (base_name_spring_deformation and the others), to put "
      "several springs with their own laws and kernels on one block, a spring in parallel with "
      "a dashpot say. The laws and the kernels of that spring take the same base_name.");
  params.addParam<bool>("compute_rates",
                        false,
                        "Also report the rates of the deformation and of the rotation from the "
                        "time derivative of the solution, for the viscous law. Requires a "
                        "transient executioner.");
  return params;
}

ComputeSpringDeformation::ComputeSpringDeformation(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _nrot(coupledComponents("rotations")),
    _compute_rates(getParam<bool>("compute_rates")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _has_local_axis_1(isParamValid("local_axis_1")),
    _has_local_axis_2(isParamValid("local_axis_2")),
    _local_axis_1(_has_local_axis_1 ? getParam<RealVectorValue>("local_axis_1")
                                    : RealVectorValue(0, 0, 0)),
    _local_axis_2(_has_local_axis_2 ? getParam<RealVectorValue>("local_axis_2")
                                    : RealVectorValue(0, 0, 0)),
    _rotation_matrix(declareProperty<RankTwoTensor>(_base_name + "spring_rotation_matrix")),
    _deformation(declareProperty<RealVectorValue>(_base_name + "spring_deformation")),
    _rotation(_nrot == 3 ? &declareProperty<RealVectorValue>(_base_name + "spring_rotation")
                         : nullptr),
    _deformation_rate(
        _compute_rates ? &declareProperty<RealVectorValue>(_base_name + "spring_deformation_rate")
                       : nullptr),
    _rotation_rate(_compute_rates && _nrot == 3
                       ? &declareProperty<RealVectorValue>(_base_name + "spring_rotation_rate")
                       : nullptr),
    _rate_du_dot_du(_compute_rates ? &declareProperty<Real>(_base_name + "spring_rate_du_dot_du")
                                   : nullptr)
{
  if (_ndisp != 3)
    paramError("displacements", "Three displacement variables are needed, one per global axis.");
  if (_nrot != 0 && _nrot != 3)
    paramError("rotations", "Three rotation variables are needed, one per global axis.");
  if (_has_local_axis_1 && MooseUtils::absoluteFuzzyEqual(_local_axis_1.norm(), 0.0))
    paramError("local_axis_1", "The direction must not be zero.");
  if (_has_local_axis_2 && MooseUtils::absoluteFuzzyEqual(_local_axis_2.norm(), 0.0))
    paramError("local_axis_2", "The direction must not be zero.");
  if (_compute_rates && !_fe_problem.isTransient())
    paramError("compute_rates", "The rates need a transient executioner.");
  if (_has_local_axis_1 && _has_local_axis_2)
  {
    const RealVectorValue axis_1 = _local_axis_1 / _local_axis_1.norm();
    const RealVectorValue projection = _local_axis_2 - (_local_axis_2 * axis_1) * axis_1;
    if (projection.norm() < _local_axis_2.norm() * std::sin(0.05 * libMesh::pi / 180.0))
      paramError("local_axis_2", "The direction is parallel to local axis 1.");
  }

  checkElementLengths();

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_dof_values.push_back(&coupledDofValues("displacements", i));
    _disp_vars.push_back(getVar("displacements", i));
  }
  for (unsigned int i = 0; i < _nrot; ++i)
  {
    _rot_dof_values.push_back(&coupledDofValues("rotations", i));
    _rot_vars.push_back(getVar("rotations", i));
  }

  // The time derivatives of the nodal values are requested once here so that the variables keep
  // them up to date on every element.
  if (_compute_rates)
  {
    for (const auto var : _disp_vars)
    {
      var->dofValuesDot();
      var->dofValuesDuDotDu();
    }
    for (const auto var : _rot_vars)
      var->dofValuesDot();
  }
}

void
ComputeSpringDeformation::checkElementLengths() const
{
  for (const auto elem : _mesh.getMesh().active_local_element_ptr_range())
    if (hasBlocks(elem->subdomain_id()) && elem->n_nodes() == 2 &&
        MooseUtils::absoluteFuzzyEqual((elem->point(1) - elem->point(0)).norm(), 0.0))
      mooseError("Spring element ",
                 elem->id(),
                 " has zero length, which the finite element map of the mesh cannot take: give it "
                 "a short length and set local_axis_1 (the length enters no spring law).");
}

RankTwoTensor
ComputeSpringDeformation::computeRotationMatrix() const
{
  const Point & node_0 = _current_elem->point(0);
  const Point & node_1 = _current_elem->point(1);
  const RealVectorValue element_direction = node_1 - node_0;

  RealVectorValue axis_1 = _has_local_axis_1 ? _local_axis_1 : element_direction;
  axis_1 /= axis_1.norm();

  // Axis 2 is the projection of the reference direction on the plane normal to axis 1. A
  // reference direction along axis 1 (within 0.05 degrees) has no projection; a given
  // local_axis_2 is then an input error, the default falls back to the global axis least
  // aligned with axis 1.
  RealVectorValue reference = _has_local_axis_2 ? _local_axis_2 : RealVectorValue(0, 1, 0);
  RealVectorValue axis_2 = reference - (reference * axis_1) * axis_1;
  const Real parallel_tolerance = std::sin(0.05 * libMesh::pi / 180.0);
  if (axis_2.norm() < reference.norm() * parallel_tolerance)
  {
    if (_has_local_axis_2)
      paramError("local_axis_2",
                 "The direction is parallel to local axis 1 of element ",
                 _current_elem->id(),
                 ".");
    unsigned int least_aligned_axis = 0;
    for (unsigned int axis = 1; axis < 3; ++axis)
      if (std::abs(axis_1(axis)) < std::abs(axis_1(least_aligned_axis)))
        least_aligned_axis = axis;
    reference = RealVectorValue(0, 0, 0);
    reference(least_aligned_axis) = 1.0;
    axis_2 = reference - (reference * axis_1) * axis_1;
  }
  axis_2 /= axis_2.norm();

  const RealVectorValue axis_3 = axis_1.cross(axis_2);
  return RankTwoTensor::initializeFromRows(axis_1, axis_2, axis_3);
}

void
ComputeSpringDeformation::computeProperties()
{
  if (_current_elem->n_nodes() != 2)
    mooseError("Element ", _current_elem->id(), " is not a two-node element.");
  if (_current_elem->level() != 0)
    mooseError("Element ",
               _current_elem->id(),
               " is a refined spring element: a spring split by mesh adaptivity would be two "
               "springs in series. Exclude the spring blocks from adaptivity.");

  const RankTwoTensor rotation_matrix = computeRotationMatrix();

  // Relative motion of node 1 with respect to node 0, in global components first.
  RealVectorValue relative_displacement, relative_rotation, relative_velocity,
      relative_rotation_rate;
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    const VariableValue & values = *_disp_dof_values[i];
    relative_displacement(i) = values[1] - values[0];
    if (_compute_rates)
    {
      const auto & rates = _disp_vars[i]->dofValuesDot();
      relative_velocity(i) = rates[1] - rates[0];
    }
  }
  for (unsigned int i = 0; i < _nrot; ++i)
  {
    const VariableValue & values = *_rot_dof_values[i];
    relative_rotation(i) = values[1] - values[0];
    if (_compute_rates)
    {
      const auto & rates = _rot_vars[i]->dofValuesDot();
      relative_rotation_rate(i) = rates[1] - rates[0];
    }
  }

  const RealVectorValue deformation = rotation_matrix * relative_displacement;
  const RealVectorValue rotation = rotation_matrix * relative_rotation;
  const RealVectorValue deformation_rate = rotation_matrix * relative_velocity;
  const RealVectorValue rotation_rate = rotation_matrix * relative_rotation_rate;
  const Real du_dot_du = _compute_rates ? _disp_vars[0]->dofValuesDuDotDu()[0] : 0.0;

  // The values are constant over the element; every quadrature point carries the same ones.
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _rotation_matrix[_qp] = rotation_matrix;
    _deformation[_qp] = deformation;
    if (_rotation)
      (*_rotation)[_qp] = rotation;
    if (_compute_rates)
    {
      (*_deformation_rate)[_qp] = deformation_rate;
      if (_rotation_rate)
        (*_rotation_rate)[_qp] = rotation_rate;
      (*_rate_du_dot_du)[_qp] = du_dot_du;
    }
  }
}
