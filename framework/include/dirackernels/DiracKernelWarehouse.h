#ifndef DIRACKERNELWAREHOUSE_H
#define DIRACKERNELWAREHOUSE_H

#include "DiracKernel.h"

#include <vector>

/**
 * Typedef to hide implementation details
 */
typedef std::vector<DiracKernel *>::iterator DiracKernelIterator;

/**
 * Holds DiracKernels and provides some services
 */
class DiracKernelWarehouse
{
public:
  DiracKernelWarehouse();
  virtual ~DiracKernelWarehouse();

  DiracKernelIterator diracKernelsBegin();
  DiracKernelIterator diracKernelsEnd();

  void addDiracKernel(DiracKernel *DiracKernel);
    
protected:
  std::vector<DiracKernel *> _dirac_kernels;
};

#endif // DIRACKERNELWAREHOUSE_H
