/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GenericPeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

#include "periodic_boundaries.h"

template<>
InputParameters validParams<GenericPeriodicBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<unsigned int>("primary", "Boundary ID associated with the primary boundary.");
  params.addRequiredParam<unsigned int>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addRequiredParam<std::vector<Real> >("translation", "Vector that translates coordinates on the primary boundary to coordinates on the secondary boundary.");  
  return params;
}

GenericPeriodicBlock::GenericPeriodicBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
GenericPeriodicBlock::execute() 
{
  TransientNonlinearImplicitSystem &system = *_moose_system.getNonlinearSystem();

  PeriodicBoundary p;
  p.myboundary = getParamValue<unsigned int>("primary");
  p.pairedboundary = getParamValue<unsigned int>("secondary");

  std::vector<Real> translation = getParamValue<std::vector<Real> >("translation");

  translation.resize(3);
  
  p.translation_vector = RealVectorValue(translation[0],translation[1],translation[2]);

  system.get_dof_map().add_periodic_boundary(p);
  
  visitChildren();
}  
