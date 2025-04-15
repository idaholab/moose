#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMaterial.h"

/**
 * Declares material properties based on names and values prescribed by input parameters.
 *
 * This is identical in function to the GenericConstantMaterial in Moose.
 */
class MFEMGenericConstantMaterial : public MFEMMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericConstantMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericConstantMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<Real> & _prop_values;
};

#endif
