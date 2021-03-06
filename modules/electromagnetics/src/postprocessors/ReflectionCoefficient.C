#include "ReflectionCoefficient.h"
#include "Function.h"
#include <complex>
#include "libmesh/quadrature.h"

registerMooseObject("ElkApp", ReflectionCoefficient);

InputParameters
ReflectionCoefficient::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addClassDescription("CURRENTLY ONLY FOR 1D SOLVES. Calculate power reflection coefficient "
                             "for impinging wave on a "
                             "surface. Assumes that wave of form F = F_incoming + R*F_reflected");
  params.addRequiredCoupledVar(
      "variable", "The name of the real field variable this postprocessor operates on.");
  params.addRequiredCoupledVar("field_imag", "Coupled imaginary field variable.");
  params.addRequiredParam<Real>("theta", "Wave incidence angle.");
  params.addRequiredParam<Real>("length", "Domain length");
  params.addRequiredParam<Real>("k", "Wave number");
  params.addParam<Real>("coeff", 1.0, "Coefficient");
  return params;
}

ReflectionCoefficient::ReflectionCoefficient(const InputParameters & parameters)
  : SidePostprocessor(parameters),

    MooseVariableInterface<Real>(this, false),
    _u(coupledValue("variable")),

    _qp(0),

    _coupled_imag(coupledValue("field_imag")),
    _theta(getParam<Real>("theta")),
    _length(getParam<Real>("length")),
    _k(getParam<Real>("k")),
    _coeff(getParam<Real>("coeff"))
{
  addMooseVariableDependency(mooseVariable());
}

void
ReflectionCoefficient::initialize()
{
  _reflection_coefficient = 0;
}

void
ReflectionCoefficient::execute()
{
  _reflection_coefficient = computeReflection();
}

PostprocessorValue
ReflectionCoefficient::getValue()
{
  return _reflection_coefficient;
}

void
ReflectionCoefficient::threadJoin(const UserObject & y)
{
  const ReflectionCoefficient & pps = static_cast<const ReflectionCoefficient &>(y);
  _reflection_coefficient = pps._reflection_coefficient;
}

Real
ReflectionCoefficient::computeReflection()
{
  Real ref = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {

    std::complex<double> jay(0, 1);
    std::complex<double> field(_u[_qp], _coupled_imag[_qp]);

    std::complex<double> reflection_coefficient_complex =
        (field - _coeff * std::exp(jay * _k * _length * std::cos(_theta * libMesh::pi / 180.))) /
        (_coeff * std::exp(-jay * _k * _length * std::cos(_theta * libMesh::pi / 180.)));

    ref = std::pow(std::abs(reflection_coefficient_complex), 2);
  }
  return ref;
}
