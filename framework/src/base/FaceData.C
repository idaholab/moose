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
#include "FaceData.h"
#include "DofData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "fe_base.h"
#include "numeric_vector.h"
#include "quadrature_gauss.h"

FaceData::FaceData(MooseSystem & moose_system, DofData & dof_data) :
  QuadraturePointData(moose_system, dof_data),
  _moose_system(moose_system),
  _dof_data(dof_data),
  _current_side_elem(NULL)
{
}

FaceData::~FaceData()
{
  delete _current_side_elem;
}

void FaceData::init()
{
  QuadraturePointData::init();

  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  int dim = _moose_system.getDim();

  //Resize data arrays
  _nodal_bc_var_dofs.resize(n_vars);
  _var_vals_nodal.resize(n_vars);

  //Max quadrature order was already found by Kernel::init()
  _qrule = new QGauss(dim - 1, _moose_system._max_quadrature_order);

  for(unsigned int var=0; var < n_vars; var++)
  {
    // TODO: Replicate dof_map
    FEType fe_type = _moose_system._dof_map->variable_type(var);

    if(!_fe[fe_type])
    {
      _fe[fe_type] = FEBase::build(dim, fe_type).release();
      _fe[fe_type]->attach_quadrature_rule(_qrule);

      _q_point[fe_type] = &_fe[fe_type]->get_xyz();
      _JxW[fe_type] = &_fe[fe_type]->get_JxW();
      _phi[fe_type] = &_fe[fe_type]->get_phi();
      _grad_phi[fe_type] = &_fe[fe_type]->get_dphi();
      _normals[fe_type] = &_fe[fe_type]->get_normals();

      FEFamily family = fe_type.family;

      if(family == CLOUGH || family == HERMITE)
        _second_phi[fe_type] = &_fe[fe_type]->get_d2phi();
    }
  }

}

void FaceData::reinit(const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side = side;

  if(_current_side_elem)
    delete _current_side_elem;
  
  _current_side_elem = elem->build_side(side).release();

  std::map<FEType, FEBase*>::iterator fe_it = _fe.begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe.end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(elem, _current_side);

  QuadraturePointData::reinit(boundary_id, soln, elem);

  for (std::set<unsigned int>::iterator it = _boundary_to_var_nums_nodal[boundary_id].begin();
       it != _boundary_to_var_nums_nodal[boundary_id].end();
       ++it)
  {
    unsigned int var_num = *it;

    std::vector<unsigned int> & var_dof_indices = _dof_data._var_dof_indices[var_num];

    _var_vals_nodal[var_num].resize(_dof_data._current_elem->n_nodes());

    for(unsigned int i=0; i<_dof_data._current_elem->n_nodes(); i++)
      _var_vals_nodal[var_num][i] = soln(var_dof_indices[i]);
  }

//  Moose::perf_log.pop("reinit()","BoundaryCondition");
}

void FaceData::reinit(const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
//  Moose::perf_log.push("reinit(node)","BoundaryCondition");

  _current_node = &node;
  _current_residual = &residual;

  unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();

  for (std::set<unsigned int>::iterator it = _boundary_to_var_nums_nodal[boundary_id].begin();
       it != _boundary_to_var_nums_nodal[boundary_id].end();
       ++it)
  {
    unsigned int var_num = *it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _nodal_bc_var_dofs[var_num] = dof_number;

    _var_vals_nodal[var_num].resize(1);

    _var_vals_nodal[var_num][0] = soln(dof_number);
  }

//  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}
