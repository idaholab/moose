#ifndef DELTAGAMMA_H
#define DELTAGAMMA_H

#include "Kernel.h"

//Forward Declarations
class DeltaGamma;
class ColumnMajorMatrix;
template<typename T> class MooseArray;

template<>
InputParameters validParams<DeltaGamma>();

class DeltaGamma : public Kernel
{
public:

  DeltaGamma(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MooseArray<ColumnMajorMatrix> & _elastic_strain;
  MooseArray<Real> & _accumulated_plastic_strain;
  MooseArray<Real> & _von_mises_stress;
  MooseArray<Real> & _yield_stress;
  MooseArray<Real> & _shear_modulus;
};
#endif //DELTAGAMMA_H
