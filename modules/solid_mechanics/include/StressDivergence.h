#ifndef STRESSDIVERGENCE_H
#define STRESSDIVERGENCE_H

#include "Kernel.h"

//Forward Declarations
class StressDivergence;
class ColumnMajorMatrix;

template<>
InputParameters validParams<StressDivergence>();

class StressDivergence : public Kernel
{
public:

  StressDivergence(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  void subdomainSetup();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::vector<RealTensorValue> * _stress;
  std::vector<ColumnMajorMatrix> * _elasticity_tensor;

private:
  unsigned int _component;
};
#endif //STRESSDIVERGENCE_H
