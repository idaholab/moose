#ifndef MATCOEFDIFFUSION_H
#define MATCOEFDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class MatCoefDiffusion;

template<>
InputParameters validParams<MatCoefDiffusion>();

/**
 * A test class for checking the operation for BlockRestrictable::hasMaterialProperty
 */
class MatCoefDiffusion : public Kernel
{
public:
  MatCoefDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  std::string _prop_name;
  MaterialProperty<Real> * _coef;
};

#endif //MATCOEFDIFFUSION_H
