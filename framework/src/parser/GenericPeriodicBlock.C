#include "GenericPeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"
#include "FunctionPeriodicBoundary.h"
#include "NonlinearSystem.h"
#include "MProblem.h"

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
  params.addParam<std::string>("variable", "Variable for the periodic boundary");
  return params;
}

GenericPeriodicBlock::GenericPeriodicBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
}

void
GenericPeriodicBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericPeriodicBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName() << "\n";
#endif
  
  if (_executed)
    return;

  Moose::NonlinearSystem & nl = _parser_handle._problem->getNonlinearSystem();

  if (getParamValue<std::vector<Real> >("translation") != std::vector<Real>())
  {
    std::vector<Real> translation = getParamValue<std::vector<Real> >("translation");
    translation.resize(3);

    PeriodicBoundary p(RealVectorValue(translation[0], translation[1], translation[2]));
    p.myboundary = getParamValue<unsigned int>("primary");
    p.pairedboundary = getParamValue<unsigned int>("secondary");
    if (getParamValue<std::string>("variable") != std::string())
      p.set_variable(nl.getVariable(0, getParamValue<std::string>("variable")).number());

    nl.dofMap().add_periodic_boundary(p);
  }
  else if (getParamValue<std::vector<std::string> >("transform_func") != std::vector<std::string>())
  {
    std::vector<std::string> fn_names = getParamValue<std::vector<std::string> >("transform_func");

    Moose::FunctionPeriodicBoundary *pb = new Moose::FunctionPeriodicBoundary(*_parser_handle._problem, fn_names);
    pb->myboundary = getParamValue<unsigned int>("primary");
    pb->pairedboundary = getParamValue<unsigned int>("secondary");
    if (getParamValue<std::string>("variable") != std::string())
      pb->set_variable(nl.getVariable(0, getParamValue<std::string>("variable")).number());

    Moose::FunctionPeriodicBoundary *ipb = NULL;
    if (getParamValue<std::vector<std::string> >("inv_transform_func") != std::vector<std::string>())
    {
      // asymmetric translation vector
      std::vector<std::string> inv_fn_names = getParamValue<std::vector<std::string> >("inv_transform_func");

      ipb = new Moose::FunctionPeriodicBoundary(*_parser_handle._problem, inv_fn_names);
      // these are switched, because we are forming the inverse translation
      ipb->myboundary = getParamValue<unsigned int>("secondary");
      ipb->pairedboundary = getParamValue<unsigned int>("primary");
      if (getParamValue<std::string>("variable") != std::string())
        ipb->set_variable(nl.getVariable(0, getParamValue<std::string>("variable")).number());
    }
    else
    {
      // symmetric translation vector
      ipb = new Moose::FunctionPeriodicBoundary(*pb, true);
    }

    nl.dofMap().add_periodic_boundary(pb, ipb);
  }
  else
  {
    mooseError("You have to specify either 'translation' or 'trans_func' in your period boundary section '" + _name + "'");
  }
  
  visitChildren();

  _executed = true;
}  
