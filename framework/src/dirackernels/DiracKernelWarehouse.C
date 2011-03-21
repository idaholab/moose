#include "DiracKernelWarehouse.h"

DiracKernelWarehouse::DiracKernelWarehouse()
{
}

DiracKernelWarehouse::~DiracKernelWarehouse()
{
  // delete  DiracKernels
  for (std::vector<DiracKernel *>::iterator i=_dirac_kernels.begin(); i!=_dirac_kernels.end(); ++i)
    delete *i;
}

DiracKernelIterator
DiracKernelWarehouse::diracKernelsBegin()
{
  return _dirac_kernels.begin();
}

DiracKernelIterator
DiracKernelWarehouse::diracKernelsEnd()
{
  return _dirac_kernels.end();
}

void
DiracKernelWarehouse::addDiracKernel(DiracKernel *DiracKernel)
{
  _dirac_kernels.push_back(DiracKernel);
}
