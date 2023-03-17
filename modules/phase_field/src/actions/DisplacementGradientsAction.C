//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacementGradientsAction.h"
#include "Factory.h"
#include "FEProblem.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", DisplacementGradientsAction, "add_kernel");

registerMooseAction("PhaseFieldApp", DisplacementGradientsAction, "add_material");

registerMooseAction("PhaseFieldApp", DisplacementGradientsAction, "add_variable");

InputParameters
DisplacementGradientsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up variables, kernels, and materials for a the displacement "
                             "gradients and their elastic free energy derivatives for non-split "
                             "Cahn-Hilliard problems.");
  params.addRequiredParam<std::vector<VariableName>>("displacements",
                                                     "Vector of displacement variables");
  params.addRequiredParam<std::vector<VariableName>>("displacement_gradients",
                                                     "Vector of displacement gradient variables");
  params.addParam<Real>(
      "scaling", 1.0, "Specifies a scaling factor to apply to the displacement gradient variables");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the variables and kernels");
  return params;
}

DisplacementGradientsAction::DisplacementGradientsAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _displacement_gradients(getParam<std::vector<VariableName>>("displacement_gradients"))
{
}

void
DisplacementGradientsAction::act()
{
  unsigned int ngrad = _displacement_gradients.size();

  if (_current_task == "add_variable")
  {
    // Loop through the gij variables
    Real scaling = getParam<Real>("scaling");
    for (unsigned int i = 0; i < ngrad; ++i)
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("family") = "LAGRANGE";
      var_params.set<MooseEnum>("order") = "FIRST";
      var_params.set<std::vector<Real>>("scaling") = {scaling};
      var_params.applySpecificParameters(_pars, {"block"});

      // Create displacement gradient variables
      _problem->addVariable("MooseVariable", _displacement_gradients[i], var_params);
    }
  }
  else if (_current_task == "add_material")
  {
    InputParameters params = _factory.getValidParams("StrainGradDispDerivatives");
    params.set<std::vector<VariableName>>("displacement_gradients") = _displacement_gradients;
    params.applySpecificParameters(parameters(), {"block"});
    _problem->addMaterial("StrainGradDispDerivatives", "strain_grad_disp_derivatives", params);
  }
  else if (_current_task == "add_kernel")
  {
    unsigned int ndisp = _displacements.size();
    if (ndisp * ndisp != ngrad)
      paramError("displacement_gradients",
                 "Number of displacement gradient variables must be the square of the number of "
                 "displacement variables.");

    // Loop through the displacements
    unsigned int i = 0;
    for (unsigned int j = 0; j < ndisp; ++j)
      for (unsigned int k = 0; k < ndisp; ++k)
      {
        InputParameters params = _factory.getValidParams("GradientComponent");
        params.set<NonlinearVariableName>("variable") = _displacement_gradients[i];
        params.set<std::vector<VariableName>>("v") = {_displacements[j]};
        params.set<unsigned int>("component") = k;
        _problem->addKernel(
            "GradientComponent", _displacement_gradients[i] + "_grad_kernel", params);
        ++i;
      }
  }
  else
    mooseError("Internal error.");
}
