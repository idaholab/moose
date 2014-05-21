#ifndef MOMENTBALANCING_H
#define MOMENTBALANCING_H

#include "Kernel.h"


//Forward Declarations
class MomentBalancing;
class ElasticityTensorR4;
class RankTwoTensor;

template<>
InputParameters validParams<MomentBalancing>();

class MomentBalancing : public Kernel
{
public:

  MomentBalancing(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;

private:
  const unsigned int _component;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
};
#endif //MOMENTBALANCING_H
