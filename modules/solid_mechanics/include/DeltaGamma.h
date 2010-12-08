#ifndef DELTAGAMMA_H
#define DELTAGAMMA_H

#include "Kernel.h"

//Forward Declarations
class DeltaGamma;
class ColumnMajorMatrix;

template<>
InputParameters validParams<DeltaGamma>();

class DeltaGamma : public Kernel
{
public:

  DeltaGamma(const std::string & name, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;
  MaterialProperty<Real> & _accumulated_plastic_strain;
  MaterialProperty<Real> & _von_mises_stress;
  MaterialProperty<Real> & _yield_stress;
  MaterialProperty<Real> & _shear_modulus;
};
#endif //DELTAGAMMA_H
