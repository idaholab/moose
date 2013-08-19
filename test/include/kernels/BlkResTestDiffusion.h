#ifndef BLKRESTESTDIFFUSION_H
#define BLKRESTESTDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class BlkResTestDiffusion;

template<>
InputParameters validParams<BlkResTestDiffusion>();

InputParameters & modifyParams(InputParameters & params);

class BlkResTestDiffusion : public Kernel
{
public:
  BlkResTestDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif //BLKRESTESTDIFFUSION_H
