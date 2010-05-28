//Moose includes
#include "ElementData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

ElementData::ElementData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{
  sizeEverything();
}

ElementData::~ElementData()
{
  for (std::vector<std::vector<DenseSubVector<Number> *> >::iterator i = _var_Res.begin(); i != _var_Res.end(); ++i)
  {
    for (std::vector<DenseSubVector<Number> *>::iterator j = i->begin(); j != i->end(); ++j)
    {
      delete *j;
    }
  }

  for (std::vector<std::vector<DenseMatrix<Number> *> >::iterator i = _var_Kes.begin(); i != _var_Kes.end(); ++i)
  {
    for (std::vector<DenseMatrix<Number> *>::iterator j = i->begin(); j != i->end(); ++j)
    {
      delete *j;
    }
  }

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
ElementData::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _current_elem.resize(n_threads);
  _dof_indices.resize(n_threads);
  _aux_var_dof_indices.resize(n_threads);
  _fe.resize(n_threads);
  _qrule.resize(n_threads);
  
  _JxW.resize(n_threads);
  _phi.resize(n_threads);
  _test.resize(n_threads);
  _dphi.resize(n_threads);
  _d2phi.resize(n_threads);
  _q_point.resize(n_threads);
  _var_dof_indices.resize(n_threads);
  _var_Res.resize(n_threads);
  _var_Kes.resize(n_threads);
  _var_vals.resize(n_threads);
  _var_grads.resize(n_threads);
  _var_seconds.resize(n_threads);
  _var_vals_old.resize(n_threads);
  _var_vals_older.resize(n_threads);
  _var_grads_old.resize(n_threads);
  _var_grads_older.resize(n_threads);

  _aux_var_dof_indices.resize(n_threads);
  _aux_var_dofs.resize(n_threads);
  _aux_var_vals.resize(n_threads);
  _aux_var_grads.resize(n_threads);
  _aux_var_vals_old.resize(n_threads);
  _aux_var_vals_older.resize(n_threads);
  _aux_var_grads_old.resize(n_threads);
  _aux_var_grads_older.resize(n_threads);
}


void
ElementData::init()
{
  _dof_map = &_moose_system.getNonlinearSystem()->get_dof_map();
  _aux_dof_map = &_moose_system.getAuxSystem()->get_dof_map();

  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  unsigned int n_aux_vars = _moose_system.getAuxSystem()->n_vars();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    // kernels
    _var_dof_indices[tid].resize(n_vars);
    _var_Res[tid].resize(n_vars);
    _var_Kes[tid].resize(n_vars);
    _var_vals[tid].resize(n_vars);
    _var_grads[tid].resize(n_vars);
    _var_seconds[tid].resize(n_vars);
    _var_vals_old[tid].resize(n_vars);
    _var_vals_older[tid].resize(n_vars);
    _var_grads_old[tid].resize(n_vars);
    _var_grads_older[tid].resize(n_vars);

    // aux var
    _aux_var_dofs[tid].resize(n_aux_vars);
    _aux_var_dof_indices[tid].resize(n_aux_vars);
    _aux_var_vals[tid].resize(n_aux_vars);
    _aux_var_vals_old[tid].resize(n_aux_vars);
    _aux_var_vals_older[tid].resize(n_aux_vars);
    _aux_var_grads[tid].resize(n_aux_vars);
    _aux_var_grads_old[tid].resize(n_aux_vars);
    _aux_var_grads_older[tid].resize(n_aux_vars);
  }

  _moose_system._max_quadrature_order = CONSTANT;
  
  //Set the default variable scaling to 1
  for(unsigned int i=0; i < _moose_system.getNonlinearSystem()->n_vars(); i++)
    _scaling_factor.push_back(1.0);

  //Find the largest quadrature order necessary... all variables _must_ use the same rule!
  for(unsigned int var=0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);
    if(fe_type.default_quadrature_order() > _moose_system._max_quadrature_order)
      _moose_system._max_quadrature_order = fe_type.default_quadrature_order();
  }

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qrule[tid] = new QGauss(_moose_system.getDim(), _moose_system._max_quadrature_order);

  initKernels();
}


void
ElementData::initKernels()
{
  //This allows for different basis functions / orders for each variable
  for(unsigned int var=0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _d2phi[tid][fe_type] = &_fe[tid][fe_type]->get_d2phi();
      }
    }
  }

  //This allows for different basis functions / orders for each Aux variable
  for(unsigned int var=0; var < _moose_system.getAuxSystem()->n_vars(); var++)
  {
    FEType fe_type = _moose_system._element_data._aux_dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();
      }
    }
  }
}


void
ElementData::setVarScaling(std::vector<Real> scaling)
{
  if(scaling.size() != _moose_system.getNonlinearSystem()->n_vars())
  {
    std::cout<<"Error: size of scaling factor vector not the same as the number of variables in the system!"<<std::endl;
    mooseError("");
  }

  _scaling_factor = scaling;
}


