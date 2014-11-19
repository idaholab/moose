#include "PolycrystalKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<PolycrystalKernelAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<VariableName>("c", "NONE", "Name of coupled concentration variable");
  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface to GB energy");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<VariableName>("T", "NONE", "Name of temperature variable");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  return params;
}

PolycrystalKernelAction::PolycrystalKernelAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _c(getParam<VariableName>("c")),
    _implicit(getParam<bool>("implicit")),
    _T(getParam<VariableName>("T"))
{
}

void
PolycrystalKernelAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolyCrystalKernelAction Object\n";
  Moose::err << "var name base:" << _var_name_base;
#endif
  // Moose::out << "Implicit = " << _implicit << Moose::out;

  for (unsigned int op = 0; op < _op_num; op++)
  {
    //Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    std::vector<VariableName> v;
    v.resize(_op_num - 1);

    unsigned int ind = 0;

    for (unsigned int j = 0; j < _op_num; j++)
    {
      if (j != op)
      {
        std::string coupled_var_name = _var_name_base;
        std::stringstream out2;
        out2 << j;
        coupled_var_name.append(out2.str());
        v[ind] = coupled_var_name;
        ind++;
      }
    }

    InputParameters poly_params = _factory.getValidParams("ACGrGrPoly");
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<std::vector<VariableName> >("v") = v;
    poly_params.set<bool>("implicit")=_implicit;
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    if (_T != "NONE")
      poly_params.set<std::vector<VariableName> >("T").push_back(_T);

    std::string kernel_name = "ACBulk_";
    kernel_name.append(var_name);

    _problem->addKernel("ACGrGrPoly", kernel_name, poly_params);

    /************/

    poly_params = _factory.getValidParams("ACInterface");
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<bool>("implicit")=getParam<bool>("implicit");
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    kernel_name = "ACInt_";
    kernel_name.append(var_name);

    _problem->addKernel("ACInterface", kernel_name, poly_params);
    //*******************

    poly_params = _factory.getValidParams("TimeDerivative");
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<bool>("implicit") = true;
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    kernel_name = "IE_";
    kernel_name.append(var_name);

    _problem->addKernel("TimeDerivative", kernel_name, poly_params);
    /************/
    if (_c != "NONE")//Add in Bubble interaction kernel, if using bubbles
    {
      poly_params = _factory.getValidParams("ACGBPoly");
      poly_params.set<NonlinearVariableName>("variable") = var_name;
      poly_params.set<std::vector<VariableName> >("c").push_back(_c);
      poly_params.set<Real>("en_ratio") = getParam<Real>("en_ratio");
      poly_params.set<bool>("implicit")=getParam<bool>("implicit");
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      kernel_name = "ACBubInteraction_";
      kernel_name.append(var_name);

      _problem->addKernel("ACGBPoly", kernel_name, poly_params);
    }
  }
}
