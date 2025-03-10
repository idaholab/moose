#ifdef MFEM_ENABLED

#pragma once
#include "MFEMFESpace.h"

/**
 * Constructs and stores an mfem::ParFiniteElementSpace object. Access using the
 * getFESpace() accessor.
 */
class MFEMGenericFESpace : public MFEMFESpace
{
public:
  static InputParameters validParams();

  MFEMGenericFESpace(const InputParameters & parameters);

protected:
  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const;

private:
  /// The name of the finite element collection
  const std::string _fec_name;

  /// The number of degrees of freedom per basis function
  const int _vdim;
};

#endif
