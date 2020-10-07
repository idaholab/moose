#include "ElectricalContactTestFunc.h"

registerMooseObject("ElkApp", ElectricalContactTestFunc);

InputParameters
ElectricalContactTestFunc::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("");
  params.addParam<Real>("mechanical_pressure",
                        3000.,
                        "Mechanical pressure uniformly applied at the contact surface area "
                        "(Pressure = Force / Surface Area).");
  params.addParam<Real>(
      "mean_hardness", 2.4797e9, "Geometric mean of the hardness of graphite and stainless steel.");
  params.addParam<Real>(
      "graphite_conductivity", 73069.2, "Conductivity in graphite (default at 300 K).");
  params.addParam<Real>("stainless_steel_conductivity",
                        1.41867e6,
                        "Conductivity in stainless steel (default at 300 K).");
  params.addParam<Real>("contact_conductance",
                        75524.,
                        "Electrical contact conductance at the interface (default is at 300 K with "
                        "3 kN/m^2 applied pressure).");
  MooseEnum domain("stainless_steel graphite");
  params.addParam<MooseEnum>("domain", domain, "Material domain / block of interest.");
  return params;
}

ElectricalContactTestFunc::ElectricalContactTestFunc(const InputParameters & parameters)
  : Function(parameters),
    _electrical_conductivity_graphite(getParam<Real>("graphite_conductivity")),
    _electrical_conductivity_stainless_steel(getParam<Real>("stainless_steel_conductivity")),
    _mean_hardness(getParam<Real>("mean_hardness")),
    _mechanical_pressure(getParam<Real>("mechanical_pressure")),
    _electrical_contact_conductance(getParam<Real>("contact_conductance")),
    _domain(getParam<MooseEnum>("domain"))
{
}

Real
ElectricalContactTestFunc::value(Real /*t*/, const Point & p) const
{
  Real denominator = _electrical_contact_conductance * _electrical_conductivity_stainless_steel +
                     _electrical_conductivity_graphite * _electrical_conductivity_stainless_steel +
                     _electrical_conductivity_graphite * _electrical_contact_conductance;

  Real graphite_coefficient =
      -_electrical_contact_conductance * _electrical_conductivity_stainless_steel / denominator;

  Real stainless_steel_coefficient =
      -_electrical_contact_conductance * _electrical_conductivity_graphite / denominator;

  Real graphite_func = graphite_coefficient * (p(0) - 2);

  Real stainless_steel_func = stainless_steel_coefficient * p(0) + 1;

  /// Enum-to-enum comparisons are a bit more lightweight, so create another
  /// enum with the possible choices.
  enum DomainEnum
  {
    STAINLESS_STEEL,
    GRAPHITE
  };

  if (_domain == STAINLESS_STEEL)
  {
    return stainless_steel_func;
  }
  else if (_domain == GRAPHITE)
  {
    return graphite_func;
  }
  else
  {
    mooseError(_name + ": Error in selecting proper domain in ElectricalContactTestFunc.");
  }
}
