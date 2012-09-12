#ifndef STRESSDIVERGENCETENSORSTONKS_H
#define STRESSDIVERGENCETENSORSTONKS_H

#include "Kernel.h"


/** StressDivergenceTensorsTonks mostly copies from StressDivergence.  There are small changes to use
 * RankFourTensor and RankTwoTensorsTonks instead of SymmElasticityTensorsTonks and SymmTensorsTonks.  This is done
 * to allow for more mathematical transparancy.
 **/

//Forward Declarations
class StressDivergenceTensorsTonks;
class ElasticityTensorR4;
class RankTwoTensorTonks;

template<>
InputParameters validParams<StressDivergenceTensorsTonks>();

class StressDivergenceTensorsTonks : public Kernel
{
public:

  StressDivergenceTensorsTonks(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RankTwoTensorTonks> & _stress;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
  // MaterialProperty<RankTwoTensor> & _d_stress_dT;

private:
  const unsigned int _component;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;
  const bool _temp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
  const unsigned int _temp_var;
};
#endif //STRESSDIVERGENCETENSORSTONKS_H
