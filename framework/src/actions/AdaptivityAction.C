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

#include "AdaptivityAction.h"

#ifdef LIBMESH_ENABLE_AMR

#include "Moose.h"
#include "Parser.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "Adaptivity.h"
#include "Executioner.h"

// libMesh includes
#include "transient_system.h"

template<>
InputParameters validParams<AdaptivityAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<unsigned int>("steps",                       0, "The number of adaptivity steps to perform at any one time for steady state");
  params.addParam<unsigned int>("initial_adaptivity",          0, "The number of adaptivity steps to perform using the initial conditions");
  params.addParam<Real> ("refine_fraction",                    0.0, "The fraction of elements or error to refine. Should be between 0 and 1.");
  params.addParam<Real> ("coarsen_fraction",                   0.0, "The fraction of elements or error to coarsen. Should be between 0 and 1.");
  params.addParam<unsigned int> ("max_h_level",                0, "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<std::string> ("error_estimator",             "KellyErrorEstimator", "The class name of the error estimator you want to use.");
  params.addParam<bool> ("print_changed_info",                 false, "Determines whether information about the mesh is printed when adaptivity occurs");
  params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that adaptivity will be active after.");
  params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which adaptivity will no longer be active.");
  params.addParam<std::vector<std::string> > ("weight_names", "List of names of variables that will be associated with weight_values");
  params.addParam<std::vector<Real> > ("weight_values", "List of values between 0 and 1 to weight the associated weight_names error by");
  params.addParam<unsigned int>("cycles_per_step", 1, "The number of adaptivity cycles per step");

  return params;
}

AdaptivityAction::AdaptivityAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AdaptivityAction::act()
{
  NonlinearSystem & system = _problem->getNonlinearSystem();

  Adaptivity & adapt = _problem->adaptivity();

  adapt.init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_adaptivity"));

  adapt.setErrorEstimator(getParam<std::string>("error_estimator"));

  adapt.setParam("cycles_per_step", getParam<unsigned int>("cycles_per_step"));
  adapt.setParam("refine fraction", getParam<Real>("refine_fraction"));
  adapt.setParam("coarsen fraction", getParam<Real>("coarsen_fraction"));
  adapt.setParam("max h-level", getParam<unsigned int>("max_h_level"));

  adapt.setPrintMeshChanged(getParam<bool>("print_changed_info"));

  const std::vector<std::string> & weight_names = getParam<std::vector<std::string> >("weight_names");
  const std::vector<Real> & weight_values = getParam<std::vector<Real> >("weight_values");

  int num_weight_names  = weight_names.size();
  int num_weight_values = weight_values.size();

  if(num_weight_names)
  {
    if(num_weight_names != num_weight_values)
      mooseError("Number of weight_names must be equal to number of weight_values in Execution/Adaptivity");

    // If weights have been specified then set the default weight to zero
    std::vector<Real> weights(system.nVariables(),0);

    for(int i=0;i<num_weight_names;i++)
    {
      std::string name = weight_names[i];
      double value = weight_values[i];

      weights[system.getVariable(0, name).number()] = value;
    }

    std::vector<FEMNormType> norms(system.nVariables(), H1_SEMINORM);

    SystemNorm sys_norm(norms, weights);

    adapt.setErrorNorm(sys_norm);
  }

  adapt.setTimeActive(getParam<Real>("start_time"), getParam<Real>("stop_time"));
}

#endif //LIBMESH_ENABLE_AMR
