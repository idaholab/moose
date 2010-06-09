//Moose includes
#include "QuadraturePointData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

QuadraturePointData::QuadraturePointData(MooseSystem & moose_system) :
  _moose_system(moose_system)
{
  sizeEverything();
}

QuadraturePointData::~QuadraturePointData()
{
  for (std::vector<std::map<FEType, FEBase*> >::iterator i = _fe.begin(); i != _fe.end(); ++i)
  {
    for (std::map<FEType, FEBase*>::iterator j = i->begin(); j != i->end(); ++j)
      delete j->second;
  }

  for (std::vector<QGauss *>::iterator i = _qrule.begin(); i != _qrule.end(); ++i)
  {
    delete *i;
  }
}

void
QuadraturePointData::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _fe.resize(n_threads);
  _qrule.resize(n_threads);
  _q_point.resize(n_threads);
  _JxW.resize(n_threads);
  _phi.resize(n_threads);
  _dphi.resize(n_threads);
  _d2phi.resize(n_threads);

  _var_vals.resize(n_threads);
  _var_vals_old.resize(n_threads);
  _var_vals_older.resize(n_threads);
  _var_seconds.resize(n_threads);
  _var_grads.resize(n_threads);
  _var_grads_old.resize(n_threads);
  _var_grads_older.resize(n_threads);

  _aux_var_vals.resize(n_threads);
  _aux_var_vals_old.resize(n_threads);
  _aux_var_vals_older.resize(n_threads);
  _aux_var_grads.resize(n_threads);
  _aux_var_grads_old.resize(n_threads);
  _aux_var_grads_older.resize(n_threads);

  _material.resize(n_threads);
}

void
QuadraturePointData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  unsigned int n_aux_vars = _moose_system.getAuxSystem()->n_vars();

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _var_vals[tid].resize(n_vars);
    _var_vals_old[tid].resize(n_vars);
    _var_vals_older[tid].resize(n_vars);
    _var_seconds[tid].resize(n_vars);
    _var_grads[tid].resize(n_vars);
    _var_grads_old[tid].resize(n_vars);
    _var_grads_older[tid].resize(n_vars);

    _aux_var_vals[tid].resize(n_aux_vars);
    _aux_var_vals_old[tid].resize(n_aux_vars);
    _aux_var_vals_older[tid].resize(n_aux_vars);
    _aux_var_grads[tid].resize(n_aux_vars);
    _aux_var_grads_old[tid].resize(n_aux_vars);
    _aux_var_grads_older[tid].resize(n_aux_vars);
  }
}

void
QuadraturePointData::reinit(THREAD_ID tid, unsigned int block_id, const NumericVector<Number>& soln, const Elem * elem)
{
  // set the number of quadrature points
  _n_qpoints = _qrule[tid]->n_points();

  std::vector<unsigned int>::iterator var_num_it = _var_nums[block_id].begin();
  std::vector<unsigned int>::iterator var_num_end = _var_nums[block_id].end();

//  Moose::perf_log.push("reinit() - compute vals","Kernel");

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _moose_system._dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    _var_vals[tid][var_num].resize(_n_qpoints);
    _var_grads[tid][var_num].resize(_n_qpoints);

    if(has_second_derivatives)
      _var_seconds[tid][var_num].resize(_n_qpoints);

    if(_moose_system._is_transient)
    {
      _var_vals_old[tid][var_num].resize(_n_qpoints);
      _var_grads_old[tid][var_num].resize(_n_qpoints);

      _var_vals_older[tid][var_num].resize(_n_qpoints);
      _var_grads_older[tid][var_num].resize(_n_qpoints);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi= *_d2phi[tid][fe_type];

    if (_moose_system._is_transient)
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             _var_seconds[tid][var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _moose_system._var_dof_indices[tid][var_num], _n_qpoints,
                             static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _moose_system._var_dof_indices[tid][var_num], _n_qpoints,
                             static_phi, static_dphi);
    }
    else
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],  _var_seconds[tid][var_num],
                             soln, _moose_system._var_dof_indices[tid][var_num], _n_qpoints, static_phi, static_dphi, static_d2phi);
      else {
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],
                             soln, _moose_system._var_dof_indices[tid][var_num], _n_qpoints, static_phi, static_dphi);
      }
    }
  }

//  Moose::perf_log.pop("reinit() - compute vals","Kernel");
//  Moose::perf_log.push("reinit() - compute aux vals","Kernel");
  const NumericVector<Number>& aux_soln = (*_moose_system.getAuxSystem()->current_local_solution);

  std::vector<unsigned int>::iterator aux_var_num_it = _aux_var_nums[0].begin();
  std::vector<unsigned int>::iterator aux_var_num_end = _aux_var_nums[0].end();

  for(;aux_var_num_it != aux_var_num_end; ++aux_var_num_it)
  {
    unsigned int var_num = *aux_var_num_it;

    FEType fe_type = _moose_system._aux_dof_map->variable_type(var_num);

    _moose_system._aux_dof_map->dof_indices(elem, _moose_system._aux_var_dof_indices[tid][var_num], var_num);

    _aux_var_vals[tid][var_num].resize(_n_qpoints);
    _aux_var_grads[tid][var_num].resize(_n_qpoints);

    if(_moose_system._is_transient)
    {
      _aux_var_vals_old[tid][var_num].resize(_n_qpoints);
      _aux_var_grads_old[tid][var_num].resize(_n_qpoints);

      _aux_var_vals_older[tid][var_num].resize(_n_qpoints);
      _aux_var_grads_older[tid][var_num].resize(_n_qpoints);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];


    if (_moose_system._is_transient)
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_vals_old[tid][var_num], _aux_var_vals_older[tid][var_num],
                           _aux_var_grads[tid][var_num], _aux_var_grads_old[tid][var_num], _aux_var_grads_older[tid][var_num],
                           aux_soln, *_moose_system.getAuxSystem()->old_local_solution, *_moose_system.getAuxSystem()->older_local_solution,
                           _moose_system._aux_var_dof_indices[tid][var_num], _n_qpoints, static_phi, static_dphi);
    }
    else
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_grads[tid][var_num],
                           aux_soln, _moose_system._aux_var_dof_indices[tid][var_num], _n_qpoints, static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute aux vals","Kernel");

//  Moose::perf_log.push("reinit() - material","Kernel");

  _material[tid] = _moose_system.getMaterial(tid,elem->subdomain_id());
  _material[tid]->materialReinit();

//  Moose::perf_log.pop("reinit() - material","Kernel");
}
