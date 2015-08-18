/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatSourcePD.h"

#include "Material.h"
#include "SymmElasticityTensor.h"
using namespace std;

template<>
InputParameters validParams<HeatSourcePD>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("temp", "The temperature");
  params.addParam<Real>("PowerDensity", 1.0, "PowerDensity");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<FunctionName>("function", "1", "Function describing the volumetric heat source");
  return params;
}


HeatSourcePD::HeatSourcePD(const InputParameters & parameters)
  :Kernel(parameters),
   _power_density(isParamValid("PowerDensity") ? getParam<Real>("PowerDensity") : 0),
   _bond_volume(getMaterialProperty<Real>("bond_volume" + getParam<std::string>("appended_property_name"))),
   _bond_status(getMaterialProperty<Real>("bond_status" + getParam<std::string>("appended_property_name"))),
   _function(getFunction("function")),
   _temp_coupled(isCoupled("temp")),
   _temp_var(_temp_coupled ? coupled("temp") : 0)
{
}

Real
HeatSourcePD::computeQpResidual()
{
  Real factor = _function.value(_t, _q_point[_qp]);
  return factor;
}

void
HeatSourcePD::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  mooseAssert(re.size() == 2, "Truss elements must have two nodes");
  _local_re.resize(re.size());
  _local_re.zero();

  //_local_re(0) = _power_density * _bond_volume[0] * _bond_status[0] / 2.0;
  //_local_re(0) = -_power_density * _bond_volume[0] / 2.0;
  _qp = 0;
  _local_re(0) = -computeQpResidual() * _bond_volume[0] / 2.0;
  _local_re(1) = _local_re(0);

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}
