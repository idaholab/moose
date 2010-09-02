/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

ElementData::ElementData(MooseSystem & moose_system, DofData & dof_data) :
  QuadraturePointData(moose_system, dof_data),
  _moose_system(moose_system)
{
}

ElementData::~ElementData()
{
}


void
ElementData::init()
{
  QuadraturePointData::init();

  _qrule = new QGauss(_moose_system.getDim(), _moose_system._max_quadrature_order);

  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  _var_vals_old_newton.resize(n_vars);
  _var_grads_old_newton.resize(n_vars);

  initKernels();
}


void
ElementData::initKernels()
{
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
ElementData::reinitKernels(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
//  Moose::perf_log.push("reinit()","Kernel");

  QuadraturePointData::reinit(0, soln, elem);

//  Moose::perf_log.pop("reinit()","Kernel");
}

void
ElementData::reinitNewtonStep(const NumericVector<Number>& soln)
{
  unsigned int block_id = 0;

  _n_qpoints = _qrule->n_points();

  std::set<unsigned int>::iterator var_num_it = _var_nums[block_id].begin();
  std::set<unsigned int>::iterator var_num_end = _var_nums[block_id].end();

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    _var_vals_old_newton[var_num].resize(_n_qpoints);
    _var_grads_old_newton[var_num].resize(_n_qpoints);

    FEType fe_type = _moose_system._dof_map->variable_type(var_num);

    const std::vector<std::vector<Real> > & static_phi = *_phi[fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_grad_phi[fe_type];

    computeQpSolutionAll(_var_vals_old_newton[var_num], _var_grads_old_newton[var_num],
                         soln, _dof_data._var_dof_indices[var_num], _n_qpoints, static_phi, static_dphi);
  }
}
