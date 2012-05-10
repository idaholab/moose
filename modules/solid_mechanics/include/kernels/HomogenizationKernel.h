#ifndef HOMOGENIZATIONKERNEL_H
#define HOMOGENIZATIONKERNEL_H

#include "Kernel.h"

//Forward Declarations
class ColumnMajorMatrix;
class HomogenizationKernel;
class SymmElasticityTensor;
class SymmTensor;

template<>
InputParameters validParams<HomogenizationKernel>();

class HomogenizationKernel : public Kernel
{
public:

  HomogenizationKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;

private:
  const unsigned int _component;
  const unsigned int _column;


};
#endif //HOMOGENIZATIONKERNEL_H
