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

#ifndef ADDSIDESETSFORSPLITBOUNDARY_H
#define ADDSIDESETSFORSPLITBOUNDARY_H

#include "MeshModifier.h"

class AddSideSetsForSplitBoundary;

template<>
InputParameters validParams<AddSideSetsForSplitBoundary>();

/*
 * This modifier will add side sets to boundaries and name them based on subdomain IDs.
 * It is useful for problems with several different subdomains with variables living solely on them. In such a case,
 * on the same domain boundary, with different subdomain belongings, will be splitted to several boundary side sets
 * such that boundary conditions will be automatically defined on the places the variables lives in.
 */

class AddSideSetsForSplitBoundary : public MeshModifier
{
public:
  AddSideSetsForSplitBoundary(const InputParameters & parameters);

  virtual ~AddSideSetsForSplitBoundary();

  virtual void modify();

protected:
  /// number of subdomains
  unsigned int _num_subdomains;

  /// numbers of blocks of all subdomains
  std::vector<unsigned int> _num_subdomain_blocks;

  /// blocks of all subdomains
  std::vector<std::set<SubdomainID> > _subdomain_blocks;

  /// names of subdomains
  std::vector<SubdomainName> _subdomain_names;
};

#endif /* ADDSIDESETSFORSPLITBOUNDARY_H */
