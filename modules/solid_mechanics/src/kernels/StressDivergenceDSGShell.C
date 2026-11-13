//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceDSGShell.h"
#include "ComputeDSGShellStrain.h"

registerMooseObject("SolidMechanicsApp", StressDivergenceDSGShell);

InputParameters
StressDivergenceDSGShell::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Quasi-static stress divergence kernel for one degree of freedom of "
                             "a DSG shell element.");
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
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

StressDivergenceDSGShell::StressDivergenceDSGShell(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _strain_displacement_matrix(
        getMaterialProperty<DenseMatrix<Real>>("dsg_shell_strain_displacement_matrix")),
    _resultants(getMaterialProperty<DenseVector<Real>>("dsg_shell_resultants")),
    _section_stiffness(getMaterialProperty<DenseMatrix<Real>>("dsg_shell_section_stiffness"))
{
  if (coupledComponents("displacements") != 3 || coupledComponents("rotations") != 3)
    paramError("displacements",
               "'displacements' must list three variables and "
               "'rotations' must list three variables.");

  for (unsigned int i = 0; i < 3; ++i)
    _variable_numbers.push_back(coupled("displacements", i));
  for (unsigned int i = 0; i < 3; ++i)
    _variable_numbers.push_back(coupled("rotations", i));

  if (_variable_numbers[_component] != _var.number())
    paramError("component",
               "The variable of this kernel is not the one listed for component ",
               _component,
               " in 'displacements' and 'rotations'.");
}

Real
StressDivergenceDSGShell::computeQpResidual()
{
  // The internal force on the degree of freedom of this test function is the column of the
  // strain-displacement matrix for that degree of freedom times the stress resultants. The
  // columns are ordered node by node, six per node (ComputeDSGShellStrain), and for Lagrange
  // variables the test function index is the node index.
  const unsigned int column = DSGShell::dofs_per_node * _i + _component;
  const DenseMatrix<Real> & B = _strain_displacement_matrix[_qp];
  const DenseVector<Real> & resultants = _resultants[_qp];

  Real residual = 0.0;
  for (unsigned int strain = 0; strain < DSGShell::n_strains; ++strain)
    residual += B(strain, column) * resultants(strain);
  return residual;
}

Real
StressDivergenceDSGShell::computeQpJacobian()
{
  return stiffnessEntry(_component);
}

Real
StressDivergenceDSGShell::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int component = 0; component < 6; ++component)
    if (_variable_numbers[component] == jvar)
      return stiffnessEntry(component);
  return 0.0;
}

Real
StressDivergenceDSGShell::stiffnessEntry(const unsigned int other_component) const
{
  // Row and column of the element stiffness for the test node and the trial node; the
  // product D B of this quadrature point was formed once before the loops over the nodes
  const unsigned int row = DSGShell::dofs_per_node * _i + _component;
  const unsigned int column = DSGShell::dofs_per_node * _j + other_component;
  const DenseMatrix<Real> & B = _strain_displacement_matrix[_qp];
  const DenseMatrix<Real> & DB = _stiffness_times_matrix[_qp];

  Real entry = 0.0;
  for (unsigned int strain = 0; strain < DSGShell::n_strains; ++strain)
    entry += B(strain, row) * DB(strain, column);
  return entry;
}

void
StressDivergenceDSGShell::precalculateJacobian()
{
  // The product of the section stiffness with the strain-displacement matrix, once per
  // quadrature point of the element, shared by the diagonal and the off-diagonal blocks
  _stiffness_times_matrix.resize(_qrule->n_points());
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const DenseMatrix<Real> & B = _strain_displacement_matrix[qp];
    const DenseMatrix<Real> & D = _section_stiffness[qp];
    DenseMatrix<Real> & DB = _stiffness_times_matrix[qp];
    DB = D;
    DB.right_multiply(B);
  }
}

void
StressDivergenceDSGShell::precalculateOffDiagJacobian(unsigned int /*jvar*/)
{
  precalculateJacobian();
}
