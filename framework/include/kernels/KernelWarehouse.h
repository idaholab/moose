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

#include "Warehouse.h"
#include "MooseTypes.h"
#include "MooseError.h"

class KernelBase;
class ScalarKernel;

/**
 * Holds kernels and provides some services
 */
class KernelWarehouse : public Warehouse<KernelBase>
{
public:
  KernelWarehouse();
  virtual ~KernelWarehouse();

  // Setup /////
  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void residualSetup();
  virtual void jacobianSetup();

  /**
   * Get the list of all active kernels
   * @return The list of all active kernels
   */
  const std::vector<KernelBase *> & active() const { return _active_kernels; }

  /**
   * Get the list of all active time kernels
   * @return The list of all active time kernels
   */
  const std::vector<KernelBase *> & activeTime() const { return _time_kernels;}

  /**
   * Get the list of all active non-time kernels
   * @return The list of all active non-time kernels
   */
  const std::vector<KernelBase *> & activeNonTime() const { return _non_time_kernels;}

  /**
   * See if there are active kernels for a variable
   * @param var The variable number
   * @return Boolean indicating whether there are active kernels
   */
  bool hasActiveKernels(unsigned int var) const
    {
      return _active_var_kernels.find(var) != _active_var_kernels.end();
    }

  /**
   * Get the list of all active kernels for a variable
   * @param var The variable number
   * @return The list of all active kernels
   */
  const std::vector<KernelBase *> & activeVar(unsigned int var) const
    {
      mooseAssert(_active_var_kernels.find(var) != _active_var_kernels.end(), "No active kernels");
      return _active_var_kernels.find(var)->second;
    }

  /**
   * Get list of scalar kernels
   * @return The list of scalar active kernels
   */
  const std::vector<ScalarKernel *> & scalars() const { return _scalar_kernels; }

  /**
   * Add a kernels
   * @param kernel Kernel being added
   * @param block_ids Set of active domain where the kernel is defined
   */
  void addKernel(MooseSharedPointer<KernelBase> & kernel, const std::set<SubdomainID> & block_ids);

  /**
   * Add a scalar kernels
   * @param kernel Scalar kernel being added
   */
  void addScalarKernel(MooseSharedPointer<ScalarKernel> & kernel);

  /**
   * Update the list of active kernels
   * @param subdomain_id Domain ID
   */
  void updateActiveKernels(unsigned int subdomain_id);

  /**
   * This returns a boolean to indicate whether this warehouse contains kernels
   * representing all of the subdomains, if not then the supplied set is filled in
   * with the complete set of subdomains represented which may or may not represent
   * the entire domain.  In addition a count variables containing one or more kernels
   * is returned through the reference parameter.
   * @param subdomains_covered A writable reference to a list of subdomains with active kernels on them if no global kernels exist
   * @param unique_variable_count A writable reference to a count of variables containing one or kernels
   * @return bool A Boolean indicating whether all subdomains are covered by kernels
   */
  bool subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variable_count) const;

protected:
  ///@{
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<KernelBase> > _all_ptrs;
  std::vector<MooseSharedPointer<ScalarKernel> > _all_scalar_ptrs;
  ///@}

  /// Kernels active on a block and in specified time
  std::vector<KernelBase *> _active_kernels;
  ///  active TimeDerivitive Kernels
  std::vector<KernelBase *> _time_kernels;

  /// active NonTimeDerivitive Kernels
  std::vector<KernelBase *> _non_time_kernels;

  /// Kernels active on a block and in specified time per variable
  std::map<unsigned int, std::vector<KernelBase *> > _active_var_kernels;
  /// Kernels that live everywhere (on the whole domain)
  std::vector<KernelBase *> _time_global_kernels;

  /// Kernels that live everywhere (on the whole domain)
  std::vector<KernelBase *> _nontime_global_kernels;
  /// Kernels that live on a specified block
  std::map<SubdomainID, std::vector<KernelBase *> > _time_block_kernels;
  /// Kernels that live on a specified block
  std::map<SubdomainID, std::vector<KernelBase *> > _nt_block_kernels;
  /// Scalar kernels
  std::vector<ScalarKernel *> _scalar_kernels;
};

#endif // KERNELWAREHOUSE_H
