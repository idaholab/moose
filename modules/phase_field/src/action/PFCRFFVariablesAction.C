#include "PFCRFFVariablesAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"
#include "AddVariableAction.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"

const Real PFCRFFVariablesAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<PFCRFFVariablesAction>()
{
  InputParameters params = validParams<Action>();
  MooseEnum familyEnum = AddVariableAction::getNonlinearVariableFamilies();
  params.addParam<MooseEnum>("family", familyEnum, "Specifies the family of FE shape functions to use for the L variables");
  MooseEnum orderEnum = AddVariableAction::getNonlinearVariableOrders();
  params.addParam<MooseEnum>("order", orderEnum,  "Specifies the order of the FE shape function to use for the L variables");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to the L variables");
  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");

  return params;
}

PFCRFFVariablesAction::PFCRFFVariablesAction(const InputParameters & params) :
    Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base"))
{
}

void
PFCRFFVariablesAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PFCRFFVariablesAction Object\n";
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

    _problem->addVariable(real_name,
                          FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
                          getParam<Real>("scaling"));

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name;
      imag_name.append("_imag");

#ifdef DEBUG
      Moose::err << "Imaginary name = " << imag_name << std::endl;
#endif

      _problem->addVariable(imag_name,
                            FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                   Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
                            getParam<Real>("scaling"));
    }
  }
}

