#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMaterial.h"

/**
 * Declares material properties based on names and functions prescribed by input parameters.
 *
 * This is identical in function to the GenericFunctionMaterial in Moose.
 */
class MFEMGenericFunctionMaterial : public MFEMMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericFunctionMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericFunctionMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<FunctionName> & _prop_values;
};

#endif
