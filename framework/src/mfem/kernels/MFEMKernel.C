#ifdef MFEM_ENABLED

#include "MFEMKernel.h"

InputParameters
MFEMKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Kernel");
  params.addParam<std::string>("variable",
                               "Variable labelling the weak form this kernel is added to");
  params.addParam<std::vector<SubdomainName>>("block",
                                              {},
                                              "The list of blocks (ids) that this "
                                              "object will be applied to. Leave empty to apply "
                                              "to all blocks.");
  return params;
}

MFEMKernel::MFEMKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _test_var_name(getParam<std::string>("variable")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
  {
    _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
  }
}

#endif
