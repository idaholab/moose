#ifndef MATDIFFUSION_H
#define MATDIFFUSION_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class MatDiffusion;

template<>
InputParameters validParams<MatDiffusion>();


class MatDiffusion : public Kernel
{
public:
  MatDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;
  MaterialProperty<Real> & _diff;
};

#endif //MATDIFFUSION_H
