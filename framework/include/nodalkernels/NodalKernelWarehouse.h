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

#ifndef NODALKERNELWAREHOUSE_H
#define NODALKERNELWAREHOUSE_H

#include "Warehouse.h"

#include <vector>
#include <map>
#include <string>
#include <list>
#include <set>

class NodalKernel;

/**
 * Warehouse for storing NodalKernels
 */
class NodalKernelWarehouse : public Warehouse<NodalKernel>
{
public:
  NodalKernelWarehouse();
  virtual ~NodalKernelWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  const std::vector<MooseSharedPointer<NodalKernel> > & allNodalKernels() const { return _all_nodal_kernels; }

  std::vector<MooseSharedPointer<NodalKernel> > & activeBlockNodalKernels(SubdomainID block) { return _active_block_nodal_kernels[block]; }

  /**
   * Adds a nodal kernel
   * @param NodalKernel being added
   */
  void addNodalKernel(MooseSharedPointer<NodalKernel> & nodal_kernel);

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
  /// all NodalKernels
  std::vector<MooseSharedPointer<NodalKernel> > _all_nodal_kernels;

  /// nodal kernels active on a block
  std::map<SubdomainID, std::vector<MooseSharedPointer<NodalKernel> > > _active_block_nodal_kernels;
};

#endif // NODALKERNELWAREHOUSE_H
