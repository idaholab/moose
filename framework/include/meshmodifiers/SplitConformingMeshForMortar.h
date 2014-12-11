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

#ifndef SPLITCONFORMINGMESHFORMORTAR_H
#define SPLITCONFORMINGMESHFORMORTAR_H

#include "MeshModifier.h"

class SplitConformingMeshForMortar;

template<>
InputParameters validParams<SplitConformingMeshForMortar>();

/*
 * This modifier will create blocks for mortar faces among subdomains with nameing convention 'interface-#1-to-#2',
 * where #1 is the number of the from subdomain ID and #2 is the number of the to subdomain ID.
 * (Note that here subdomain is different from libMesh subdomain, which is indeed just block.) a bunch of side sets:
 *
 * It will also disconnect all subdomains by duplicating interface nodes and reset the connectivity of all elements on
 * subdomain interfaces to these nodes.
 *
 * It will also split existing side sets with respect to all subdomains into three parts with naming
 * '#ss_name-#1-boundary', '#ss_name-#1-interior' and '#ss_name-#1-outside', where
 * #ss_name is the name of the side set (if the name is missing, it will be the side set ID), #1 is the subdomain ID.
 *
 * It will also create new side sets between subdomains with nameing
 * 'interface-#1-to-#2', where #1 is the number of the from subdomain ID and #2 is the number of the to subdomain ID.
 */

class SplitConformingMeshForMortar :
  public MeshModifier
{
public:
  SplitConformingMeshForMortar(const std::string & name, InputParameters parameters);

  virtual ~SplitConformingMeshForMortar();

  virtual void modify();

protected:
  /// number of subdomains for mortar FEM
  unsigned int _num_subdomains;
  /// number of blocks of all subdomains
  std::vector<unsigned int> _num_subdomain_blocks;
  /// blocks of all subdomains
  std::vector<std::set<SubdomainID> > _subdomain_blocks;
};

#endif /* SPLITCONFORMINGMESHFORMORTAR_H */
