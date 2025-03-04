#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"

/*
Class to construct an MFEM integrator to apply to the equation system.
*/
class MFEMKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams()
  {
    InputParameters params = MFEMGeneralUserObject::validParams();
    params.registerBase("Kernel");
    params.addParam<VariableName>("variable",
                                  "Variable labelling the weak form this kernel is added to");
    params.addParam<std::vector<SubdomainName>>("block",
                                                {},
                                                "The list of blocks (ids or names) that this "
                                                "object will be applied to. Leave empty to apply "
                                                "to all blocks.");
    return params;
  }

  MFEMKernel(const InputParameters & parameters)
    : MFEMGeneralUserObject(parameters),
      _test_var_name(getParam<VariableName>("variable")),
      _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
      _subdomain_attributes(_subdomain_names.size())
  {
    for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
    {
      _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
    }
  }

  virtual ~MFEMKernel() = default;

  // Create a new MFEM integrator to apply to the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() { return nullptr; }
  virtual mfem::BilinearFormIntegrator * createIntegrator() { return nullptr; }

  // Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const { return _test_var_name; }

  bool isSubdomainRestricted() { return _subdomain_names.size(); }

  mfem::Array<int> & getSubdomains() { return _subdomain_attributes; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName _test_var_name;
  std::vector<SubdomainName> _subdomain_names;
  mfem::Array<int> _subdomain_attributes;
};

#endif
