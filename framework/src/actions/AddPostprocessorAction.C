#include "AddPostprocessorAction.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddPostprocessorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddPostprocessorAction::AddPostprocessorAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
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
AddPostprocessorAction::act() 
{
#ifdef DEBUG
  std::cerr << "Acting on AddPostprocessorAction Object\n";
  std::cerr << "Postprocessor:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
  _parser_handle._problem->addPostprocessor(_type, getShortName(), _moose_object_pars, _pps_type);
}
