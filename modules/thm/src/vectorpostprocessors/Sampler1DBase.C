#include "Sampler1DBase.h"

template <>
InputParameters
validParams<Sampler1DBase<Real>>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params += validParams<SamplerBase>();
  params += validParams<BlockRestrictable>();

  params.addRequiredParam<std::vector<std::string>>(
      "property", "Names of the material properties to be output along a line");

  // This parameter exists in BlockRestrictable, but it is made required here
  // because it is undesirable to use the default, which is to add all blocks.
  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) for which this object will be applied");

  return params;
}
