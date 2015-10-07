#include "CHPFCRFFSplitVariablesAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"
#include "AddVariableAction.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/string_to_enum.h"

const Real CHPFCRFFSplitVariablesAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<CHPFCRFFSplitVariablesAction>()
{
  InputParameters params = validParams<Action>();
  MooseEnum familyEnum = AddVariableAction::getNonlinearVariableFamilies();
  params.addParam<MooseEnum>("family", familyEnum, "Specifies the family of FE shape functions to use for the L variables");
  MooseEnum orderEnum = AddVariableAction::getNonlinearVariableOrders();
  params.addParam<MooseEnum>("order", orderEnum,  "Specifies the order of the FE shape function to use for the L variables");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to the L variables");
  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addRequiredParam<std::vector<FileName> >("sub_filenames", "This is the filename of the sub.i file");
  params.addRequiredParam<AuxVariableName>("n_name", "Name of atomic density variable");

  return params;
}

CHPFCRFFSplitVariablesAction::CHPFCRFFSplitVariablesAction(const InputParameters & params) :
    Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _sub_filenames(getParam<std::vector<FileName> >("sub_filenames")),
    _n_name(getParam<AuxVariableName>("n_name"))
{
}

void
CHPFCRFFSplitVariablesAction::act()
{
  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";

  // Setup MultiApp
  InputParameters poly_params = _factory.getValidParams("TransientMultiApp");
  poly_params.set<MooseEnum>("app_type") = "PhaseFieldApp";
  poly_params.set<MultiMooseEnum>("execute_on") = execute_options;
  poly_params.set<std::vector<FileName> >("input_files") = _sub_filenames;
  poly_params.set<unsigned int>("max_procs_per_app") = 1;

  Point one(0.0, 0.0, 0.0);

  std::vector<Point > positions;
  positions.push_back(one);

  poly_params.set<std::vector<Point> >("positions") = positions;

  _problem->addMultiApp("TransientMultiApp", "HHEquationSolver", poly_params);

  poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
  poly_params.set<MooseEnum>("direction") = "to_multiapp";
  poly_params.set<MultiMooseEnum>("execute_on") = execute_options;
  poly_params.set<AuxVariableName>("variable") = _n_name;
  poly_params.set<VariableName>("source_variable") = _n_name;
  poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";


  // Create Name for Transfer
  std::string trans_name = _n_name;
  trans_name.append("_trans");

  _problem->addTransfer("MultiAppNearestNodeTransfer", trans_name, poly_params);

#ifdef DEBUG
  Moose::err << "Inside the CHPFCRFFSplitVariablesAction Object\n";
  Moose::err << "VariableBase: " << _L_name_base
            << "\torder: " << getParam<MooseEnum>("order")
            << "\tfamily: " << getParam<MooseEnum>("family") << std::endl;
#endif

  // Loop through the number of L variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());

    // Create real L variable
    std::string real_name = L_name;
    real_name.append("_real");


#ifdef DEBUG
    Moose::err << "Real name = " << real_name << std::endl;
#endif

    _problem->addAuxVariable(real_name,
                             FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                    Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))));

    poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
    poly_params.set<MooseEnum>("direction") = "from_multiapp";
    poly_params.set<AuxVariableName>("variable") = real_name;
    poly_params.set<VariableName>("source_variable") = real_name;
    poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";


    // Create Name for Transfer
    std::string trans_name = real_name;
    trans_name.append("_trans");

    _problem->addTransfer("MultiAppNearestNodeTransfer", trans_name, poly_params);

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name;
      imag_name.append("_imag");

#ifdef DEBUG
      Moose::err << "Imaginary name = " << imag_name << std::endl;
#endif

      _problem->addAuxVariable(imag_name,
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                      Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))));

      poly_params = _factory.getValidParams("MultiAppNearestNodeTransfer");
      poly_params.set<MooseEnum>("direction") = "from_multiapp";
      poly_params.set<AuxVariableName>("variable") = imag_name;
      poly_params.set<VariableName>("source_variable") = imag_name;
      poly_params.set<MultiAppName>("multi_app") = "HHEquationSolver";

      // Create Name for Transfer
      std::string trans_name = imag_name;
      trans_name.append("_trans");

      _problem->addTransfer("MultiAppNearestNodeTransfer", trans_name, poly_params);
    }

  }

}
