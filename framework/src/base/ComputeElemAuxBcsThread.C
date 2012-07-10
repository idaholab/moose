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

#include "ComputeElemAuxBcsThread.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"

// libmesh includes
#include "threads.h"

ComputeElemAuxBcsThread::ComputeElemAuxBcsThread(FEProblem & problem,
                                                 AuxiliarySystem & sys,
                                                 std::vector<AuxWarehouse> & auxs) :
    _problem(problem),
    _sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeElemAuxBcsThread::ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x, Threads::split /*split*/) :
    _problem(x._problem),
    _sys(x._sys),
    _auxs(x._auxs)
{
}

void
ComputeElemAuxBcsThread::operator() (const ConstBndElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstBndElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const BndElement * belem = *elem_it;

    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == libMesh::processor_id())
    {
      // prepare variables
      for (std::map<std::string, MooseVariable *>::iterator it = _sys._elem_vars[_tid].begin(); it != _sys._elem_vars[_tid].end(); ++it)
      {
        MooseVariable * var = it->second;
        var->prepare_aux();
      }

      if (_auxs[_tid].elementalBCs(boundary_id).size() > 0)
      {
        _problem.prepare(elem, _tid);
        _problem.reinitElemFace(elem, side, boundary_id, _tid);
        _problem.reinitMaterialsBoundary(boundary_id, _tid);

        const std::vector<AuxKernel*> & bcs = _auxs[_tid].elementalBCs(boundary_id);
        for (std::vector<AuxKernel*>::const_iterator element_bc_it = bcs.begin();
            element_bc_it != bcs.end(); ++element_bc_it)
          (*element_bc_it)->compute();
      }

      // update the solution vector
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (std::map<std::string, MooseVariable *>::iterator it = _sys._elem_vars[_tid].begin(); it != _sys._elem_vars[_tid].end(); ++it)
        {
          MooseVariable * var = it->second;
          var->insert(_sys.solution());
        }
      }
    }
  }
}

void
ComputeElemAuxBcsThread::join(const ComputeElemAuxBcsThread & /*y*/)
{
}
