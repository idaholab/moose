#include "PrintNumElems.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintNumElems>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintNumElems::PrintNumElems(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

Real
PrintNumElems::getValue()
{
  return _subproblem.mesh().n_elem();
}
