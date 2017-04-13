/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMASSFRACTION_H
#define POROUSFLOWMASSFRACTION_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowMassFraction;

template <>
InputParameters validParams<PorousFlowMassFraction>();

/**
 * Material designed to form a std::vector<std::vector>
 * of mass fractions from the individual mass fraction variables
 */
class PorousFlowMassFraction : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowMassFraction(const InputParameters & parameters);

protected:
  /// Mass fraction matrix at quadpoint or nodes
  MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;

  /// Gradient of the mass fraction matrix at the quad points
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _grad_mass_frac;

  /// Derivative of the mass fraction matrix with respect to the porous flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Builds the mass-fraction variable matrix at the quad point
   * @param qp the quad point
   */
  void build_mass_frac(unsigned int qp);

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
};

#endif // POROUSFLOWMASSFRACTION_H
