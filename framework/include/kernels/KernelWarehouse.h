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

  void addKernel(Kernel *kernel, const std::set<subdomain_id_type> & block_ids);

  void updateActiveKernels(Real t, Real dt, subdomain_id_type subdomain_id);

protected:
  /**
   * Kernels active on a block and in specified time
   */
  std::vector<Kernel *> _active_kernels;

  /**
   * All instances of kernels
   */
  std::vector<Kernel *> _all_kernels;

  /**
   * Kernels that live everywhere (on the whole domain)
   */
  std::vector<Kernel *> _global_kernels;

  /**
   * Kernels that live on a specified block
   */
  std::map<unsigned int, std::vector<Kernel *> > _block_kernels;
};

#endif // KERNELWAREHOUSE_H
