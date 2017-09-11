/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALDERIVATIVERANKFOURTESTKERNEL_H
#define MATERIALDERIVATIVERANKFOURTESTKERNEL_H

#include "MaterialDerivativeTestKernelBase.h"
#include "RankFourTensor.h"

class MaterialDerivativeRankFourTestKernel;

template <>
InputParameters validParams<MaterialDerivativeRankFourTestKernel>();

/**
 * This kernel is used for testing derivatives of a material property.
 */
class MaterialDerivativeRankFourTestKernel : public MaterialDerivativeTestKernelBase<RankFourTensor>
{
public:
  MaterialDerivativeRankFourTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const unsigned int _component_i;
  const unsigned int _component_j;
  const unsigned int _component_k;
  const unsigned int _component_l;
};

#endif /* MATERIALDERIVATIVERANKFOURTESTKERNEL_H */
