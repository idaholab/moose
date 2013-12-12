//  Base class for effective saturation as a function of capillary pressure
//
#include "RichardsSeff.h"

template<>
InputParameters validParams<RichardsSeff>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Fluid seff base class.  Override seff, dseff and d2seff in your class");
  return params;
}

RichardsSeff::RichardsSeff(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters)
{}

void
RichardsSeff::initialize()
{}

void
RichardsSeff::execute()
{}

void RichardsSeff::finalize()
{}


Real
RichardsSeff::seff(std::vector<Real> p) const
{
  return 0;
}

Real
RichardsSeff::dseff(std::vector<Real> p) const
{
  return 0;
}

Real
RichardsSeff::d2seff(std::vector<Real> p) const
{
  return 0;
}

