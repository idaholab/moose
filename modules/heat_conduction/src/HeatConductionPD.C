/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "HeatConductionPD.h"
#include "MooseMesh.h"
#include "Material.h"
#include "Assembly.h"

template<>
InputParameters validParams<HeatConductionPD>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to materials properties to make them unique");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HeatConductionPD::HeatConductionPD(const InputParameters & parameters) 
  :Kernel(parameters),
  _bond_response(getMaterialProperty<Real>("bond_response" + getParam<std::string>("appended_property_name"))),
  _bond_response_dif(getMaterialProperty<Real>("bond_response_dif" + getParam<std::string>("appended_property_name")))
{
}

HeatConductionPD::~HeatConductionPD() 
{
}

void
HeatConductionPD::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  int sign(-_test[0][0]/std::abs(_test[0][0]));
  _local_re(0) = sign * _bond_response[0];
  _local_re(1) = -_local_re(0);

  re += _local_re;   

  if (_has_save_in)
  { 
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
HeatConductionPD::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (_i = 0; _i < _test.size(); _i++)
  {
    for (_j = 0; _j < _phi.size(); _j++)
    {
      int sign( _i == _j ? 1 : -1 );
      _local_ke(_i, _j) += sign * _bond_response_dif[0];
    }  
  }

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++) diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i < _diag_save_in.size(); i++)
    _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}
