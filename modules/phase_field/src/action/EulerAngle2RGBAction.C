/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EulerAngle2RGBAction.h"
#include "Factory.h"
#include "FEProblem.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<EulerAngle2RGBAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("auxvariable_name_base", "RGB", "Base name of the auxvariables");
  params.addClassDescription("Set up auxvariables and auxkernels to output Euler angles as RGB "
                             "values interpolated across inverse pole figure");
  MooseEnum sd_enum = MooseEnum("100=1 010=2 001=3", "001");
  params.addParam<MooseEnum>("sd", sd_enum, "Reference sample direction");
  MooseEnum structure_enum = MooseEnum(
      "cubic=43 hexagonal=62 tetragonal=42 trigonal=32 orthorhombic=22 monoclinic=2 triclinic=1");
  params.addRequiredParam<MooseEnum>(
      "crystal_structure", structure_enum, "Crystal structure of the material");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addParam<Point>(
      "no_grain_color",
      Point(0, 0, 0),
      "RGB value of color used to represent area with no grains, defaults to black");
  return params;
}

EulerAngle2RGBAction::EulerAngle2RGBAction(const InputParameters & params)
  : Action(params), _var_name_base(getParam<std::string>("auxvariable_name_base"))
{
}

void
EulerAngle2RGBAction::act()
{
  // Auxvariable suffix names that will automatically become a vector in Paraview
  std::vector<std::string> suffixes = {"_x", "_y", "_z"};

  // Three color types that will be outputted
  std::vector<std::string> colors = {"red", "green", "blue"};

  for (unsigned int i = 0; i < 3; ++i)
  {
    // Create the auxvariable name
    std::string var_name = _var_name_base + suffixes[i];

    if (_current_task == "add_aux_variable")
    {
      // Create scalar auxvariables for the three components of the RGB vector
      _problem->addAuxVariable(var_name,
                               FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                      Utility::string_to_enum<FEFamily>("MONOMIAL")));
    }
    else if (_current_task == "add_aux_kernel")
    {
      // Create auxkernels corresponding to each auxvariable
      InputParameters params = _factory.getValidParams("EulerAngleProvider2RGBAux");
      params.set<AuxVariableName>("variable") = var_name;
      params.set<MooseEnum>("sd") = getParam<MooseEnum>("sd");
      params.set<MooseEnum>("crystal_structure") = getParam<MooseEnum>("crystal_structure");
      params.set<MooseEnum>("output_type") = colors[i];
      params.set<UserObjectName>("euler_angle_provider") =
          getParam<UserObjectName>("euler_angle_provider");
      params.set<UserObjectName>("grain_tracker") = getParam<UserObjectName>("grain_tracker");
      MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_TIMESTEP_END});
      params.set<Point>("no_grain_color") = getParam<Point>("no_grain_color");
      _problem->addAuxKernel("EulerAngleProvider2RGBAux", var_name, params);
    }
    else
      mooseError("Internal error in EulerAngle2RGBAction.");
  }
}
