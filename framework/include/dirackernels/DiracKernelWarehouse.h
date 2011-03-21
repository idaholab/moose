#ifndef DIRACKERNELWAREHOUSE_H_
#define DIRACKERNELWAREHOUSE_H_

#include "DiracKernel.h"

#include <vector>

/**
 * Holds DiracKernels and provides some services
 */
class DiracKernelWarehouse
{
public:
  DiracKernelWarehouse();
  virtual ~DiracKernelWarehouse();

  /**
   * Get the list of all dirac kernels
   */
  std::vector<DiracKernel *> & all();

  void addDiracKernel(DiracKernel *DiracKernel);
    
protected:
  std::vector<DiracKernel *> _dirac_kernels;
};

#endif // DIRACKERNELWAREHOUSE_H_
