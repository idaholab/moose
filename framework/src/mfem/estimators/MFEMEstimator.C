#ifdef MFEM_ENABLED

#include "MFEMEstimator.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMEstimator);

// static method
InputParameters
MFEMEstimator::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Indicator");

  params.addRequiredParam<std::string>("variable", "Variable to perform amr with");
  params.addRequiredParam<std::string>("kernel", "Kernel to perform amr with");
  params.addRequiredParam<std::string>("fe_space", "FESpace to set order from");

  return params;
}


MFEMEstimator::MFEMEstimator(const InputParameters & params)
  : MFEMGeneralUserObject(params), _test_var_name( getParam<std::string>("variable") ),
    _kernel_name( getParam<std::string>("kernel") ), _fe_space_name( getParam<std::string>("fe_space") )
{}

std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMEstimator::getFESpace()
{
  MFEMProblemData & problem = getMFEMProblem().getProblemData();
  return problem.fespaces.GetShared( _fe_space_name );
}


#endif
