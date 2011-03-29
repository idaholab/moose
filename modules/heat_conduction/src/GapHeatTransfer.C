#include "GapHeatTransfer.h"
#include "SystemBase.h"

Threads::spin_mutex slave_flux_mutex;

template<>
InputParameters validParams<GapHeatTransfer>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("gap_distance", "Distance across the gap");
  params.addRequiredCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<Real>("roughness_1", 1e-6, "The roughness value for this surface");
  params.addParam<Real>("roughness_2", 1e-6, "The roughness value for the opposite surface");
  params.addParam<Real>("max_gap", -99999, "A maximum gap size");
  return params;
}

GapHeatTransfer::GapHeatTransfer(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _slave_flux(_sys.getVector("slave_flux")),
   _gap_distance(coupledValue("gap_distance")),
   _gap_temp(coupledValue("gap_temp")),
   _gap_conductivity(getParam<Real>("gap_conductivity")),
   _roughness_1(getParam<Real>("roughness_1")),
   _roughness_2(getParam<Real>("roughness_2")),
   _max_gap(getParam<Real>("max_gap"))
{
}

Real
GapHeatTransfer::computeQpResidual()
{
  Real h_gap = 0.0;
  Real grad_t = 0.0;
  
  h_gap = h_conduction() + h_contact() + h_radiation();
  grad_t = (_u[_qp] - _gap_temp[_qp]) * h_gap;

  // This is keeping track of this residual contribution so it can be used as the flux on the other side of the gap.
  {
    Threads::spin_mutex::scoped_lock lock(slave_flux_mutex);
    _slave_flux.add(_var.dofIndices()[_i], _JxW[_qp]*_phi[_i][_qp]*grad_t);
  }
  
  return _phi[_i][_qp]*grad_t;
}

Real
GapHeatTransfer::computeQpJacobian()
{
  Real h_gap = 0.0;
  Real grad_t = 0.0;
  Real dgrad_t = 0.0;

  h_gap = h_conduction() + h_contact() + h_radiation();
  grad_t = (_u[_qp] - _gap_temp[_qp]) * h_gap;
  dgrad_t = _phi[_j][_qp] * h_gap;

  // Check this: Should we be multiplying by phi twice?
  return _phi[_i][_qp]*dgrad_t;
}


Real
GapHeatTransfer::h_conduction()
{
  Real gap_L = gapLength();
  return gapK()/gap_L;  
}


Real
GapHeatTransfer::h_contact()
{
  return 0.0;  
}


Real
GapHeatTransfer::h_radiation()
{
  return 0.0;  
}


Real
GapHeatTransfer::gapLength()
{
  Real gap_L = _gap_distance[_qp];

  if(gap_L < -99999)
  {
    gap_L = _max_gap;
  }

  gap_L = std::abs(std::min(0.0, gap_L));

  if(gap_L < 1.5*(_roughness_1 + _roughness_2))
    gap_L = 1.5*(_roughness_1 + _roughness_2);

  return gap_L;
}


Real
GapHeatTransfer::gapK()
{
  
  return _gap_conductivity;
}

