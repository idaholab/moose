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

//Moose Includes
#include "Moose.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "DamperWarehouse.h"
#include "ComputeBase.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalDamping : public ComputeBase<ConstElemRange>
{
public:
  ComputeInternalDamping(MooseSystem &sys, const NumericVector<Number>& in_soln, const NumericVector<Number>& update) :
    ComputeBase<ConstElemRange>(sys),
    _damping(1.0),
    _soln(in_soln),
    _update(update)
  {}

  // Splitting Constructor
  ComputeInternalDamping(ComputeInternalDamping & x, Threads::split) :
    ComputeBase<ConstElemRange>(x._moose_system),
    _damping(1.0),
    _soln(x._soln),
    _update(x._update)
  {}

  virtual void onElement(const Elem *elem)
  {
    _moose_system.reinitKernels(_tid, _soln, elem, NULL);
    _moose_system.reinitDampers(_tid, _update);

    DamperIterator damper_begin = _moose_system._dampers[_tid].dampersBegin();
    DamperIterator damper_end = _moose_system._dampers[_tid].dampersEnd();
    DamperIterator damper_it = damper_begin;

    for(damper_it=damper_begin;damper_it!=damper_end;++damper_it)
    {
      Real cur_damping = (*damper_it)->computeDamping();
      if(cur_damping < _damping)
        _damping = cur_damping;
    }
  }

  // Join Operator
  void join(const ComputeInternalDamping & y)
  {
    if(y._damping < _damping)
      _damping = y._damping;
  }    


  Real _damping;

protected:
  const NumericVector<Number> & _soln;
  const NumericVector<Number> & _update;
};

Real MooseSystem::computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  DamperIterator damper_begin = _dampers[0].dampersBegin();
  DamperIterator damper_end = _dampers[0].dampersEnd();
  DamperIterator damper_it = damper_begin;

  if(damper_begin != damper_end)
  {
    updateAuxVars(soln);

    ComputeInternalDamping cid(*this, soln, update);
    
    Threads::parallel_reduce(*getActiveLocalElementRange(), cid);

    damping = cid._damping;
  }

  Parallel::min(damping);
  
  Moose::perf_log.pop("compute_dampers()","Solve");

  return damping;
}

