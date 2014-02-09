/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsPlotQuantity.h"
#include "RichardsSumQuantity.h"

template<>
InputParameters validParams<RichardsPlotQuantity>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("uo", "user object name that has the total mass value");

  return params;
}

RichardsPlotQuantity::RichardsPlotQuantity(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _total_mass(getUserObject<RichardsSumQuantity>("uo"))
{
}

RichardsPlotQuantity::~RichardsPlotQuantity()
{
}

void
RichardsPlotQuantity::initialize()
{
}

void
RichardsPlotQuantity::execute()
{
}

PostprocessorValue
RichardsPlotQuantity::getValue()
{
  return _total_mass.getValue();
}
