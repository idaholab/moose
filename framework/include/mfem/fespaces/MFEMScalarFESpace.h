#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSimplifiedFESpace.h"

class MFEMScalarFESpace : public MFEMSimplifiedFESpace
{
public:
  static InputParameters validParams();

  MFEMScalarFESpace(const InputParameters & parameters);

protected:
  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const;

private:
  /// Name of the family of finite element collections to use
  const std::string _fec_type;
};

#endif
