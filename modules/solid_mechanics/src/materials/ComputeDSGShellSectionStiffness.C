//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDSGShellSectionStiffness.h"
#include "ComputeDSGShellStrain.h"

#include <array>

registerMooseObject("SolidMechanicsApp", ComputeDSGShellSectionStiffness);

InputParameters
ComputeDSGShellSectionStiffness::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the linear elastic section stiffness of a DSG shell "
                             "element from the Young's modulus, the Poisson's ratio and the "
                             "thickness.");
  params.addRequiredRangeCheckedParam<Real>(
      "youngs_modulus",
      "youngs_modulus > 0",
      "Young's modulus of the shell material, in the stress unit of the input; the resultants "
      "then come out in force per unit length.");
  params.addRequiredRangeCheckedParam<Real>("poissons_ratio",
                                            "poissons_ratio > -1 & poissons_ratio < 0.5",
                                            "Poisson's ratio of the shell material, between -1 "
                                            "and 0.5.");
  params.addRequiredRangeCheckedParam<Real>(
      "thickness",
      "thickness > 0",
      "Thickness of the shell, in the length unit of the input, the same for every element the "
      "material is defined on.");
  params.addRangeCheckedParam<Real>("shear_correction_factor",
                                    5.0 / 6.0,
                                    "shear_correction_factor > 0",
                                    "Factor on the transverse shear stiffness G * thickness, "
                                    "which accounts for the parabolic distribution of the "
                                    "transverse shear stress through the thickness. The default "
                                    "5/6 is the value of a homogeneous section.");
  params.addRangeCheckedParam<Real>(
      "drilling_stiffness_factor",
      1.0,
      "drilling_stiffness_factor > 0",
      "Factor on the drilling stiffness G * thickness, the penalty that ties the drilling "
      "rotation to the rotation of the in-plane displacement field. The default 1 is the "
      "in-plane shear stiffness of the section; a smaller value softens the penalty on "
      "membrane-dominated problems with coarse or distorted meshes.");
  params.addRangeCheckedParam<Real>(
      "thin_limit_factor",
      0.15,
      "thin_limit_factor >= 0",
      "Constant alpha of the factor t^2 / (t^2 + alpha h^2) that scales the transverse shear "
      "stiffness of the three-node element, with t the thickness and h the longest edge of the "
      "element. It relaxes the shear constraint of that element on meshes coarse relative to the "
      "thickness; the default 0.15 is the value of Lyly, Stenberg and Vihinen (1993). It has no "
      "effect on the other members of the family, and zero turns it off.");
  return params;
}

ComputeDSGShellSectionStiffness::ComputeDSGShellSectionStiffness(const InputParameters & parameters)
  : Material(parameters),
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _thickness(getParam<Real>("thickness")),
    _shear_correction_factor(getParam<Real>("shear_correction_factor")),
    _drilling_stiffness_factor(getParam<Real>("drilling_stiffness_factor")),
    _thin_limit_factor(getParam<Real>("thin_limit_factor")),
    _longest_edge(getMaterialProperty<Real>("dsg_shell_longest_edge")),
    _section_stiffness(declareProperty<DenseMatrix<Real>>("dsg_shell_section_stiffness")),
    _thickness_property(declareProperty<Real>("dsg_shell_thickness"))
{
}

void
ComputeDSGShellSectionStiffness::computeQpProperties()
{
  const Real E = _youngs_modulus;
  const Real nu = _poissons_ratio;
  const Real t = _thickness;
  const Real G = E / (2.0 * (1.0 + nu));

  // The plane stress matrix of the isotropic material
  std::array<std::array<Real, 3>, 3> plane_stress = {
      {{{1.0, nu, 0.0}}, {{nu, 1.0, 0.0}}, {{0.0, 0.0, 0.5 * (1.0 - nu)}}}};
  for (unsigned int row = 0; row < 3; ++row)
    for (unsigned int column = 0; column < 3; ++column)
      plane_stress[row][column] /= (1.0 - nu * nu);

  DenseMatrix<Real> & D = _section_stiffness[_qp];
  D.resize(DSGShell::n_strains, DSGShell::n_strains);

  // Membrane block: E t times the plane stress matrix
  // Bending block: E t^3 / 12 times the plane stress matrix
  for (unsigned int row = 0; row < 3; ++row)
    for (unsigned int column = 0; column < 3; ++column)
    {
      D(row, column) = E * t * plane_stress[row][column];
      D(3 + row, 3 + column) = E * t * t * t / 12.0 * plane_stress[row][column];
    }

  // Transverse shear block: the corrected shear stiffness, with the thin-limit factor on the
  // three-node element, whose single linear gap field would otherwise lock in thin plates
  Real shear_stiffness = _shear_correction_factor * G * t;
  if (_current_elem->n_nodes() == 3)
  {
    const Real h = _longest_edge[_qp];
    shear_stiffness *= t * t / (t * t + _thin_limit_factor * h * h);
  }
  D(6, 6) = shear_stiffness;
  D(7, 7) = shear_stiffness;

  // Drilling stiffness: the in-plane shear stiffness G t, the consistent penalty on the
  // drilling strain, times the user's factor
  D(8, 8) = _drilling_stiffness_factor * G * t;

  _thickness_property[_qp] = t;
}
