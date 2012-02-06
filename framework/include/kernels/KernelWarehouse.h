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
#include "ScalarKernel.h"


/**
 * Holds kernels and provides some services
 */
class KernelWarehouse
{
public:
  KernelWarehouse();
  virtual ~KernelWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Get list of all kernels
   * @return The list of all active kernels
   */
  const std::vector<Kernel *> & all() { return _all_kernels; }

  /**
   * Get the list of all active kernels
   * @return The list of all active kernels
   */
  const std::vector<Kernel *> & active() { return _active_kernels; }

  /**
   * Get the list of all active kernels for a variable
   * @param var The variable number
   * @return The list of all active kernels
   */
  const std::vector<Kernel *> & activeVar(unsigned int var) { return _active_var_kernels[var]; }

  /**
   * Get list of scalar kernels
   * @return The list of scalar active kernels
   */
  const std::vector<ScalarKernel *> & scalars() { return _scalar_kernels; }

  /**
   * Add a kernels
   * @param kernel Kernel being added
   * @param block_ids Set of active domain where the kernel is defined
   */
  void addKernel(Kernel *kernel, const std::set<subdomain_id_type> & block_ids);

  /**
   * Add a scalar kernels
   * @param kernel Scalar kernel being added
   */
  void addScalarKernel(ScalarKernel *kernel);

  /**
   * Update the list of active kernels
   * @param t Time
   * @param dt Time step size
   * @param subdomain_id Domain ID
   */
  void updateActiveKernels(Real t, Real dt, unsigned int subdomain_id);

  /**
   * This returns a boolean to indicate whether this warehouse contains kernels
   * representing all of the subdomains, if not then the supplied set is filled in
   * with the complete set of subdomains represented which may or may not represent
   * the entire domain.
   */
  bool subdomains_covered(std::set<subdomain_id_type> & return_set) const;

protected:
  std::vector<Kernel *> _active_kernels;                                ///< Kernels active on a block and in specified time
  std::map<unsigned int, std::vector<Kernel *> > _active_var_kernels;   ///< Kernels active on a block and in specified time per variable
  std::vector<Kernel *> _all_kernels;                                   ///< All instances of kernels
  std::vector<Kernel *> _global_kernels;                                ///< Kernels that live everywhere (on the whole domain)
  std::map<unsigned int, std::vector<Kernel *> > _block_kernels;        ///< Kernels that live on a specified block

  std::vector<ScalarKernel *> _scalar_kernels;                          ///< Scalar kernels
};

#endif // KERNELWAREHOUSE_H
