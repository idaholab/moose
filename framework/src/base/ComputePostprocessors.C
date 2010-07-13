//Moose Includes
#include "Moose.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "ElementData.h"

//libMesh includes
#include "numeric_vector.h"
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

    PostprocessorIterator postprocessor_begin = _moose_system._pps.elementPostprocessorsBegin(tid);
    PostprocessorIterator postprocessor_end = _moose_system._pps.elementPostprocessorsEnd(tid);
    PostprocessorIterator postprocessor_it = postprocessor_begin;

    unsigned int subdomain = 999999999;

    //Global Postprocessors
    for(postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
      (*postprocessor_it)->initialize();

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      _moose_system.reinitKernels(tid, _soln, elem, NULL);

      unsigned int cur_subdomain = elem->subdomain_id();

      //Global Postprocessors
      for(postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
        (*postprocessor_it)->execute();

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = _moose_system._mesh->boundary_info->boundary_id (elem, side);

          PostprocessorIterator side_postprocessor_begin = _moose_system._pps.sidePostprocessorsBegin(tid,boundary_id);
          PostprocessorIterator side_postprocessor_end = _moose_system._pps.sidePostprocessorsEnd(tid,boundary_id);
          PostprocessorIterator side_postprocessor_it = postprocessor_begin;

          if(side_postprocessor_begin != side_postprocessor_end)
          {
            _moose_system.reinitBCs(tid, _soln, elem, side, boundary_id);
            
            for(; side_postprocessor_it!=side_postprocessor_end; ++side_postprocessor_it)
              (*side_postprocessor_it)->execute();
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
  }
}

void MooseSystem::compute_postprocessors(const NumericVector<Number>& soln)
{
  Moose::perf_log.push("compute_postprocessors()","Solve");

  // TODO: Make this work with threads!
  PostprocessorIterator postprocessor_begin = _pps.elementPostprocessorsBegin(0);
  PostprocessorIterator postprocessor_end = _pps.elementPostprocessorsEnd(0);
  PostprocessorIterator postprocessor_it = postprocessor_begin;

  if(postprocessor_begin != postprocessor_end)
  {
    update_aux_vars(soln);
    
    Threads::parallel_for(*getActiveLocalElementRange(),
                          ComputeInternalPostprocessors(*this, soln));



    for(postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
    {
      std::string name = (*postprocessor_it)->name();
      Real value = (*postprocessor_it)->getValue();
      Real time = _t;

      if(!_is_transient)
        time = _t_step;
    
      _postprocessor_data._values[name] = value;
      _postprocessor_data._output_table.addData(name, value, _t);
    }

    // Postprocesser Output
    if (_postprocessor_screen_output)
    {
      std::cout<<std::endl<<"Postprocessor Values:"<<std::endl;
      _postprocessor_data._output_table.print_table(std::cout);
      std::cout<<std::endl;
    }
    if (_postprocessor_csv_output)
    {
      _postprocessor_data._output_table.print_csv(_file_base + ".csv");
    }
  }

  Moose::perf_log.pop("compute_postprocessors()","Solve");
}

