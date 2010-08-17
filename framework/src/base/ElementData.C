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
