#pragma once

#include "GeneralUserObject.h"
#include "inputs.hpp"
#include "gridfunctions.hpp"

class MFEMFESpace : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMFESpace(const InputParameters & parameters);
  virtual ~MFEMFESpace();
  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  static const std::string createFECName(const std::string & fespace_type, const int order);
  const int order;
  const int vdim;
  const std::string fespace_type;
  const std::string fec_name;
};
