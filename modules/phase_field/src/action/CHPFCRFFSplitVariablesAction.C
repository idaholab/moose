/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHPFCRFFSplitVariablesAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<CHPFCRFFSplitVariablesAction>()
{
  InputParameters params = validParams<Action>();
  MooseEnum familyEnum = AddVariableAction::getNonlinearVariableFamilies();
  params.addParam<MooseEnum>(
      "family",
      familyEnum,
      "Specifies the family of FE shape functions to use for the L variables");
  MooseEnum orderEnum = AddVariableAction::getNonlinearVariableOrders();
  params.addParam<MooseEnum>(
      "order",
      orderEnum,
      "Specifies the order of the FE shape function to use for the L variables");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to the L variables");
  params.addRequiredParam<unsigned int>(
      "num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addRequiredParam<std::vector<FileName>>("sub_filenames",
                                                 "This is the filename of the sub.i file");
  params.addRequiredParam<AuxVariableName>("n_name", "Name of atomic density variable");

  return params;
}

CHPFCRFFSplitVariablesAction::CHPFCRFFSplitVariablesAction(const InputParameters & params)
  : Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _sub_filenames(getParam<std::vector<FileName>>("sub_filenames")),
    _n_name(getParam<AuxVariableName>("n_name"))
{
}

void
CHPFCRFFSplitVariablesAction::act()
{
  MultiMooseEnum execute_options = MooseUtils::createExecuteOnEnum({EXEC_TIMESTEP_BEGIN});

  // Setup MultiApp
  InputParameters poly_params = _factory.getValidParams("TransientMultiApp");
  poly_params.set<MooseEnum>("app_type") = "PhaseFieldApp";
  poly_params.set<MultiMooseEnum>("execute_on") = execute_options;
  poly_params.set<std::vector<FileName>>("input_files") = _sub_filenames;
  poly_params.set<unsigned int>("max_procs_per_app") = 1;
  poly_params.set<std::vector<Point>>("positions") = {Point()};
  _problem->addMultiApp("TransientMultiApp", "HHEquationSolver", poly_params);

  poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
  poly_params.set<MooseEnum>("direction") = "to_multiapp";
  poly_params.set<MultiMooseEnum>("execute_on") = execute_options;
  poly_params.set<AuxVariableName>("variable") = _n_name;
  poly_params.set<VariableName>("source_variable") = _n_name;
  poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";
  _problem->addTransfer("MultiAppNearestNodeTransfer", _n_name + "_trans", poly_params);

  // Loop through the number of L variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base + Moose::stringify(l);

    // Create real L variable
    std::string real_name = L_name + "_real";

    _problem->addAuxVariable(
        real_name,
        FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
               Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))));

    poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
    poly_params.set<MooseEnum>("direction") = "from_multiapp";
    poly_params.set<AuxVariableName>("variable") = real_name;
    poly_params.set<VariableName>("source_variable") = real_name;
    poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";
    _problem->addTransfer("MultiAppNearestNodeTransfer", real_name + "_trans", poly_params);

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name + "_imag";

      _problem->addAuxVariable(
          imag_name,
          FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))));

      poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
      poly_params.set<MooseEnum>("direction") = "from_multiapp";
      poly_params.set<AuxVariableName>("variable") = imag_name;
      poly_params.set<VariableName>("source_variable") = imag_name;
      poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";
      _problem->addTransfer("MultiAppNearestNodeTransfer", imag_name + "_trans", poly_params);
    }
  }
}
