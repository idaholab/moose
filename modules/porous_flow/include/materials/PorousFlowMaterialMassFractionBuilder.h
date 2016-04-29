/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PORFLOWMATERIALMASSFRACTIONBUILDER_H
#define PORFLOWMATERIALMASSFRACTIONBUILDER_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialMassFractionBuilder;

template<>
InputParameters validParams<PorousFlowMaterialMassFractionBuilder>();

/**
 * Material designed to form a std::vector<std::vector>
 * of mass fractions from the individual mass fraction variables
 */
class PorousFlowMaterialMassFractionBuilder : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialMassFractionBuilder(const InputParameters & parameters);

protected:
  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;

  /// Number of fluid phases
  unsigned int _num_phases;

  /// Number of fluid components
  unsigned int _num_components;

  /// Mass fraction matrix
  MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;

  /// Old value of mass fraction matrix
  MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac_old;

  /// Gradient of the mass fraction matrix at the quad points
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _grad_mass_frac;

  /// Derivative of the mass fraction matrix with respect to the porous flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_frac_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  /**
   * Number of mass-fraction variables provided by the user
   * This needs to be _num_phases*(_num_components - 1), since the
   * mass fraction of the final component in each phase is
   * determined as 1 - sum_{components}(mass fraction of all other components in the phase)
   */
  const unsigned int _num_passed_mf_vars;

  /// the variable number of the mass-fraction variables
  std::vector<unsigned int> _mf_vars_num;

  /// the mass-fraction variables
  std::vector<const VariableValue *> _mf_vars;

  /// the gradient of the mass-fraction variables
  std::vector<const VariableGradient *> _grad_mf_vars;

  /**
   * Builds the mass-fraction variable matrix at the quad point
   * @param qp the quad point
   */
  void build_mass_frac(unsigned int qp);
};

#endif //PORFLOWMATERIALMASSFRACTIONBUILDER_H
