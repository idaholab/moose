#ifndef STRESSDIVERGENCERZ_H
#define STRESSDIVERGENCERZ_H

#include "Kernel.h"

//Forward Declarations
class StressDivergenceRZ;
class ColumnMajorMatrix;

template<>
InputParameters validParams<StressDivergenceRZ>();

class StressDivergenceRZ : public Kernel
{
public:

  StressDivergenceRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;

private:
  const unsigned int _component;

  const bool _rdisp_coupled;
  const bool _zdisp_coupled;
  const unsigned int _rdisp_var;
  const unsigned int _zdisp_var;
};
#endif //STRESSDIVERGENCERZ_H
