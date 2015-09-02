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

#ifndef MAXVARNDOFSPERELEM_H
#define MAXVARNDOFSPERELEM_H

#include "ThreadedElementLoop.h"
// libMesh includes
#include "libmesh/elem_range.h"

class NonlinearSystem;


class MaxVarNDofsPerElem : public ThreadedElementLoop<ConstElemRange>
{
public:
  MaxVarNDofsPerElem(FEProblem & feproblem, NonlinearSystem & sys);

  // Splitting Constructor
  MaxVarNDofsPerElem(MaxVarNDofsPerElem & x, Threads::split split);

  virtual ~MaxVarNDofsPerElem();

  virtual void onElement(const Elem *elem);

  void join(const MaxVarNDofsPerElem &);

  dof_id_type max() { return _max; }

protected:
  /// Maximum number of dofs for any one variable on any one element
  size_t _max;

  /// DOF map
  const DofMap & _dof_map;

  /// Reusable storage
  std::vector<dof_id_type> _dof_indices;
};

#endif //MAXVARNDOFSPERELEM_H
