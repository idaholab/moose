//  "Power" form of relative permeability
//
#include "RichardsRelPermUnity.h"

template<>
InputParameters validParams<RichardsRelPermUnity>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addClassDescription("Relative Permeability = 1.  This is unphysical - only use for testing!");
  return params;
}

RichardsRelPermUnity::RichardsRelPermUnity(const std::string & name, InputParameters parameters) :
  RichardsRelPerm(name, parameters)
{}


Real
RichardsRelPermUnity::relperm(Real seff) const
{
  return 1.0;
}


Real
RichardsRelPermUnity::drelperm(Real seff) const
{
  return 0.0;
}


Real
RichardsRelPermUnity::d2relperm(Real seff) const
{
  return 0.0;
}

