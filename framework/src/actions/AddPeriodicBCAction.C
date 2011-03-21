#include "AddPeriodicBCAction.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"
#include "FunctionPeriodicBoundary.h"
#include "NonlinearSystem.h"
#include "MProblem.h"

// libMesh includes
#include "periodic_boundaries.h"

template<>
InputParameters validParams<AddPeriodicBCAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<unsigned int>("primary", "Boundary ID associated with the primary boundary.");
  params.addRequiredParam<unsigned int>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addParam<std::vector<Real> >("translation", "Vector that translates coordinates on the primary boundary to coordinates on the secondary boundary.");
  params.addParam<std::vector<std::string> >("transform_func", "Functions that specify the transformation");
  params.addParam<std::vector<std::string> >("inv_transform_func", "Functions that specify the inverse transformation");
  params.addParam<std::string>("variable", "Variable for the periodic boundary");
  return params;
}

AddPeriodicBCAction::AddPeriodicBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddPeriodicBCAction::act() 
{
#ifdef DEBUG
  std::cerr << "Acting on AddPeriodicBCAction\n"
            << "\tname: " << getShortName() << "\n";
#endif
  
  NonlinearSystem & nl = _parser_handle._problem->getNonlinearSystem();

  if (getParam<std::vector<Real> >("translation") != std::vector<Real>())
  {
    std::vector<Real> translation = getParam<std::vector<Real> >("translation");
    translation.resize(3);

    PeriodicBoundary p(RealVectorValue(translation[0], translation[1], translation[2]));
    p.myboundary = getParam<unsigned int>("primary");
    p.pairedboundary = getParam<unsigned int>("secondary");
    if (getParam<std::string>("variable") != std::string())
      p.set_variable(nl.getVariable(0, getParam<std::string>("variable")).number());

    nl.dofMap().add_periodic_boundary(p);
  }
  else if (getParam<std::vector<std::string> >("transform_func") != std::vector<std::string>())
  {
    std::vector<std::string> fn_names = getParam<std::vector<std::string> >("transform_func");

    FunctionPeriodicBoundary *pb = new FunctionPeriodicBoundary(*_parser_handle._problem, fn_names);
    pb->myboundary = getParam<unsigned int>("primary");
    pb->pairedboundary = getParam<unsigned int>("secondary");
    if (getParam<std::string>("variable") != std::string())
      pb->set_variable(nl.getVariable(0, getParam<std::string>("variable")).number());

    FunctionPeriodicBoundary *ipb = NULL;
    if (getParam<std::vector<std::string> >("inv_transform_func") != std::vector<std::string>())
    {
      // asymmetric translation vector
      std::vector<std::string> inv_fn_names = getParam<std::vector<std::string> >("inv_transform_func");

      ipb = new FunctionPeriodicBoundary(*_parser_handle._problem, inv_fn_names);
      // these are switched, because we are forming the inverse translation
      ipb->myboundary = getParam<unsigned int>("secondary");
      ipb->pairedboundary = getParam<unsigned int>("primary");
      if (getParam<std::string>("variable") != std::string())
        ipb->set_variable(nl.getVariable(0, getParam<std::string>("variable")).number());
    }
    else
    {
      // symmetric translation vector
      ipb = new FunctionPeriodicBoundary(*pb, true);
    }

    nl.dofMap().add_periodic_boundary(pb, ipb);
  }
  else
  {
    mooseError("You have to specify either 'translation' or 'trans_func' in your period boundary section '" + _name + "'");
  }
}  
