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
#include "DiracKernelWarehouse.h"
#include "ComputeBase.h"
#include "DiracKernel.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"
#include "fe_interface.h"


#include <vector>
typedef StoredRange<std::set<const Elem *>::const_iterator, const Elem *> DistElemRange;
class ComputeDiracKernels : public ComputeBase<DistElemRange>
{
public:
  ComputeDiracKernels(MooseSystem &sys, const NumericVector<Number>& in_soln, NumericVector<Number>& residual)
    : ComputeBase<DistElemRange>(sys),
      _soln(in_soln),
      _residual(residual)
  {}

  // Splitting Constructor
  ComputeDiracKernels(ComputeDiracKernels & x, Threads::split)
    : ComputeBase<DistElemRange>(x._moose_system),
      _soln(x._soln),
      _residual(x._residual)
  {}

  virtual void preElement(const Elem * /*elem*/)
  {
    _re.zero();
  }
  
  virtual void onElement(const Elem *elem)
  {    
    std::set<Point> & points = _moose_system._dirac_kernel_info._points[elem];

    if(points.size())
    {
      std::set<Point>::iterator pit = points.begin();
      std::set<Point>::iterator pend = points.end();

      std::vector<Point> points_vec;
      points_vec.reserve(points.size());

      for(; pit != pend; ++pit)
      {
        Point p = *pit;
        points_vec.push_back(p);
      }

      _moose_system.reinitDiracKernels(_tid, _soln, elem, points_vec, &_re, NULL);
    
      DiracKernelIterator dirac_kernel_begin = _moose_system._dirac_kernels[_tid].diracKernelsBegin();
      DiracKernelIterator dirac_kernel_end = _moose_system._dirac_kernels[_tid].diracKernelsEnd();
      DiracKernelIterator dirac_kernel_it = dirac_kernel_begin;

      for(dirac_kernel_it=dirac_kernel_begin;dirac_kernel_it!=dirac_kernel_end;++dirac_kernel_it)
      {
        DiracKernel * dirac = *dirac_kernel_it;

        if(dirac->hasPointsOnElem(elem))
          dirac->computeResidual();
      }
    }
  }

  virtual void postElement(const Elem * /*elem*/)
  {
    _moose_system._dof_map->constrain_element_vector (_re, _moose_system._dof_data[_tid]._dof_indices, false);
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _residual.add_vector(_re, _moose_system._dof_data[_tid]._dof_indices);
    }
  }

  void join(const ComputeDiracKernels & /*y*/)
  {
  }

protected:
  const NumericVector<Number> & _soln;
  NumericVector<Number> & _residual;

  DenseVector<Number> _re;
};

void MooseSystem::computeDiracKernels(const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  Moose::perf_log.push("compute_dirac_kernels()","Solve");

  // Default to no dirac_kernels
  // Real dirac_kernels = 1.0;

  DiracKernelIterator dirac_kernel_begin = _dirac_kernels[0].diracKernelsBegin();
  DiracKernelIterator dirac_kernel_end = _dirac_kernels[0].diracKernelsEnd();
  DiracKernelIterator dirac_kernel_it = dirac_kernel_begin;

  for(dirac_kernel_it=dirac_kernel_begin;dirac_kernel_it!=dirac_kernel_end;++dirac_kernel_it)
    (*dirac_kernel_it)->addPoints();

  if(dirac_kernel_begin != dirac_kernel_end)
  {
    ComputeDiracKernels cd(*this, soln, residual);

    DistElemRange range(_dirac_kernel_info._elements.begin(),
                        _dirac_kernel_info._elements.end(),
                        1);
    
    Threads::parallel_reduce(range, cd);
  }

  residual.close();

  Moose::perf_log.pop("compute_dirac_kernels()","Solve");
}

