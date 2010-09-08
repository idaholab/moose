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

  StressDivergence(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;

private:
  unsigned int _component;

  unsigned int _xdisp_var, _ydisp_var, _zdisp_var;
};
#endif //STRESSDIVERGENCE_H
