#include "AdaptivityBlock.h"
#include "Moose.h"

// libMesh includes
#include "transient_system.h"

template<>
InputParameters validParams<AdaptivityBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<unsigned int>("steps",                       0, "The number of adaptivity steps to perform at any one time for steady state");
  params.addParam<unsigned int>("initial_adaptivity",          0, "The number of adaptivity steps to perform using the initial conditions");
  params.addParam<Real> ("refine_fraction",                    0.0, "The fraction of elements or error to refine. Should be between 0 and 1.");
  params.addParam<Real> ("coarsen_fraction",                   0.0, "The fraction of elements or error to coarsen. Should be between 0 and 1.");
  params.addParam<unsigned int> ("max_h_level",                0, "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<std::string> ("error_estimator",             "KellyErrorEstimator", "The class name of the error estimator you want to use.");
  params.addParam<bool> ("print_changed_info",                 false, "Determines whether information about the mesh is printed when adapativity occors");

  params.addParam<std::vector<std::string> > ("weight_names", "List of names of variables that will be associated with weight_values");
  params.addParam<std::vector<Real> > ("weight_values", "List of values between 0 and 1 to weight the associated weight_names error by");
  
  return params;
}

AdaptivityBlock::AdaptivityBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
AdaptivityBlock::execute() 
{
  TransientNonlinearImplicitSystem & system = *_moose_system.getNonlinearSystem();
  
  _moose_system.initAdaptivity(getParamValue<unsigned int>("steps"),
      getParamValue<unsigned int>("initial_adaptivity"));

  _moose_system.setErrorEstimator(getParamValue<std::string>("error_estimator"));
  
  _moose_system.setAdaptivityParam("refine fraction", getParamValue<Real>("refine_fraction"));
  _moose_system.setAdaptivityParam("coarsen fraction", getParamValue<Real>("coarsen_fraction"));
  _moose_system.setAdaptivityParam("max h-level", getParamValue<unsigned int>("max_h_level"));

  _moose_system.setPrintMeshChanged(getParamValue<bool>("print_changed_info"));

  const std::vector<std::string> & weight_names = getParamValue<std::vector<std::string> >("weight_names");
  const std::vector<Real> & weight_values = getParamValue<std::vector<Real> >("weight_values");

  int num_weight_names  = weight_names.size();
  int num_weight_values = weight_values.size();

  if(num_weight_names)
  {
    if(num_weight_names != num_weight_values)
      mooseError("Number of weight_names must be equal to number of weight_values in Execution/Adaptivity");

    // If weights have been specified then set the default weight to zero
    std::vector<Real> weights(system.n_vars(),0);

    for(int i=0;i<num_weight_names;i++)
    {
      std::string name = weight_names[i];
      double value = weight_values[i];

      weights[_moose_system.getVariableNumber(name)] = value;
    }

    std::vector<FEMNormType> norms(system.n_vars(), H1_SEMINORM);

    SystemNorm sys_norm(norms, weights);

    _moose_system.setErrorNorm(sys_norm);
  }
  visitChildren();
}

