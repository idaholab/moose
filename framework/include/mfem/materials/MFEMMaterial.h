#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMCoefficient.h"
#include "PropertyManager.h"
#include "coefficients.h"

class MFEMMaterial : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMMaterial(const InputParameters & parameters);
  virtual ~MFEMMaterial();

protected:
  std::vector<SubdomainName> _block_ids;
  platypus::PropertyManager & _properties;
};
