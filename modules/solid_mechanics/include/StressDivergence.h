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

  StressDivergence(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to=std::vector<std::string>(0),
            std::vector<std::string> coupled_as=std::vector<std::string>(0));
    
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
