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

#include "GenericPeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"
#include "FunctionPeriodicBoundary.h"

#include "periodic_boundaries.h"

template<>
InputParameters validParams<GenericPeriodicBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<unsigned int>("primary", "Boundary ID associated with the primary boundary.");
  params.addRequiredParam<unsigned int>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addParam<std::vector<Real> >("translation", "Vector that translates coordinates on the primary boundary to coordinates on the secondary boundary.");
  params.addParam<std::vector<std::string> >("transform_func", "Functions that specify the transformation");
  params.addParam<std::vector<std::string> >("inv_transform_func", "Functions that specify the inverse transformation");
  return params;
}

GenericPeriodicBlock::GenericPeriodicBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
GenericPeriodicBlock::execute() 
{
  TransientNonlinearImplicitSystem &system = *_moose_system.getNonlinearSystem();

  if (getParamValue<std::vector<Real> >("translation") != std::vector<Real>())
  {
    std::vector<Real> translation = getParamValue<std::vector<Real> >("translation");
    translation.resize(3);

    PeriodicBoundary p(RealVectorValue(translation[0], translation[1], translation[2]));
    p.myboundary = getParamValue<unsigned int>("primary");
    p.pairedboundary = getParamValue<unsigned int>("secondary");

    system.get_dof_map().add_periodic_boundary(p);
  }
  else if (getParamValue<std::vector<std::string> >("transform_func") != std::vector<std::string>())
  {
    std::vector<std::string> fn_names = getParamValue<std::vector<std::string> >("transform_func");

    FunctionPeriodicBoundary *pb = new FunctionPeriodicBoundary(_moose_system, fn_names);
    pb->myboundary = getParamValue<unsigned int>("primary");
    pb->pairedboundary = getParamValue<unsigned int>("secondary");

    FunctionPeriodicBoundary *ipb = NULL;
    if (getParamValue<std::vector<std::string> >("inv_transform_func") != std::vector<std::string>())
    {
      // asymmetric translation vector
      std::vector<std::string> inv_fn_names = getParamValue<std::vector<std::string> >("inv_transform_func");

      ipb = new FunctionPeriodicBoundary(_moose_system, inv_fn_names);
      // these are switched, because we are forming the inverse translation
      ipb->myboundary = getParamValue<unsigned int>("secondary");
      ipb->pairedboundary = getParamValue<unsigned int>("primary");
    }
    else
    {
      // symmetric translation vector
      ipb = new FunctionPeriodicBoundary(*pb, true);
    }

    system.get_dof_map().add_periodic_boundary(pb, ipb);
  }
  else
  {
    mooseError("You have to specify either 'translation' or 'trans_func' in your period boundary section '" + _name + "'");
  }
  
  visitChildren();
}  
