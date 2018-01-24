//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Original class author: A.M. Jokisaari
// O. Heinonen, et al. at ANL also have contributed significantly - thanks guys!

#ifndef LINEARGENERALANISOTROPICMATERIAL_H
#define LINEARGENERALANISOTROPICMATERIAL_H

#include "SolidMechanicsMaterial.h"
#include "SymmTensor.h"
#include "SymmAnisotropicElasticityTensor.h"

/**
 * LinearGeneralAnisotropicMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */

// Forward declaration
class LinearGeneralAnisotropicMaterial;

template <>
InputParameters validParams<LinearGeneralAnisotropicMaterial>();

class LinearGeneralAnisotropicMaterial : public SolidMechanicsMaterial
{
public:
  LinearGeneralAnisotropicMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  virtual void computeQpElasticityTensor();

  virtual void computeQpStrain();

  virtual void computeQpStress();

private:
  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;

  // vectors to get the input values
  std::vector<Real> _Cijkl_matrix_vector;

  // bool to indicate if using 9 stiffness values or all 21
  bool _all_21;

  // Individual material information
  SymmAnisotropicElasticityTensor _Cijkl_matrix;
};

#endif // LINEARGENERALANISOTROPICMATERIAL_H
