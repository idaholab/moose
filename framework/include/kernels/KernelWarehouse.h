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

#ifndef KERNELWAREHOUSE_H
#define KERNELWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "Kernel.h"


/**
 * Typedef to hide implementation details
 */
typedef std::vector<Kernel *>::iterator KernelIterator;


/**
 * Holds kernels and provides some services
 */
class KernelWarehouse
{
public:
  KernelWarehouse();
  virtual ~KernelWarehouse();

  KernelIterator activeKernelsBegin();
  KernelIterator activeKernelsEnd();

  KernelIterator blockKernelsBegin(unsigned int block_id);
  KernelIterator blockKernelsEnd(unsigned int block_id);

  void addKernel(Kernel *kernel);
  void addBlockKernel(unsigned int block_id, Kernel *kernel);

  bool activeKernelBlocks(std::set<subdomain_id_type> & set_buffer) const;

  void updateActiveKernels(Real t, Real dt);

protected:
  std::vector<Kernel *> _active_kernels;
  std::vector<Kernel *> _all_kernels;
  std::map<unsigned int, std::vector<Kernel *> > _block_kernels;
  std::map<unsigned int, std::vector<Kernel *> > _all_block_kernels;
};

#endif // KERNELWAREHOUSE_H
