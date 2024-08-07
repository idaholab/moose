#include "MFEMBilinearFormKernel.h"

InputParameters
MFEMBilinearFormKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += BlockRestrictable::validParams();

  params.registerBase("Kernel");
  params.addParam<std::string>("variable", "Variable on which to apply the kernel");

  return params;
}

MFEMBilinearFormKernel::MFEMBilinearFormKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), _elem_attr(1)
{
  // int i = 0;
  // for (auto const & blockID : blockIDs())
  // {
  //   _elem_attr[i] = blockID;
  //   i++;
  // }
}
