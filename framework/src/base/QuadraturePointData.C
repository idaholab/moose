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

//Moose includes
#include "QuadraturePointData.h"
#include "MooseSystem.h"
#include "DofData.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

QuadraturePointData::QuadraturePointData(MooseSystem & moose_system, DofData & dof_data) :
  _moose_system(moose_system),
  _dof_data(dof_data),
  _qrule(NULL)
{
}

QuadraturePointData::~QuadraturePointData()
{
  for (std::map<FEType, FEBase*>::iterator i = _fe.begin(); i != _fe.end(); ++i)
    delete i->second;

  for (std::map<FEType, FEBase*>::iterator i = _fe_displaced.begin(); i != _fe_displaced.end(); ++i)
    delete i->second;

  delete _qrule;
}

void
QuadraturePointData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  unsigned int n_aux_vars = _moose_system.getAuxSystem()->n_vars();

  _var_vals.resize(n_vars);
  _var_vals_old.resize(n_vars);
  _var_vals_older.resize(n_vars);
  _var_seconds.resize(n_vars);
  _var_grads.resize(n_vars);
  _var_grads_old.resize(n_vars);
  _var_grads_older.resize(n_vars);
  _var_dots.resize(n_vars);
  _var_du_dot_dus.resize(n_vars);

  _aux_var_vals.resize(n_aux_vars);
  _aux_var_vals_old.resize(n_aux_vars);
  _aux_var_vals_older.resize(n_aux_vars);
  _aux_var_grads.resize(n_aux_vars);
  _aux_var_grads_old.resize(n_aux_vars);
  _aux_var_grads_older.resize(n_aux_vars);

  _var_vals_old_newton.resize(n_vars);
  _var_grads_old_newton.resize(n_vars);

  //This allows for different basis functions / orders for each variable
  for(unsigned int var=0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
  {
    FEType fe_type = _moose_system._dof_map->variable_type(var);

    if(!_fe[fe_type])
    {
      _fe[fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
      _fe[fe_type]->attach_quadrature_rule(_qrule);

      _fe_displaced[fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
      _fe_displaced[fe_type]->attach_quadrature_rule(_qrule);

      _JxW[fe_type] = &_fe[fe_type]->get_JxW();
      _JxW_displaced[fe_type] = &_fe_displaced[fe_type]->get_JxW();

      _phi[fe_type] = &_fe[fe_type]->get_phi();
      _grad_phi[fe_type] = &_fe[fe_type]->get_dphi();

      _q_point[fe_type] = &_fe[fe_type]->get_xyz();
      _q_point_displaced[fe_type] = &_fe_displaced[fe_type]->get_xyz();

      FEFamily family = fe_type.family;

      if(family == CLOUGH || family == HERMITE)
        _second_phi[fe_type] = &_fe[fe_type]->get_d2phi();
    }
  }

  //This allows for different basis functions / orders for each Aux variable
  for(unsigned int var=0; var < _moose_system.getAuxSystem()->n_vars(); var++)
  {
    FEType fe_type = _moose_system._aux_dof_map->variable_type(var);

    if(!_fe[fe_type])
    {
      _fe[fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
      _fe[fe_type]->attach_quadrature_rule(_qrule);

      _JxW[fe_type] = &_fe[fe_type]->get_JxW();
      _phi[fe_type] = &_fe[fe_type]->get_phi();
      _grad_phi[fe_type] = &_fe[fe_type]->get_dphi();
      _q_point[fe_type] = &_fe[fe_type]->get_xyz();
    }
  }
}

void
QuadraturePointData::reinit(const NumericVector<Number>& soln, const Elem * elem)
{
  // set the number of quadrature points
  _n_qpoints = _qrule->n_points();

  std::set<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::set<unsigned int>::iterator var_num_end = _var_nums.end();

//  Moose::perf_log.push("reinit() - compute vals","Kernel");

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _moose_system._dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    _var_vals[var_num].resize(_n_qpoints);
    _var_grads[var_num].resize(_n_qpoints);

    if(has_second_derivatives)
      _var_seconds[var_num].resize(_n_qpoints);

    if(_moose_system._is_transient)
    {
      _var_dots[var_num].resize(_n_qpoints);
      _var_du_dot_dus[var_num].resize(_n_qpoints);

      _var_vals_old[var_num].resize(_n_qpoints);
      _var_grads_old[var_num].resize(_n_qpoints);

      _var_vals_older[var_num].resize(_n_qpoints);
      _var_grads_older[var_num].resize(_n_qpoints);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_grad_phi[fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi= *_second_phi[fe_type];

    if (_moose_system._is_transient)
    {
      std::vector<unsigned int> & dof_indices = _dof_data._var_dof_indices[var_num];
      MooseArray<Real> & u_dot = _var_dots[var_num];
      MooseArray<Real> & du_dot_du = _var_du_dot_dus[var_num];

      // Compute the increment at each quadrature point
      for(unsigned int qp=0; qp<_n_qpoints; qp++)
      {
        computeQpSolution(u_dot[qp], *_moose_system._u_dot_soln, dof_indices, qp, static_phi);
        computeQpSolution(du_dot_du[qp], *_moose_system._du_dot_du_soln, dof_indices, qp, static_phi);
      }

      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[var_num], _var_vals_old[var_num], _var_vals_older[var_num],
                             _var_grads[var_num], _var_grads_old[var_num], _var_grads_older[var_num],
                             _var_seconds[var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _dof_data._var_dof_indices[var_num], _n_qpoints,
                             static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[var_num], _var_vals_old[var_num], _var_vals_older[var_num],
                             _var_grads[var_num], _var_grads_old[var_num], _var_grads_older[var_num],
                             soln, *_moose_system.getNonlinearSystem()->old_local_solution, *_moose_system.getNonlinearSystem()->older_local_solution,
                             _dof_data._var_dof_indices[var_num], _n_qpoints,
                             static_phi, static_dphi);
    }
    else
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[var_num], _var_grads[var_num],  _var_seconds[var_num],
                             soln, _dof_data._var_dof_indices[var_num], _n_qpoints, static_phi, static_dphi, static_d2phi);
      else {
        computeQpSolutionAll(_var_vals[var_num], _var_grads[var_num],
                             soln, _dof_data._var_dof_indices[var_num], _n_qpoints, static_phi, static_dphi);
      }
    }
  }

//  Moose::perf_log.pop("reinit() - compute vals","Kernel");
//  Moose::perf_log.push("reinit() - compute aux vals","Kernel");
  const NumericVector<Number>& aux_soln = (*_moose_system.getAuxSystem()->current_local_solution);

  for (std::set<unsigned int>::iterator it = _aux_var_nums.begin();
      it != _aux_var_nums.end();
      ++it)
  {
    unsigned int var_num = *it;

    FEType fe_type = _moose_system._aux_dof_map->variable_type(var_num);

    _moose_system._aux_dof_map->dof_indices(elem, _dof_data._aux_var_dof_indices[var_num], var_num);

    _aux_var_vals[var_num].resize(_n_qpoints);
    _aux_var_grads[var_num].resize(_n_qpoints);

    if(_moose_system._is_transient)
    {
      _aux_var_vals_old[var_num].resize(_n_qpoints);
      _aux_var_grads_old[var_num].resize(_n_qpoints);

      _aux_var_vals_older[var_num].resize(_n_qpoints);
      _aux_var_grads_older[var_num].resize(_n_qpoints);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_grad_phi[fe_type];


    if (_moose_system._is_transient)
    {
      computeQpSolutionAll(_aux_var_vals[var_num], _aux_var_vals_old[var_num], _aux_var_vals_older[var_num],
                           _aux_var_grads[var_num], _aux_var_grads_old[var_num], _aux_var_grads_older[var_num],
                           aux_soln, *_moose_system.getAuxSystem()->old_local_solution, *_moose_system.getAuxSystem()->older_local_solution,
                           _dof_data._aux_var_dof_indices[var_num], _n_qpoints, static_phi, static_dphi);
    }
    else
    {
      computeQpSolutionAll(_aux_var_vals[var_num], _aux_var_grads[var_num],
                           aux_soln, _dof_data._aux_var_dof_indices[var_num], _n_qpoints, static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute aux vals","Kernel");
}

