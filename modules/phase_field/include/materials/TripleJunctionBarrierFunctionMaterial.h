//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * TripleJunctionBarrierFunctionMaterial provides an obstacle-type free energy contribution
 * that suppresses the spurious nucleation of a fourth phase at triple junctions, as described
 * by Kundin, Pogorelov, and Emmerich, Acta Mater., v. 83, p. 448-459 (2015):
 * \f$ f_{obs} = \sum_{i<j<k} h_{ijk} \eta_i^2 \eta_j^2 \eta_k^2 \f$, summed over all distinct
 * unordered triples of the coupled order parameters. The coefficients \f$h_{ijk}\f$ may be
 * supplied either as a single uniform value or individually per triple.
 */
class TripleJunctionBarrierFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  TripleJunctionBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// name of the function (used to generate the material property names)
  std::string _function_name;

  /// order parameters
  unsigned int _num_eta;
  const std::vector<VariableName> _eta_names;
  const std::vector<const VariableValue *> _eta;

  /// uniform obstacle coefficient applied to every triple when h_ijk is not supplied
  const Real _h;

  /// per-triple obstacle coefficients in lexicographic i<j<k order (overrides _h if non-empty)
  const std::vector<Real> _h_ijk;

  ///@{ Obstacle function and its derivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg;
  std::vector<std::vector<MaterialProperty<Real> *>> _prop_d2g;
  ///@}
};
