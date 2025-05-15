#ifdef MFEM_ENABLED

#pragma once
#include "MFEMFunctorMaterial.h"

/**
 * Declares material properties based on names and values prescribed by input parameters.
 *
 * This is identical in function to the GenericConstantVectorMaterial in Moose.
 */
class MFEMGenericConstantVectorFunctorMaterial : public MFEMFunctorMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericConstantVectorFunctorMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericConstantVectorFunctorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<Real> & _prop_values;
  const int _prop_dims;
};

#endif
