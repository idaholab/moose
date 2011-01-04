/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GenericPostprocessorBlock.h"
#include "PostprocessorFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericPostprocessorBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericPostprocessorBlock::GenericPostprocessorBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params),
   _type(getType())
{
  setClassParams(PostprocessorFactory::instance()->getValidParams(_type));

  if (_parser_handle.pathContains(_name, "Residual"))
    _pps_type = Moose::PPS_RESIDUAL;
  else if (_parser_handle.pathContains(_name, "Jacobian"))
    _pps_type = Moose::PPS_JACOBIAN;
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
  
  _moose_system.addPostprocessor(_type, getShortName(), getClassParams(), _pps_type);
}
