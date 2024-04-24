#pragma once

#include "GeneralUserObject.h"
#include "MFEMCoefficient.h"
#include "coefficients.hpp"

class MFEMMaterial : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMMaterial(const InputParameters & parameters);
  virtual ~MFEMMaterial();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  virtual void storeCoefficients(hephaestus::Subdomain & subdomain) {}

  std::vector<SubdomainName> blocks;
};
