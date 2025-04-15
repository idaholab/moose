#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMaterial.h"

/**
 * Declares material properties based on names and values prescribed by input parameters.
 *
 * This is identical in function to the GenericConstantVectorMaterial in Moose.
 */
class MFEMGenericConstantVectorMaterial : public MFEMMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericConstantVectorMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericConstantVectorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<Real> & _prop_values;
  const int _prop_dims;
};

#endif
