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
  _qrule = new QGauss(_moose_system.getDim(), _moose_system._max_quadrature_order);

  QuadraturePointData::init();
}


void
ElementData::reinitKernels(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * /*Re*/, DenseMatrix<Number> * /*Ke*/)
{
//  Moose::perf_log.push("reinit()","Kernel");

  QuadraturePointData::reinit(soln, elem);

//  Moose::perf_log.pop("reinit()","Kernel");
}

void
ElementData::reinitNewtonStep(const NumericVector<Number>& soln)
{
  _n_qpoints = _qrule->n_points();

  std::set<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::set<unsigned int>::iterator var_num_end = _var_nums.end();

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

void
ElementData::reinitMaterials(std::vector<Material *> & materials)
{
//  Moose::perf_log.push("reinit() - material", "ElementData");

  _material = materials;
  for (std::vector<Material *>::iterator it = _material.begin(); it != _material.end(); ++it)
    (*it)->materialReinit();

//  Moose::perf_log.pop("reinit() - material","ElementData");
}

