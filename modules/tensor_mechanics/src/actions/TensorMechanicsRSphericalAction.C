/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsRSphericalAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

template<>
InputParameters validParams<TensorMechanicsRSphericalAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernel for the 1D spherical problem");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variable for the problem (should only be a single variable)");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_r", "Auxiliary variables to save the r displacement residuals.");
  return params;
}

TensorMechanicsRSphericalAction::TensorMechanicsRSphericalAction(const InputParameters & params) :
    Action(params)
{
}

void
TensorMechanicsRSphericalAction::act()
{
  std::vector<NonlinearVariableName> displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");
  std::vector<VariableName> coupled_displacements;
  unsigned int dim = displacements.size();

  // Error checking:  Can only take one displacement variables in RSpherical kernel
  mooseAssert(dim == 1, "Expected a single displacement variable but recieved " << dim);

  for (unsigned int i = 0; i < dim; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<std::vector<AuxVariableName> > save_in;
  save_in.assign(1, getParam<std::vector<AuxVariableName> >("save_in_disp_r"));

  // Set up the information needed to pass to create the new kernel
  InputParameters params = _factory.getValidParams("StressDivergenceRSphericalTensors");
  params.set<std::vector<VariableName> >("displacements") = coupled_displacements;

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

  for (unsigned int i = 0; i < dim; ++i)
  {
    // Create kernel name dependent on the displacement variable
    std::string kernel_name = "TensorMechanicsRSpherical_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];

    // Create the kernel
    _problem->addKernel("StressDivergenceRSphericalTensors", kernel_name, params);
  }
}
