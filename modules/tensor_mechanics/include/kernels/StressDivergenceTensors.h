#ifndef STRESSDIVERGENCETENSORS_H
#define STRESSDIVERGENCETENSORS_H

#include "Kernel.h"

//Forward Declarations
class StressDivergenceTensors;
class ElasticityTensorR4;
class RankTwoTensor;

template<>
InputParameters validParams<StressDivergenceTensors>();

/**
 * StressDivergenceTensors mostly copies from StressDivergence.  There are small changes to use
 * RankFourTensor and RankTwoTensors instead of SymmElasticityTensors and SymmTensors.  This is done
 * to allow for more mathematical transparancy.
 */
class StressDivergenceTensors : public Kernel
{
public:
  StressDivergenceTensors(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
  // MaterialProperty<RankTwoTensor> & _d_stress_dT;

  const unsigned int _component;

private:
  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;
  const bool _temp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
  const unsigned int _temp_var;
};
#endif //STRESSDIVERGENCETENSORS_H
