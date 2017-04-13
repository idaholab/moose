/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDICTATOR_H
#define POROUSFLOWDICTATOR_H

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "ZeroInterface.h"

class PorousFlowDictator;

template <>
InputParameters validParams<PorousFlowDictator>();

/**
 * This holds maps between the nonlinear variables
 * used in a PorousFlow simulation and the
 * variable number used internally by MOOSE, as
 * well as the number of fluid phases and
 * the number of fluid components.
 *
 * All PorousFlow Materials and Kernels calculate
 * and use derivatives with respect to all the variables
 * mentioned in this Object, at least in principal
 * (in practice they may be lazy and not compute
 * all derivatives).
 */
class PorousFlowDictator : public GeneralUserObject, public Coupleable, public ZeroInterface
{
public:
  PorousFlowDictator(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  /**
   * The number of PorousFlow variables.  Materials
   * and Kernels will calculate and use derivatives
   * with respect to these variables in the Jacobian
   */
  unsigned int numVariables() const;

  /// the number of fluid phases
  unsigned int numPhases() const;

  /// the number of fluid components
  unsigned int numComponents() const;

  /**
   * the PorousFlow variable number
   * @param moose_var_num the MOOSE variable number
   * eg if porous_flow_vars = 'pwater pgas', and the variables in
   * the simulation are 'energy pwater pgas shape'
   * then porousFlowVariableNum(2) = 1
   */
  unsigned int porousFlowVariableNum(unsigned int moose_var_num) const;

  /**
   * returns true if moose_var_num is a porous flow variable
   * @param moose_var_num the MOOSE variable number
   * eg if porous_flow_vars = 'pwater pgas', and the variables in
   * the simulation are 'energy pwater pgas shape'
   * then isPorousFlowVariable(0) = false, isPorousFlowVariable(1) = true
   */
  bool isPorousFlowVariable(unsigned int moose_var_num) const;

  /**
   * returns true if moose_var_num is not a porous flow variabe
   * @param moose_var_num the MOOSE variable number
   * eg if porous_flow_vars = 'pwater pgas', and the variables in
   * the simulation are 'energy pwater pgas shape'
   * then notPorousFlowVariable(0) = true, notPorousFlowVariable(1) = false
   */
  bool notPorousFlowVariable(unsigned int moose_var_num) const;

  /**
   * Dummy pressure variable name for use in derivatives using the
   * DerivativeMaterialInterface
   */
  const VariableName pressureVariableNameDummy() const;

  /**
   * Dummy saturation variable name for use in derivatives using the
   * DerivativeMaterialInterface
   */
  const VariableName saturationVariableNameDummy() const;

  /**
   * Dummy temperature variable name for use in derivatives using the
   * DerivativeMaterialInterface
   */
  const VariableName temperatureVariableNameDummy() const;

  /**
   * Dummy mass fraction variable name for use in derivatives using the
   * DerivativeMaterialInterface
   */
  const VariableName massFractionVariableNameDummy() const;

protected:
  /// number of porousflow variables
  const unsigned int _num_variables;

  /// number of fluid phases
  const unsigned int _num_phases;

  /// number of fluid components
  const unsigned int _num_components;

private:
  /// _moose_var_num[i] = the moose variable number corresponding to porous flow variable i
  std::vector<unsigned int> _moose_var_num;

  /// _pf_var_num[i] = the porous flow variable corresponding to moose variable i
  std::vector<unsigned int> _pf_var_num;
};

#endif // POROUSFLOWDICTATOR_H
