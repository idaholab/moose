#include "BulkCoolantBC.h"

template<>
InputParameters validParams<BulkCoolantBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("heat_transfer_coefficient",2E3, "heat transfer coefficient W/m^2-K"); //should be optionally a material property
  params.addParam<Real>("bulk_temperature",800., "Bulk fluid temperature K");
  params.addParam<FunctionName>("function", "function describing bulk temperature");
  return params;
}

BulkCoolantBC::BulkCoolantBC(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _alpha(getParam<Real>("heat_transfer_coefficient")),
   _tempb(getParam<Real>("bulk_temperature")),
   _has_function(getParam<FunctionName>("function") != ""),
   _function( _has_function ? &getFunction("function") : NULL ),
   _conductivity(getMaterialProperty<Real>("thermal_conductivity"))

  {}

Real
BulkCoolantBC::computeQpResidual()
  {

    Real bulk_temp( _tempb );

    if ( _has_function )
    bulk_temp *= _function->value(_t, _q_point[_qp]);

    return -( _test[_i][_qp]*(_alpha)/(_conductivity[_qp])*(bulk_temp - _u[_qp] ) );

  }

Real
BulkCoolantBC::computeQpJacobian()
{
  return -( _test[_i][_qp]*(_alpha)/(_conductivity[_qp])*(-_phi[_j][_qp]) );
}

