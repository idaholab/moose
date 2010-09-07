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
#include "DamperData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

DamperData::DamperData(MooseSystem & moose_system, ElementData & element_data) :
  _moose_system(moose_system),
  _element_data(element_data),
  _var_dof_indices(_element_data._dof_data._var_dof_indices),
  _qrule(element_data._qrule),
  _n_qpoints(element_data._n_qpoints),
  _var_nums(element_data._var_nums),
  _phi(element_data._phi)
{}

DamperData::~DamperData()
{}

void
DamperData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  _var_increments.resize(n_vars);
}

void
DamperData::reinit(const NumericVector<Number>& increment_vec)
{
  std::set<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::set<unsigned int>::iterator var_num_end = _var_nums.end();

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _moose_system._dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    _var_increments[var_num].resize(_n_qpoints);

    const std::vector<std::vector<Real> > & static_phi = *_phi[fe_type];

    std::vector<unsigned int> & dof_indices = _var_dof_indices[var_num];

    MooseArray<Real> & increment = _var_increments[var_num];

    // Compute the increment at each quadrature point
    for(unsigned int qp=0; qp<_n_qpoints; qp++)
      computeQpSolution(increment[qp], increment_vec, dof_indices, qp, static_phi);
  }
}
