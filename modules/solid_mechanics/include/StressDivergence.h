#ifndef STRESSDIVERGENCE_H
#define STRESSDIVERGENCE_H

#include "Kernel.h"

//Forward Declarations
class StressDivergence;
class ColumnMajorMatrix;
template<typename T> class MooseArray;

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

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MooseArray<RealTensorValue> * _stress;
  MooseArray<ColumnMajorMatrix> * _elasticity_tensor;

private:
  unsigned int _component;

  unsigned int _xdisp_var, _ydisp_var, _zdisp_var;
};
#endif //STRESSDIVERGENCE_H
