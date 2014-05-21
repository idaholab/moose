#include "StochasticFieldAux.h"

template<>
InputParameters validParams<StochasticFieldAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("file_name", "The name of the file that contains one stochastic parameter values");
  return params;
}


StochasticFieldAux::StochasticFieldAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _stoch_field(getParam<std::string>("file_name"))
{}

Real
StochasticFieldAux::computeValue()
{
  return _stoch_field.value(_current_elem->centroid());
}
