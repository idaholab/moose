#include "PolycrystalElDrForceAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<PolycrystalElDrForceAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Action that addes the elastic driving force for each order parameter");
  params.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

PolycrystalElDrForceAction::PolycrystalElDrForceAction(const std::string & name,
                                                     InputParameters params) :
    Action(name, params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalElDrForceAction::act()
{
  #ifdef DEBUG
    Moose::err << "Inside the PolycrystalElDrForceAction Object\n";
    Moose::err << "var name base:" << _var_name_base;
  #endif

  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Create variable name
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    // Create Stiffness derivative name
    MaterialPropertyName D_stiff_name = "D_elastic_tensor";
    D_stiff_name.append(out.str());

    //Set name of kernel being created
    std::string kernel_type = "ACGrGrElDrForce";

    // Set the actual parameters for the kernel
    InputParameters poly_params = _factory.getValidParams(kernel_type);
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<MaterialPropertyName>("D_tensor_name") = D_stiff_name;
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    std::string kernel_name = "AC_ElDrForce_";
    kernel_name.append(var_name);

    // Create kernel
    _problem->addKernel(kernel_type, kernel_name, poly_params);
  }
}
