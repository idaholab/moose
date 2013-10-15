#include "PolycrystalKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<PolycrystalKernelAction>()
{
  InputParameters params = validParams<Action>();
  
  params.addRequiredParam<unsigned int>("crys_num", "specifies the number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<bool>("with_bub",false,"specifies if the grain growth simulation is run with a bubble model or by itself");
  params.addParam<std::string>("c_name","c","Name of coupled concentration variable");
  params.addParam<Real>("en_ratio","Ratio of surface to GB energy");
  params.addParam<bool>("implicit",true,"Whether kernels are implicit or not");
  params.addParam<bool>("tgrad_correction",false,"Add in correction factor to cancel out false temperature gradient driving force");
  params.addParam<VariableName>("T","Name of coupled concentration variable");
  
  return params;
}

PolycrystalKernelAction::PolycrystalKernelAction(const std::string & name, InputParameters params)
  :Action(name, params),
   _crys_num(getParam<unsigned int>("crys_num")),
   _var_name_base(getParam<std::string>("var_name_base")),
   _with_bub(getParam<bool>("with_bub")),
   _c_name(getParam<std::string>("c_name")),
   _en_ratio(getParam<Real>("en_ratio")),
   _implicit(getParam<bool>("implicit")),
   _T(getParam<VariableName>("T"))
{}

void
PolycrystalKernelAction::act() 
{
#ifdef DEBUG
  std::cerr << "Inside the PolyCrystalKernelAction Object\n";
  std::cerr << "var name base:" << _var_name_base;
#endif
  // std::cout << "Implicit = " << _implicit << std::cout;
  
  
  for (unsigned int crys = 0; crys<_crys_num; crys++)
  {
    //Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << crys;
    var_name.append(out.str());

    std::vector<VariableName> v;
    v.resize(_crys_num - 1);

    unsigned int ind = 0;

    for (unsigned int j = 0; j<_crys_num; j++)
    {
      
      if (j != crys) 
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
    poly_params.set<bool>("tgrad_correction")=getParam<bool>("tgrad_correction");
    if (!_T.empty())
      poly_params.set<std::vector<VariableName> >("T").push_back(_T);
    
    std::string kernel_name = "ACBulk_";
    kernel_name.append(var_name);
    
    _problem->addKernel("ACGrGrPoly", kernel_name, poly_params);

    /************/
    
    poly_params = _factory.getValidParams("ACInterface");
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<bool>("implicit")=getParam<bool>("implicit");

    kernel_name = "ACInt_";
    kernel_name.append(var_name);
    
    _problem->addKernel("ACInterface", kernel_name, poly_params);
    //*******************
    
    poly_params = _factory.getValidParams("TimeDerivative");
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<bool>("implicit") = true;
    
    kernel_name = "IE_";
    kernel_name.append(var_name);
    
    _problem->addKernel("TimeDerivative", kernel_name, poly_params);
    /************/
    if (_with_bub)//Add in Bubble interaction kernel, if using bubbles
    {
      poly_params = _factory.getValidParams("ACGBPoly");
      poly_params.set<NonlinearVariableName>("variable") = var_name;
      std::vector<VariableName> c_bin(1);
      c_bin[0] = _c_name;
      
      poly_params.set<std::vector<VariableName> >("c") =c_bin;
      poly_params.set<Real>("en_ratio") = _en_ratio;
      poly_params.set<bool>("implicit")=getParam<bool>("implicit");

      kernel_name = "ACBubInteraction_";
      kernel_name.append(var_name);
    
      _problem->addKernel("ACGBPoly", kernel_name, poly_params);
    }
  }
  
  
}
