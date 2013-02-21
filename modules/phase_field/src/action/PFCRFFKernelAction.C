#include "PFCRFFKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<PFCRFFKernelAction>()
{
  InputParameters params = validParams<Action>();
  
  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("n_name","Variable name used for the n variable");
  params.addRequiredParam<std::string>("L_name_base","Base name for the complex L variables");
  params.addParam<std::string>("mob_name","M","The mobility used for n in this model");
  
  return params;
}

PFCRFFKernelAction::PFCRFFKernelAction(const std::string & name, InputParameters params)
  :Action(name, params),
   _num_L(getParam<unsigned int>("num_L")),
   _L_name_base(getParam<std::string>("L_name_base")),
   _n_name(getParam<std::string>("n_name"))
{
}

void
PFCRFFKernelAction::act() 
{
#ifdef DEBUG
  std::cerr << "Inside the PFCRFFKernelAction Object\n";
  std::cerr << "L name base:" << _L_name_base;
#endif

  //Create the two kernels required for the n_variable, starting with the time derivative
  InputParameters poly_params = _factory.getValidParams("TimeDerivative");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
    
  _problem->addKernel("TimeDerivative", "IE_n", poly_params);
  
  //Now the CHPFCRFF kernel
  std::vector<std::string> real_v; //First, we have to create the vector containing the names of the real L variables
  real_v.resize(_num_L);
  for (unsigned int l = 0; l<_num_L; l++)
  {
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());
    L_name.append("_real");
    real_v[l] = L_name;
  }
  
  poly_params = _factory.getValidParams("CHPFCRFF");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<std::vector<std::string> >("v") = real_v;
  poly_params.set<std::string>("mob_name") = getParam<std::string>("mob_name");
    
  _problem->addKernel("CHPFCRFF", "CH_bulk_n", poly_params);

  //Loop over the L_variables
  for (unsigned int l = 0; l<_num_L; l++)
  {
    //Create L base name
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());

    //Create real  and imaginary L variable names
    std::string real_name = L_name;
    real_name.append("_real");
    std::string imag_name = L_name;
    imag_name.append("_imag");

    //Create the kernels for the real L variable ***********************************
    //**Create the diffusion kernel for L_real_l
    InputParameters poly_params = _factory.getValidParams("Diffusion");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    
    std::string kernel_name = "diff_";
    kernel_name.append(real_name);
    
    _problem->addKernel("Diffusion", kernel_name, poly_params);

    //**Create the (alpha^R_m L^R_m) term
    poly_params = _factory.getValidParams("HHPFCRFF");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("positive") = true;

    std::string pname = "alpha_R_";
    pname.append(out.str());
    poly_params.set<std::string>("prop_name") = pname;

    kernel_name = "HH1_";
    kernel_name.append(real_name);
    
    _problem->addKernel("HHPFCRFF", kernel_name, poly_params);
    
    //**Create the -(alpha^I_m L^I_m) term
    if (l > 0)
    {
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = real_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<std::string> >("coupled_var").push_back(imag_name);

      pname = "alpha_I_";
      pname.append(out.str());
      poly_params.set<std::string>("prop_name") = pname;

      kernel_name = "HH2_";
      kernel_name.append(real_name);
      
      _problem->addKernel("HHPFCRFF", kernel_name, poly_params);
    }
    
    //**Create the -(A^R_m n) term
    poly_params = _factory.getValidParams("HHPFCRFF");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("positive") = false;
    poly_params.set<std::vector<std::string> >("coupled_var").push_back(_n_name);

    pname = "A_R_";
    pname.append(out.str());
    poly_params.set<std::string>("prop_name") = pname;

    kernel_name = "HH3_";
    kernel_name.append(real_name);
    
    _problem->addKernel("HHPFCRFF", kernel_name, poly_params);
    //Create the kernels for the imaginary L variable, l > 0 ***********************************
    if (l > 0)
    {
      //**Create the diffusion kernel for L_imag_l
      InputParameters poly_params = _factory.getValidParams("Diffusion");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
    
      kernel_name = "diff_";
      kernel_name.append(imag_name);
    
      _problem->addKernel("Diffusion", kernel_name, poly_params);

      //**Create the (alpha^R_m L^I_m) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = true;

      pname = "alpha_R_";
      pname.append(out.str());
      poly_params.set<std::string>("prop_name") = pname;

      kernel_name = "HH1_";
      kernel_name.append(imag_name);
      
      _problem->addKernel("HHPFCRFF", kernel_name, poly_params);

      //**Create the (alpha^I_m L^R_m) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = true;
      poly_params.set<std::vector<std::string> >("coupled_var").push_back(real_name);

      pname = "alpha_I_";
      pname.append(out.str());
      poly_params.set<std::string>("prop_name") = pname;

      kernel_name = "HH2_";
      kernel_name.append(imag_name);
    
      _problem->addKernel("HHPFCRFF", kernel_name, poly_params);

      //**Create the -(A^I_m n) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<std::string> >("coupled_var").push_back(_n_name);

      pname = "A_I_";
      pname.append(out.str());
      poly_params.set<std::string>("prop_name") = pname;

      kernel_name = "HH3_";
      kernel_name.append(imag_name);
    
      _problem->addKernel("HHPFCRFF", kernel_name, poly_params);
      //*******************
    }
  }
  
  
}
