#include "PFCRFFVariablesAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"

const Real PFCRFFVariablesAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<PFCRFFVariablesAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for the L variables");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for the L variables");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to the L variables");
  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("L_name_base","Base name for the complex L variables");

  return params;
}

PFCRFFVariablesAction::PFCRFFVariablesAction(const std::string & name, InputParameters params)
  :Action(name, params),
   _num_L(getParam<unsigned int>("num_L")),
   _L_name_base(getParam<std::string>("L_name_base"))
{}

void
PFCRFFVariablesAction::act() 
{ 
#ifdef DEBUG
  std::cerr << "Inside the PFCRFFVariablesAction Object\n";
  std::cerr << "VariableBase: " << _L_name_base
            << "\torder: " << getParam<std::string>("order")
            << "\tfamily: " << getParam<std::string>("family") << std::endl;
#endif
  
  // Loop through the number of L variables
  for (unsigned int l = 0; l<_num_L; l++)
  {
    //Create L base name
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());
    
    //Create real L variable
    std::string real_name = L_name;
    real_name.append("_real");
    

#ifdef DEBUG
    std::cerr << "Real name = " << real_name << std::endl;
#endif
    
    _problem->addVariable(real_name,
                          FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                 Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
                          getParam<Real>("scaling"));

    if (l > 0)
    {
      //Create imaginary L variable IF l > 0
      std::string imag_name = L_name;
      imag_name.append("_imag");

#ifdef DEBUG
      std::cerr << "Imaginary name = " << imag_name << std::endl;
#endif
      
        _problem->addVariable(imag_name,
                              FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                     Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
                              getParam<Real>("scaling"));
    } 
    
  }

}
