#include "FlowBoundaryFlux1Phase.h"
#include "BoundaryFlux3EqnGhostBase.h"

registerMooseObject("THMApp", FlowBoundaryFlux1Phase);

InputParameters
FlowBoundaryFlux1Phase::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  MooseEnum equation("mass=0 momentum=1 energy=2");
  params.addRequiredParam<MooseEnum>(
      "equation", equation, "Equation for which to query flux vector");
  params.addCoupledVar("variables", "Single-phase flow variables");
  params.set<std::vector<VariableName>>("variables") = {"rhoA", "rhouA", "rhoEA"};
  params.addClassDescription(
      "Retrieves an entry of a flux vector for a connection attached to a 1-phase junction");

  return params;
}

FlowBoundaryFlux1Phase::FlowBoundaryFlux1Phase(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _n_components(3),
    _boundary_name(getParam<std::vector<BoundaryName>>("boundary")[0]),
    _boundary_uo_name(_boundary_name + ":boundary_uo"),
    _boundary_uo(getUserObjectByName<BoundaryFlux3EqnGhostBase>(_boundary_uo_name)),
    _equation_index(getParam<MooseEnum>("equation"))
{
  for (unsigned int i = 0; i < _n_components; i++)
    _U.push_back(&coupledValue("variables", i));
}

Real
FlowBoundaryFlux1Phase::computeQpIntegral()
{
  std::vector<Real> U(_n_components);
  for (unsigned int i = 0; i < _n_components; i++)
    U[i] = (*_U[i])[_qp];

  const auto & flux = _boundary_uo.getFlux(_current_side, _current_elem->id(), U, _normals[_qp]);
  return flux[_equation_index];
}
