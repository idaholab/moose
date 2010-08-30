//Moose Includes
#include "Moose.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "DamperWarehouse.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalDamping
{
public:
  ComputeInternalDamping(MooseSystem &sys, const NumericVector<Number>& in_soln, const NumericVector<Number>& update)
    :_moose_system(sys),
     _soln(in_soln),
     _update(update),
     _damping(1.0)
  {}

  // Splitting Constructor
  ComputeInternalDamping(ComputeInternalDamping & x, Threads::split)
    : _moose_system(x._moose_system),
      _soln(x._soln),
      _update(x._update),
      _damping(1.0)
  {}

  // Join Operator
  void join(const ComputeInternalDamping & y)
  {
    if(y._damping < _damping)
      _damping = y._damping;
  }    

  void operator() (const ConstElemRange & range)
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;

    ConstElemRange::const_iterator el = range.begin();

    DamperIterator damper_begin = _moose_system._dampers[tid].dampersBegin();
    DamperIterator damper_end = _moose_system._dampers[tid].dampersEnd();
    DamperIterator damper_it = damper_begin;

    Real cur_damping;

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      _moose_system.reinitKernels(tid, _soln, elem, NULL);
      _moose_system.reinitDampers(tid, _update);

      unsigned int cur_subdomain = elem->subdomain_id();

      for(damper_it=damper_begin;damper_it!=damper_end;++damper_it)
      {
        cur_damping = (*damper_it)->computeDamping();
        if(cur_damping < _damping)
          _damping = cur_damping;
      }
    }
  }
  
  Real _damping;

protected:
  MooseSystem & _moose_system;
  const NumericVector<Number> & _soln;
  const NumericVector<Number> & _update;
};

Real MooseSystem::compute_damping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  // TODO: Make this work with threads!
  DamperIterator damper_begin = _dampers[0].dampersBegin();
  DamperIterator damper_end = _dampers[0].dampersEnd();
  DamperIterator damper_it = damper_begin;

  if(damper_begin != damper_end)
  {
    update_aux_vars(soln);

    ComputeInternalDamping cid(*this, soln, update);
    
    Threads::parallel_reduce(*getActiveLocalElementRange(), cid);

    damping = cid._damping;
  }

  return damping;
  
  Moose::perf_log.pop("compute_dampers()","Solve");
}

