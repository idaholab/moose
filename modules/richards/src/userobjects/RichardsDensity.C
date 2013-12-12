//  Base class for fluid density as a function of pressure
//
#include "RichardsDensity.h"

template<>
InputParameters validParams<RichardsDensity>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Fluid density base class.  Override density, ddensity and d2density in your class");
  return params;
}

RichardsDensity::RichardsDensity(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters)
{}

void
RichardsDensity::initialize()
{}

void
RichardsDensity::execute()
{}

void RichardsDensity::finalize()
{}
