#include "Moose.h"
#include "ElementPostprocessor.h"
#include "Variable.h"
#include "Problem.h"

template<>
InputParameters validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addParam<unsigned int>("block", Moose::ANY_BLOCK_ID, "block ID where the postprocessor works");
  return params;
}

ElementPostprocessor::ElementPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    Moose::TransientInterface(parameters),
    _block_id(parameters.get<unsigned int>("block")),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _q_point(_var.qpoints()),
    _current_elem(_problem.elem()),
    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder())
{
}
