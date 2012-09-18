#ifndef APPLIEDSTRESSDIVERGENCE_H
#define APPLIEDSTRESSDIVERGENCE_H

#include "Kernel.h"
#include "RankTwoTensor.h"


/**
 * AppliedStressDivergence applies a uniform applied stress or strain.
 **/

//Forward Declarations
class AppliedStressDivergence;
class ElasticityTensorR4;

template<>
InputParameters validParams<AppliedStressDivergence>();

class AppliedStressDivergence : public Kernel
{
public:

  AppliedStressDivergence(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
  MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;
  std::vector<Real> _applied_strain_vector;

private:
  const unsigned int _component;
  RankTwoTensor _applied_strain;
};
#endif //APPLIEDSTRESSDIVERGENCE_H
