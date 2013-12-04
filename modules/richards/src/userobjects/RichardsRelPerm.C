//  Base class for relative permeability as a function of effective saturation
//
#include "RichardsRelPerm.h"

template<>
InputParameters validParams<RichardsRelPerm>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Relative permeability base class.  Override relperm, drelperm and d2relperm in your class");
  return params;
}

RichardsRelPerm::RichardsRelPerm(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters)
{}

void
RichardsRelPerm::initialize()
{}

void
RichardsRelPerm::execute()
{}

void RichardsRelPerm::finalize()
{}


Real
RichardsRelPerm::relperm(Real seff) const
{
  return 0;
}

Real
RichardsRelPerm::drelperm(Real seff) const
{
  return 0;
}

Real
RichardsRelPerm::d2relperm(Real seff) const
{
  return 0;
}

