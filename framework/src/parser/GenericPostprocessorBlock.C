#include "GenericPostprocessorBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericPostprocessorBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericPostprocessorBlock::GenericPostprocessorBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));

  if (_parser_handle.pathContains(_name, "Residual"))
    _pps_type = Moose::PPS_RESIDUAL;
  else if (_parser_handle.pathContains(_name, "Jacobian"))
    _pps_type = Moose::PPS_JACOBIAN;
  else if (_parser_handle.pathContains(_name, "NewtonIter"))
    _pps_type = Moose::PPS_NEWTONIT;
  else
    _pps_type = Moose::PPS_TIMESTEP;
}

void
GenericPostprocessorBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericPostprocessorBlock Object\n";
  std::cerr << "Postprocessor:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
  _parser_handle._problem->addPostprocessor(_type, getShortName(), getClassParams(), _pps_type);
}
