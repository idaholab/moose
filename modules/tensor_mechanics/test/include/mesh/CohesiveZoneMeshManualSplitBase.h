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

#ifndef COHESIVEZONEMESHMANUALSPLITBASE_H
#define COHESIVEZONEMESHMANUALSPLITBASE_H

#include "CohesiveZoneMeshSplitBase.h"

// forward declaration
class CohesiveZoneMeshManualSplitBase;

template <>
InputParameters validParams<CohesiveZoneMeshManualSplitBase>();

class CohesiveZoneMeshManualSplitBase : public CohesiveZoneMeshSplitBase
{
public:
  CohesiveZoneMeshManualSplitBase(const InputParameters & parameters);
  CohesiveZoneMeshManualSplitBase(const CohesiveZoneMeshManualSplitBase & other_mesh);
  virtual ~CohesiveZoneMeshManualSplitBase(); // empty dtor required for unique_ptr with forward
                                              // declarations

  virtual void init() override; // method to override to implement other mesh splitting algorithms

  virtual std::unique_ptr<MooseMesh>  safeClone() const override;
  
protected:

  /// method that given an element id and a local node ID duplicate it and set it
  virtual void duplicateAndSetLocalNode(dof_id_type element_id, dof_id_type local_node);

  /// method setting the local_node of element_id to global_node
  virtual void setElemNode(dof_id_type element_id, dof_id_type local_node, dof_id_type global_node);
};

#endif // COHESIVEZONEMESHMANUALSPLITBASE_H
