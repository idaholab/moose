#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMCoefficient.h"
#include "PropertyManager.h"
#include "coefficients.h"

class MFEMMaterial : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();
  static std::vector<std::string> subdomainsToStrings(std::vector<SubdomainName> blocks);

  MFEMMaterial(const InputParameters & parameters);
  virtual ~MFEMMaterial();

  const std::vector<SubdomainName> & getBlocks() const { return _block_ids; }

protected:
  std::vector<SubdomainName> _block_ids;
  platypus::PropertyManager & _properties;
};
