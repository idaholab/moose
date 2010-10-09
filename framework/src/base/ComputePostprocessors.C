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

//libMesh includes
#include "numeric_vector.h"
#include "nonlinear_implicit_system.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalPostprocessors
{
public:
  ComputeInternalPostprocessors(MooseSystem &sys, const NumericVector<Number>& in_soln)
    :_moose_system(sys),
     _soln(in_soln)
  {}

  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;

    DenseVector<Number> Re;

    ConstElemRange::const_iterator el = range.begin();

    //Initialize side and element post processors

    std::set<unsigned int>::iterator block_begin = _moose_system._pps[tid]._block_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator block_end = _moose_system._pps[tid]._block_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator block_it = block_begin;

    for (block_it=block_begin;block_it!=block_end;++block_it)
    {
      unsigned int block_id = *block_it;

      PostprocessorIterator postprocessor_begin = _moose_system._pps[tid].elementPostprocessorsBegin(block_id);
      PostprocessorIterator postprocessor_end = _moose_system._pps[tid].elementPostprocessorsEnd(block_id);
      PostprocessorIterator postprocessor_it = postprocessor_begin;

      for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
        (*postprocessor_it)->initialize();
    }

    std::set<unsigned int>::iterator boundary_begin = _moose_system._pps[tid]._boundary_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator boundary_end = _moose_system._pps[tid]._boundary_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator boundary_it = boundary_begin;

    for (boundary_it=boundary_begin;boundary_it!=boundary_end;++boundary_it)
    {
      //note: for threaded applications where the elements get broken up it
      //may be more efficient to initialize these on demand inside the loop
      PostprocessorIterator side_postprocessor_begin = _moose_system._pps[tid].sidePostprocessorsBegin(*boundary_it);
      PostprocessorIterator side_postprocessor_end = _moose_system._pps[tid].sidePostprocessorsEnd(*boundary_it);
      PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

      for (side_postprocessor_it=side_postprocessor_begin;side_postprocessor_it!=side_postprocessor_end;++side_postprocessor_it)
        (*side_postprocessor_it)->initialize();
    }

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      _moose_system.reinitKernels(tid, _soln, elem, NULL);
      _moose_system._element_data[tid]->reinitMaterials(_moose_system._materials[tid].getMaterials(elem->subdomain_id()));

      unsigned int cur_subdomain = elem->subdomain_id();

      //Global Postprocessors
      PostprocessorIterator postprocessor_begin = _moose_system._pps[tid].elementPostprocessorsBegin(Moose::ANY_BLOCK_ID);
      PostprocessorIterator postprocessor_end = _moose_system._pps[tid].elementPostprocessorsEnd(Moose::ANY_BLOCK_ID);
      PostprocessorIterator postprocessor_it = postprocessor_begin;

      for(postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
        (*postprocessor_it)->execute();

      postprocessor_begin = _moose_system._pps[tid].elementPostprocessorsBegin(cur_subdomain);
      postprocessor_end = _moose_system._pps[tid].elementPostprocessorsEnd(cur_subdomain);
      postprocessor_it = postprocessor_begin;

      for(postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
        (*postprocessor_it)->execute();

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          std::vector<short int> boundary_ids = _moose_system._mesh->boundary_info->boundary_ids (elem, side);

          for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          {
            short int bnd_id = *it;

            PostprocessorIterator side_postprocessor_begin = _moose_system._pps[tid].sidePostprocessorsBegin(bnd_id);
            PostprocessorIterator side_postprocessor_end = _moose_system._pps[tid].sidePostprocessorsEnd(bnd_id);
            PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

            if(side_postprocessor_begin != side_postprocessor_end)
            {
              _moose_system.reinitBCs(tid, _soln, elem, side, bnd_id);

              for(; side_postprocessor_it!=side_postprocessor_end; ++side_postprocessor_it)
                (*side_postprocessor_it)->execute();
            }
          }
        }
      }
    }
  }

protected:
  MooseSystem &_moose_system;
  const NumericVector<Number>& _soln;
};


namespace Moose
{
  void compute_postprocessors(const NumericVector<Number>& soln, NonlinearImplicitSystem& sys)
  {
    MooseSystem * moose_system = sys.get_equation_systems().parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    moose_system->compute_postprocessors(soln);
    moose_system->output_postprocessors();
  }
}

