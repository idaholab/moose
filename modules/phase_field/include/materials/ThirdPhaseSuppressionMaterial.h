/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
