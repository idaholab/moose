#include "GenericPeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

#include "dof_map.h"

template<>
InputParameters validParams<GenericPeriodicBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<unsigned int>("primary", "Boundary ID associated with the primary boundary.");
  params.addRequiredParam<unsigned int>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addRequiredParam<std::vector<Real> >("translation", "Vector that translates coordinates on the primary boundary to coordinates on the secondary boundary.");  
  return params;
}

GenericPeriodicBlock::GenericPeriodicBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{}

void
GenericPeriodicBlock::execute() 
{
  std::cout<<"Executing GenericPeriodic block!"<<std::endl;

  TransientNonlinearImplicitSystem &system =
    Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  PeriodicBoundary p;
  p.myboundary = getParamValue<unsigned int>("primary");
  p.pairedboundary = getParamValue<unsigned int>("secondary");

  std::vector<Real> translation = getParamValue<std::vector<Real> >("translation");

  translation.resize(3);
  
  p.translation_vector = RealVectorValue(translation[0],translation[1],translation[2]);

  system.get_dof_map().add_periodic_boundary(p);
  
  visitChildren();
}  
