#include "AdaptivityBlock.h"

// libMesh includes
#include "mesh_refinement.h"
#include "error_estimator.h"

AdaptivityBlock::AdaptivityBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  addParam<unsigned int>("steps", 0, "The number of adaptivity steps to perform at any one time", false);
  addParam<Real> ("refine_fraction", 0.0, "The fraction of elements or error to refine. Should be between 0 and 1.", false);
  addParam<Real> ("coarse_fraction", 0.0, "The fraction of elements or error to coarsen. Should be between 0 and 1.", false);
  addParam<unsigned int> ("max_h_level", 0, "Maximum number of times a single element can be refined. If 0 then infinite.", false);
  addParam<std::vector<std::string> > ("weight_names", "List of names of variables that will be associated with weight_values", false);
  addParam<std::vector<Real> > ("weight_values", "List of values between 0 and 1 to weight the associated weight_names error by", false);
}

void
AdaptivityBlock::execute() 
{
  TransientNonlinearImplicitSystem & system = Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  
  unsigned int max_r_steps = getParamValue<unsigned int>("steps");

  Moose::equation_system->parameters.set<unsigned int>("max_r_steps") = max_r_steps;
  Moose::equation_system->parameters.set<bool>("adaptivity") = true;

  if(Moose::mesh_refinement)
    mooseError("Mesh refinement object has already been initialized!");

  Moose::mesh_refinement = new MeshRefinement(*Moose::mesh);

  Moose::mesh_refinement->refine_fraction()  = getParamValue<Real>("refine_fraction");
  Moose::mesh_refinement->coarsen_fraction() = getParamValue<Real>("coarsen_fraction");
  Moose::mesh_refinement->max_h_level()      = getParamValue<Real>("max_h_level");

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

