//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DSGShellMassEigenKernel.h"
#include "MooseUtils.h"

registerMooseObject("SolidMechanicsApp", DSGShellMassEigenKernel);

InputParameters
DSGShellMassEigenKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Provides the mass matrix of one degree of freedom of a DSG shell element for an "
      "eigenvalue solve, with the sign convention of MassEigenKernel. Six kernels are needed, "
      "one per variable, each tagged with extra_vector_tags = 'eigen'.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 6",
      "The degree of freedom this kernel assembles: 0, 1 and 2 for the displacements along the "
      "global axes X, Y and Z, 3, 4 and 5 for the rotations about them. Six kernels are needed, "
      "one per variable, and the variable of each must be the one listed at that position of "
      "'displacements' and 'rotations'.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables of the shell nodes, in the order of the global axes X, Y "
      "and Z, for example 'disp_x disp_y disp_z'. Every object of the DSG shell family takes the "
      "same list.");
  params.addRequiredCoupledVar(
      "rotations",
      "The three rotation variables of the shell nodes, the rotations about the global axes X, Y "
      "and Z in that order, for example 'rot_x rot_y rot_z'. The rotation about the normal of an "
      "element is its drilling rotation.");
  params.addParam<MaterialPropertyName>(
      "density",
      "density",
      "Name of the material property that gives the mass density of the shell material, a mass "
      "per unit volume (the mass per unit area of the shell is the density times the "
      "thickness). The property can be declared with GenericConstantMaterial.");
  params.addParam<bool>(
      "rotary_inertia",
      true,
      "Whether the two rotations in the plane of each element carry the rotary inertia of the "
      "thickness, density * thickness^3 / 12 per unit area. Set to false to keep the "
      "translational mass only, which raises the frequencies slightly (0.1 percent on the first "
      "mode of a plate whose thickness is a fifteenth of its length). The drilling rotation "
      "carries no inertia either way.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

DSGShellMassEigenKernel::DSGShellMassEigenKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _rotary_inertia(getParam<bool>("rotary_inertia")),
    _density(getMaterialProperty<Real>("density")),
    _thickness(getMaterialProperty<Real>("dsg_shell_thickness")),
    _rotation_matrix(getMaterialProperty<RankTwoTensor>("dsg_shell_rotation_matrix"))
{
  if (coupledComponents("displacements") != 3 || coupledComponents("rotations") != 3)
    paramError("displacements",
               "'displacements' must list three variables and 'rotations' must list three "
               "variables.");

  for (const char * group : {"displacements", "rotations"})
    for (unsigned int i = 0; i < 3; ++i)
    {
      _variable_numbers.push_back(coupled(group, i));
      _values.push_back(&coupledValue(group, i));
    }

  // Without the eigen vector tag the kernel would add -M u to the residual of a static solve
  bool has_eigen_tag = false;
  for (const auto & tag : getParam<std::vector<TagName>>("extra_vector_tags"))
    if (MooseUtils::toUpper(tag) == "EIGEN")
      has_eigen_tag = true;
  if (!has_eigen_tag)
    paramError("extra_vector_tags",
               "This kernel provides the mass matrix of an eigenvalue problem and must carry the "
               "'eigen' vector tag: extra_vector_tags = 'eigen'.");

  if (_variable_numbers[_component] != _var.number())
    paramError("component",
               "The variable of this kernel is not the one listed for component ",
               _component,
               " in 'displacements' and 'rotations'.");
}

Real
DSGShellMassEigenKernel::inPlaneProjector(const unsigned int axis_a,
                                          const unsigned int axis_b) const
{
  // The identity minus the outer product of the element normal, the third row of the rotation
  // matrix: it keeps the rotations about the two axes in the plane of the element
  const RankTwoTensor & rotation = _rotation_matrix[_qp];
  const Real identity = (axis_a == axis_b) ? 1.0 : 0.0;
  return identity - rotation(2, axis_a) * rotation(2, axis_b);
}

Real
DSGShellMassEigenKernel::massPerArea() const
{
  const Real thickness = _thickness[_qp];
  if (_component < 3)
    return _density[_qp] * thickness;
  return _density[_qp] * thickness * thickness * thickness / 12.0;
}

Real
DSGShellMassEigenKernel::computeQpResidual()
{
  // The eigenvalue solvers of the framework assemble the mass from a residual written with a
  // minus sign, as MassEigenKernel does
  if (_component < 3)
    return -_test[_i][_qp] * massPerArea() * _u[_qp];

  if (!_rotary_inertia)
    return 0.0;

  const unsigned int axis = _component - 3;
  Real projected_rotation = 0.0;
  for (unsigned int other_axis = 0; other_axis < 3; ++other_axis)
    projected_rotation += inPlaneProjector(axis, other_axis) * (*_values[3 + other_axis])[_qp];
  return -_test[_i][_qp] * massPerArea() * projected_rotation;
}

Real
DSGShellMassEigenKernel::computeQpJacobian()
{
  if (_component < 3)
    return -_test[_i][_qp] * _phi[_j][_qp] * massPerArea();

  if (!_rotary_inertia)
    return 0.0;

  const unsigned int axis = _component - 3;
  return -_test[_i][_qp] * _phi[_j][_qp] * massPerArea() * inPlaneProjector(axis, axis);
}

Real
DSGShellMassEigenKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Only the rotations couple with each other, through the in-plane projector
  if (_component < 3 || !_rotary_inertia)
    return 0.0;

  for (unsigned int other_axis = 0; other_axis < 3; ++other_axis)
  {
    const unsigned int other_component = 3 + other_axis;
    if (_variable_numbers[other_component] == jvar && other_component != _component)
      return -_test[_i][_qp] * _phi[_j][_qp] * massPerArea() *
             inPlaneProjector(_component - 3, other_axis);
  }
  return 0.0;
}
