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
#include "MooseSystem.h"
#include "AuxData.h"
#include "DofData.h"
#include "ElementData.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

AuxData::AuxData(MooseSystem & moose_system, DofData & dof_data, ElementData & element_data) :
  _moose_system(moose_system),
  _dof_data(dof_data),
  _element_data(element_data),
  _element_var_nums(_element_data._aux_var_nums),
  _var_vals_element(_element_data._var_vals),
  _var_vals_old_element(_element_data._var_vals_old),
  _var_vals_older_element(_element_data._var_vals_older),
  _var_grads_element(_element_data._var_grads),
  _var_grads_old_element(_element_data._var_grads_old),
  _var_grads_older_element(_element_data._var_grads_older),

  _aux_var_vals_element(_element_data._aux_var_vals),
  _aux_var_vals_old_element(_element_data._aux_var_vals_old),
  _aux_var_vals_older_element(_element_data._aux_var_vals_older),
  _aux_var_grads_element(_element_data._aux_var_grads),
  _aux_var_grads_old_element(_element_data._aux_var_grads_old),
  _aux_var_grads_older_element(_element_data._aux_var_grads_older)
{
}

AuxData::~AuxData()
{

}

void AuxData::init()
{
  TransientNonlinearImplicitSystem *system = _moose_system.getNonlinearSystem();
  TransientExplicitSystem *aux_system = _moose_system.getAuxSystem();

  _nonlinear_old_soln = system->old_local_solution.get();
  _nonlinear_older_soln = system->older_local_solution.get();

  _aux_soln = aux_system->solution.get();
  _aux_old_soln = aux_system->old_local_solution.get();
  _aux_older_soln = aux_system->older_local_solution.get();

  unsigned int n_vars = system->n_vars();
  unsigned int n_aux_vars = aux_system->n_vars();

  _aux_var_vals_nodal.resize(n_aux_vars);
  _aux_var_vals_old_nodal.resize(n_aux_vars);
  _aux_var_vals_older_nodal.resize(n_aux_vars);

  _var_vals_nodal.resize(n_vars);
  _var_vals_old_nodal.resize(n_vars);
  _var_vals_older_nodal.resize(n_vars);
}

void AuxData::reinit(const NumericVector<Number>& soln, const Node & node)
{
//  Moose::perf_log.push("reinit(node)", "AuxKernel");

  unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();
  unsigned int aux_system_number = _moose_system.getAuxSystem()->number();

  // Only ever one "qp" for aux kernels
  unsigned int qp = 0;

  //Non Aux vars first
  for (std::set<unsigned int>::iterator it = _element_data._var_nums.begin(); it != _element_data._var_nums.end(); ++it)
  {
    unsigned int var_num = *it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _var_vals_nodal[var_num].resize(1);
    _var_vals_nodal[var_num][qp] = soln(dof_number);

    if (_moose_system._is_transient)
    {
      _var_vals_old_nodal[var_num].resize(1);
      _var_vals_older_nodal[var_num].resize(1);
      
      _var_vals_old_nodal[var_num][qp] = (*_nonlinear_old_soln)(dof_number);
      _var_vals_older_nodal[var_num][qp] = (*_nonlinear_older_soln)(dof_number);
    }
  }

  //Now Nodal Aux vars
  for (std::set<unsigned int>::iterator it = _nodal_var_nums.begin(); it != _nodal_var_nums.end(); ++it)
  {
    unsigned int var_num = *it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(aux_system_number, var_num, 0);

    _dof_data._aux_var_dofs[var_num] = dof_number;
    _aux_var_vals_nodal[var_num].resize(1);
    _aux_var_vals_nodal[var_num][qp] = (*_aux_soln)(dof_number);

    if (_moose_system._is_transient)
    {
      _aux_var_vals_old_nodal[var_num].resize(1);
      _aux_var_vals_older_nodal[var_num].resize(1);
      
      _aux_var_vals_old_nodal[var_num][qp] = (*_aux_old_soln)(dof_number);
      _aux_var_vals_older_nodal[var_num][qp] = (*_aux_older_soln)(dof_number);
    }
  }

//  Moose::perf_log.pop("reinit(node)","AuxKernel");
}

void AuxData::reinit(const NumericVector<Number>& soln, const Elem & elem)
{
//  Moose::perf_log.push("reinit(elem)", "AuxKernel");

  unsigned int aux_system_number = _moose_system.getAuxSystem()->number();

  //Compute the area of the element
  _current_volume = 0;
  //Just use any old JxW... they are all actually the same
  const std::vector<Real> & jxw = *_element_data._JxW.begin()->second;

  if (_moose_system._geom_type == Moose::XYZ)
  {
    for (unsigned int qp = 0; qp < _element_data._qrule->n_points(); qp++)
      _current_volume += jxw[qp];
  }
  else if (_moose_system._geom_type == Moose::CYLINDRICAL)
  {
    const std::vector<Point> & q_point = *(_element_data._q_point.begin()->second);
    for (unsigned int qp = 0; qp < _element_data._qrule->n_points(); qp++)
      _current_volume += q_point[qp](0) * jxw[qp];
  }
  else
    mooseError("geom_type must either be XYZ or CYLINDRICAL\n");

  //Grab the dof numbers for the element variables
  for (std::set<unsigned int>::iterator it = _element_var_nums.begin(); it != _element_var_nums.end(); ++it)
  {
    unsigned int var_num = *it;

    //The zero is the component... that works fine for FIRST order monomials
    unsigned int dof_number = elem.dof_number(aux_system_number, var_num, 0);

    _dof_data._aux_var_dofs[var_num] = dof_number;
  }

//  Moose::perf_log.pop("reinit(elem)", "AuxKernel");
}
