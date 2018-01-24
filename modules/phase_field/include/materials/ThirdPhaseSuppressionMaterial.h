//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THIRDPHASESUPPRESSIONMATERIAL_H
#define THIRDPHASESUPPRESSIONMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class ThirdPhaseSuppressionMaterial;

template <>
InputParameters validParams<ThirdPhaseSuppressionMaterial>();

/**
 * OPInterfaceBarrierMaterial is a Free Energy Penalty contribution
 * material that acts on all of the eta_i variables to
 * prevent more than two eta variables going above 0 on an interface.
 */
class ThirdPhaseSuppressionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  ThirdPhaseSuppressionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// name of the function of eta (used to generate the material property names)
  std::string _function_name;

  /// order parameters
  unsigned int _num_eta;
  std::vector<const VariableValue *> _eta;

  /// Barrier functions and their drivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg;

  /// Material properties to store the second derivatives.
  std::vector<std::vector<MaterialProperty<Real> *>> _prop_d2g;
};

#endif // THIRDPHASESUPPRESSIONMATERIAL_H
