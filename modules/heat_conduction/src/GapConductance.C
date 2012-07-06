#include "GapConductance.h"

template<>
InputParameters validParams<GapConductance>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("gap_distance", "Distance across the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  return params;
}

GapConductance::GapConductance(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _gap_distance(coupledValue("gap_distance")),
   _gap_conductance(declareProperty<Real>("gap_conductance")),
   _gap_conductance_dT(declareProperty<Real>("gap_conductance_dT")),
   _gap_conductivity(getParam<Real>("gap_conductivity")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap"))
{
}


void
GapConductance::computeQpProperties()
{
  _gap_conductance[_qp] = h_conduction();
  _gap_conductance_dT[_qp] = dh_conduction();
}


Real
GapConductance::h_conduction()
{
  return gapK()/gapLength(-(_gap_distance[_qp]), _min_gap, _max_gap);
}


Real
GapConductance::dh_conduction()
{
  return 0;
}

Real
GapConductance::gapLength(Real distance, Real min_gap, Real max_gap)
{
  Real gap_L = distance;

  if(gap_L > max_gap)
  {
    gap_L = max_gap;
  }

  gap_L = std::max(min_gap, gap_L);

  return gap_L;
}


Real
GapConductance::gapK()
{
  return _gap_conductivity;
}

