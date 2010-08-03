#ifndef DIFFMKERNEL_H
#define DIFFMKERNEL_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffMKernel;

template<>
InputParameters validParams<DiffMKernel>();


class DiffMKernel : public Kernel
{
public:
  DiffMKernel(std::string name, MooseSystem & moose_system, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;
  MaterialProperty<Real> & _diff;
  MaterialProperty<std::vector<Real> > & _vec_prop;
};
#endif //DIFFMKERNEL_H
