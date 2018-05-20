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

#ifndef COHESIVEZONEMESHSPLIT_H
#define COHESIVEZONEMESHSPLIT_H

#include "CohesiveZoneMeshSplitBase.h"

// forward declaration
class CohesiveZoneMeshSplit;

template <>
InputParameters validParams<CohesiveZoneMeshSplit>();

class CohesiveZoneMeshSplit : public CohesiveZoneMeshSplitBase
{
public:
  CohesiveZoneMeshSplit(const InputParameters & parameters);
  CohesiveZoneMeshSplit(const CohesiveZoneMeshSplit & other_mesh);
  virtual ~CohesiveZoneMeshSplit(); // empty dtor required for unique_ptr with forward declarations

  virtual void init() override;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

protected:
private:
  /// loop over the node and find their support
  void buildNodeSupport();

  /// find nodes duplicity (i.e. how many times a node need to be duplicated)
  void buildInterfacialNodes();

  /// duplicate nodes with duplicity greater than 1
  void duplicateNodes();

  /// assign duplicated nodes to the approriate element
  void tearElements();

  /// generate the new boundary interface
  void addInterfaceBoundary();

  std::map<dof_id_type, std::vector<dof_id_type>> _node_support;
  std::set<std::pair<subdomain_id_type, subdomain_id_type>> _boundary_pairs;
  std::map<std::pair<subdomain_id_type, subdomain_id_type>,
           std::set<std::pair<dof_id_type, unsigned int>>>
      _boundary_sides;
  std::map<dof_id_type, unsigned int> _node_duplicity;
  std::map<dof_id_type, std::vector<subdomain_id_type>> _duplicated_node_materials;
  std::map<dof_id_type, std::vector<dof_id_type>> _duplicated_node;
};

#endif // COHESIVEZONEMESHSPLIT_H
