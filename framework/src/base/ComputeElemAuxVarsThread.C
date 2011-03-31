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

#include "ComputeElemAuxVarsThread.h"

#include "AuxiliarySystem.h"
#include "MProblem.h"

// libmesh includes
#include "threads.h"

ComputeElemAuxVarsThread::ComputeElemAuxVarsThread(MProblem & mproblem,
                                                   AuxiliarySystem & sys,
                                                   std::vector<AuxWarehouse> & auxs) :
    _mproblem(mproblem),
    _sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeElemAuxVarsThread::ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x, Threads::split split) :
    _mproblem(x._mproblem),
    _sys(x._sys),
    _auxs(x._auxs)
{
}

void
ComputeElemAuxVarsThread::operator() (const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  AuxKernelIterator aux_begin = _auxs[_tid].activeElementAuxKernelsBegin();
  AuxKernelIterator aux_end = _auxs[_tid].activeElementAuxKernelsEnd();

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();
  for (ConstElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const Elem * elem = *elem_it;

    // prepare variables
    for (std::map<std::string, MooseVariable *>::iterator it = _sys._elem_vars[_tid].begin(); it != _sys._elem_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepare_aux();
    }

    unsigned int cur_subdomain = elem->subdomain_id();

//    if(unlikely(_calculate_element_time))
//      startElementTiming(elem->id());

    AuxKernelIterator block_element_aux_it = _auxs[_tid].activeBlockElementAuxKernelsBegin(cur_subdomain);
    AuxKernelIterator block_element_aux_end = _auxs[_tid].activeBlockElementAuxKernelsEnd(cur_subdomain);

    if(block_element_aux_it != block_element_aux_end || aux_begin != aux_end)
    {
      _mproblem.prepare(elem, _tid);
      _mproblem.reinitElem(elem, _tid);
      _mproblem.reinitMaterials(elem->subdomain_id(), _tid);

      //Compute the area of the element
      _sys._data[_tid]._current_volume = _mproblem.assembly(_tid).computeVolume();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        for(AuxKernelIterator aux_it=aux_begin;aux_it!=aux_end;aux_it++)
          (*aux_it)->subdomainSetup();
      }

      // block
      for(; block_element_aux_it != block_element_aux_end; ++block_element_aux_it)
        (*block_element_aux_it)->compute();

      // global
      for(AuxKernelIterator aux_it=aux_begin;aux_it!=aux_end;aux_it++)
        (*aux_it)->compute();
    }

//    if(unlikely(_calculate_element_time))
//      stopElementTiming(elem->id());

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

void
ComputeElemAuxVarsThread::join(const ComputeElemAuxVarsThread & /*y*/)
{
}
