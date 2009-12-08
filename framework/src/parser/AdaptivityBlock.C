#include "AdaptivityBlock.h"
#include "Moose.h"

// libMesh includes
#include "mesh_refinement.h"
#include "error_estimator.h"
#include "error_vector.h"
#include "kelly_error_estimator.h"
#include "fourth_error_estimators.h"
#include "transient_system.h"

AdaptivityBlock::AdaptivityBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{}

void
AdaptivityBlock::execute() 
{
  TransientNonlinearImplicitSystem & system = Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  
  unsigned int max_r_steps = getParamValue<unsigned int>("steps");

  Moose::equation_system->parameters.set<unsigned int>("max_r_steps") = max_r_steps;
  Moose::equation_system->parameters.set<bool>("adaptivity") = true;

  Moose::equation_system->parameters.set<unsigned int>("initial_adaptivity") = getParamValue<unsigned int>("initial_adaptivity");

  if(Moose::mesh_refinement)
    mooseError("Mesh refinement object has already been initialized!");

  Moose::mesh_refinement = new MeshRefinement(*Moose::mesh);
  Moose::error = new ErrorVector;

  std::string error_estimator = getParamValue<std::string>("error_estimator");
  
  if(error_estimator == "KellyErrorEstimator")
    Moose::error_estimator = new KellyErrorEstimator;
  else if(error_estimator == "LaplacianErrorEstimator")
    Moose::error_estimator = new LaplacianErrorEstimator;
  else
    mooseError((std::string("Unknown error_estimator selection: ") + error_estimator).c_str());

  Moose::mesh_refinement->refine_fraction()  = getParamValue<Real>("refine_fraction");
  Moose::mesh_refinement->coarsen_fraction() = getParamValue<Real>("coarsen_fraction");
  Moose::mesh_refinement->max_h_level()      = getParamValue<unsigned int>("max_h_level");

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

      weights[system.variable_number(name)] = value;
    }

    std::vector<FEMNormType> norms(system.n_vars(), H1_SEMINORM);

    SystemNorm sys_norm(norms, weights);

    Moose::error_estimator->error_norm = sys_norm;
  }
  visitChildren();
}

