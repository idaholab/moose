#ifndef HHPFCRFF_H
#define HHPFCRFF_H

#include "KernelValue.h"

//Forward Declarations
class HHPFCRFF;

template<>
InputParameters validParams<HHPFCRFF>();

class HHPFCRFF : public KernelValue
{
public:
  HHPFCRFF(const std::string & name, InputParameters parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _positive;
  Real _kernel_sign;

  std::string _prop_name;
  MaterialProperty<Real> & _prop;

  bool _has_coupled_var;
  VariableValue * _coupled_var;
  unsigned int _coupled_var_var;
};

#endif //HHFPCRFF_H