void MooseSystem::compute_postprocessors(const NumericVector<Number>& soln)
{
  Moose::perf_log.push("compute_postprocessors()","Solve");

  // TODO: Make this work with threads!
  if (_pps[0]._block_ids_with_postprocessors.size() > 0 || _pps[0]._boundary_ids_with_postprocessors.size() > 0)
  {
    update_aux_vars(soln);
    
    Threads::parallel_for(*getActiveLocalElementRange(),
                          ComputeInternalPostprocessors(*this, soln));

    // Store element postprocessors values
    std::set<unsigned int>::iterator block_ids_begin = _pps[0]._block_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator block_ids_end = _pps[0]._block_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator block_ids_it = block_ids_begin;

    for(; block_ids_it != block_ids_end; ++block_ids_it)
    {
      unsigned int block_id = *block_ids_it;

      PostprocessorIterator element_postprocessor_begin = _pps[0].elementPostprocessorsBegin(block_id);
      PostprocessorIterator element_postprocessor_end = _pps[0].elementPostprocessorsEnd(block_id);
      PostprocessorIterator element_postprocessor_it = element_postprocessor_begin;

      // Store element postprocessors values
      for(element_postprocessor_it=element_postprocessor_begin;
          element_postprocessor_it!=element_postprocessor_end;
          ++element_postprocessor_it)
      {
        std::string name = (*element_postprocessor_it)->name();
        Real value = (*element_postprocessor_it)->getValue();
        Real time = _t;

        if(!_is_transient)
          time = _t_step;

        _postprocessor_data[0].addData(name, value, time);
      }
    }

    // Store side postprocessors values
    std::set<unsigned int>::iterator boundary_ids_begin = _pps[0]._boundary_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator boundary_ids_end = _pps[0]._boundary_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator boundary_ids_it = boundary_ids_begin;

    for(; boundary_ids_it != boundary_ids_end; ++boundary_ids_it)
    {
      unsigned int boundary_id = *boundary_ids_it;
      
      PostprocessorIterator side_postprocessor_begin = _pps[0].sidePostprocessorsBegin(boundary_id);
      PostprocessorIterator side_postprocessor_end = _pps[0].sidePostprocessorsEnd(boundary_id);
      PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;
    
      for(side_postprocessor_it=side_postprocessor_begin;
          side_postprocessor_it!=side_postprocessor_end;
          ++side_postprocessor_it)
      {
        std::string name = (*side_postprocessor_it)->name();
        Real value = (*side_postprocessor_it)->getValue();
        Real time = _t;
      
        if(!_is_transient)
          time = _t_step;
        
        _postprocessor_data[0].addData(name, value, time);
      }
    }
  }

  // Compute and store generic postprocessors values
  PostprocessorIterator generic_postprocessor_begin = _pps[0].genericPostprocessorsBegin();
  PostprocessorIterator generic_postprocessor_end = _pps[0].genericPostprocessorsEnd();
  PostprocessorIterator generic_postprocessor_it = generic_postprocessor_begin;

  for(generic_postprocessor_it =generic_postprocessor_begin;
      generic_postprocessor_it!=generic_postprocessor_end;
      ++generic_postprocessor_it)
  {
    std::string name = (*generic_postprocessor_it)->name();
    (*generic_postprocessor_it)->initialize();
    (*generic_postprocessor_it)->execute();
    Real value = (*generic_postprocessor_it)->getValue();
    Real time = _t;
    
    if(!_is_transient)
      time = _t_step;
  
    _postprocessor_data[0].addData(name, value, time);
  }

  Moose::perf_log.pop("compute_postprocessors()","Solve");
}

void MooseSystem::output_postprocessors()
{
  // Postprocesser Output
  if (!_postprocessor_data[0].empty())
  {
    if(_postprocessor_screen_output)
    {
      std::cout<<std::endl<<"Postprocessor Values:"<<std::endl;
      _postprocessor_data[0].print_table(std::cout);
      std::cout<<std::endl;
    }
  
    if(_postprocessor_csv_output)
    {
      _postprocessor_data[0].print_csv(_file_base + ".csv");
    }
  }
}
