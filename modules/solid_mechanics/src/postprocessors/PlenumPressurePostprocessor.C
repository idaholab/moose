#include "PlenumPressurePostprocessor.h"

#include "PlenumPressureUserObject.h"

template<>
InputParameters validParams<PlenumPressurePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("plenum_pressure_uo", "The PlenumPressureUserObject that computes the initial moles.");
  params.addRequiredParam<std::string>("quantity", "The quantity to report.");

  params.set<bool>("use_displaced_mesh") = true;

  // Hide from input file dump
  params.addPrivateParam<std::string>("built_by_action", "" );
  return params;
}

PlenumPressurePostprocessor::PlenumPressurePostprocessor(const std::string & name, InputParameters params)
  :GeneralPostprocessor(name, params),
   _ppuo(getUserObject<PlenumPressureUserObject>("plenum_pressure_uo")),
   _quantity(getParam<std::string>("quantity"))
{
}

PostprocessorValue
PlenumPressurePostprocessor::getValue()
{
  return _ppuo.getValue( _quantity );
}