void
ElementData::reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
//  Moose::perf_log.push("reinit()","Kernel");
//  Moose::perf_log.push("reinit() - dof_indices","Kernel");

  _current_elem[tid] = elem;

  _dof_map->dof_indices(elem, _dof_indices[tid]);

  std::map<FEType, FEBase*>::iterator fe_it = _fe[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe[tid].end();

//  Moose::perf_log.pop("reinit() - dof_indices","Kernel");
//  Moose::perf_log.push("reinit() - fereinit","Kernel");

  static std::vector<bool> first(libMesh::n_threads(), true);

  if(_moose_system.dontReinitFE())
  {
    if(first[tid])
    {
      for(;fe_it != fe_end; ++fe_it)
        fe_it->second->reinit(elem);
    }
  }
  else
  {
    for(;fe_it != fe_end; ++fe_it)
      fe_it->second->reinit(elem);
  }

  first[tid] = false;

//  Moose::perf_log.pop("reinit() - fereinit","Kernel");

//  Moose::perf_log.push("reinit() - resizing","Kernel");
  if(Re)
    Re->resize(_dof_indices[tid].size());

  if(Ke)
    Ke->resize(_dof_indices[tid].size(),_dof_indices[tid].size());

  unsigned int position = 0;

  for(unsigned int i=0; i<_var_nums.size();i++)
  {
    _dof_map->dof_indices(elem, _var_dof_indices[tid][i], i);

    unsigned int num_dofs = _var_dof_indices[tid][i].size();

    if(Re)
    {
      if(_var_Res[tid][i])
        delete _var_Res[tid][i];

      _var_Res[tid][i] = new DenseSubVector<Number>(*Re,position, num_dofs);
    }

    if(Ke)
    {
      if(_var_Kes[tid][i])
        delete _var_Kes[tid][i];

      _var_Kes[tid][i] = new DenseMatrix<Number>(num_dofs,num_dofs);
    }
    position+=num_dofs;
  }

  unsigned int num_q_points = _qrule[tid]->n_points();

  _moose_system._real_zero[tid] = 0;
  _moose_system._zero[tid].resize(num_q_points,0);
  _moose_system._grad_zero[tid].resize(num_q_points,0);
  _moose_system._second_zero[tid].resize(num_q_points,0);

  std::vector<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::vector<unsigned int>::iterator var_num_end = _var_nums.end();

//  Moose::perf_log.pop("reinit() - resizing","Kernel");
//  Moose::perf_log.push("reinit() - compute vals","Kernel");

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    _var_vals[tid][var_num].resize(num_q_points);
    _var_grads[tid][var_num].resize(num_q_points);

    if(has_second_derivatives)
      _var_seconds[tid][var_num].resize(num_q_points);

    if(_moose_system._is_transient)
    {
      _var_vals_old[tid][var_num].resize(num_q_points);
      _var_grads_old[tid][var_num].resize(num_q_points);

      _var_vals_older[tid][var_num].resize(num_q_points);
      _var_grads_older[tid][var_num].resize(num_q_points);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi= *_d2phi[tid][fe_type];

    // Copy phi to the test functions.
    _test[tid][var_num] = static_phi;

    if (_moose_system._is_transient)
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             _var_seconds[tid][var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _var_dof_indices[tid][var_num], _qrule[tid]->n_points(),
                             static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _var_dof_indices[tid][var_num], _qrule[tid]->n_points(),
                             static_phi, static_dphi);
    }
    else
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],  _var_seconds[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute vals","Kernel");
//  Moose::perf_log.push("reinit() - compute aux vals","Kernel");
  const NumericVector<Number>& aux_soln = (*_moose_system.getAuxSystem()->current_local_solution);

  std::vector<unsigned int>::iterator aux_var_num_it = _aux_var_nums.begin();
  std::vector<unsigned int>::iterator aux_var_num_end = _aux_var_nums.end();

  for(;aux_var_num_it != aux_var_num_end; ++aux_var_num_it)
  {
    unsigned int var_num = *aux_var_num_it;

    FEType fe_type = _aux_dof_map->variable_type(var_num);

    _aux_dof_map->dof_indices(elem, _aux_var_dof_indices[tid][var_num], var_num);

    _aux_var_vals[tid][var_num].resize(num_q_points);
    _aux_var_grads[tid][var_num].resize(num_q_points);

    if(_moose_system._is_transient)
    {
      _aux_var_vals_old[tid][var_num].resize(num_q_points);
      _aux_var_grads_old[tid][var_num].resize(num_q_points);

      _aux_var_vals_older[tid][var_num].resize(num_q_points);
      _aux_var_grads_older[tid][var_num].resize(num_q_points);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];


    if (_moose_system._is_transient)
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_vals_old[tid][var_num], _aux_var_vals_older[tid][var_num],
                           _aux_var_grads[tid][var_num], _aux_var_grads_old[tid][var_num], _aux_var_grads_older[tid][var_num],
                           aux_soln, *_moose_system.getAuxSystem()->old_local_solution, *_moose_system.getAuxSystem()->older_local_solution,
                           _aux_var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
    else
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_grads[tid][var_num],
                           aux_soln, _aux_var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute aux vals","Kernel");
//  Moose::perf_log.pop("reinit()","Kernel");
}

