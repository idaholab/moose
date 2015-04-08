#ifndef TOTALFREEENERGYBASE_H
#define TOTALFREEENERGYBASE_H

#include "AuxKernel.h"

//Forward Declarations
class TotalFreeEnergyBase;

template<>
InputParameters validParams<TotalFreeEnergyBase>();

/**
 * Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined in a material and called f_name
 */
class TotalFreeEnergyBase : public AuxKernel
{
public:
  TotalFreeEnergyBase(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue() = 0;

  /// Coupled interface variables
  unsigned int _nvars;
  std::vector<VariableValue *> _vars;
  std::vector<VariableGradient *> _grad_vars;

  /// Gradient free energy prefactor kappa
  std::vector<std::string> _kappa_names;
  unsigned int _nkappas;

  /// Additional free energy contribution
  VariableValue & _additional_free_energy;
};

#endif //TOTALFREEENERGYBASE_H
