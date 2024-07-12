#pragma once
#include "MFEMGeneralUserObject.h"

class MFEMFESpace : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMFESpace(const InputParameters & parameters);
  virtual ~MFEMFESpace();

  static const std::string createFECName(const std::string & fespace_type, const int order);
  const int order;
  const int vdim;
  const std::string fespace_type;
  const std::string fec_name;
};
