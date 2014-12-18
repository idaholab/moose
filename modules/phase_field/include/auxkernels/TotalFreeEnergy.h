#ifndef TOTALFREEENERGY_H
#define TOTALFREEENERGY_H

#include "AuxKernel.h"

//Forward Declarations
class TotalFreeEnergy;

template<>
InputParameters validParams<TotalFreeEnergy>();

/**
 * Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined in a material and called f_name
 */
class TotalFreeEnergy : public AuxKernel
{
public:
  TotalFreeEnergy(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  unsigned int _nvars;
  MaterialProperty<Real> & _F;
  std::vector<std::string> _kappa_names;
  std::vector<VariableGradient *> _grad_vars;
  std::vector<MaterialProperty<Real> *> _kappas;
};

#endif //TOTALFREEENERGY_H
