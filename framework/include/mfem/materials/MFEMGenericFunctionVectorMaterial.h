#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMaterial.h"

/**
 * Declares material properties based on names and functions prescribed by input parameters.
 *
 * This is identical in function to the GenericFunctionVectorMaterial in Moose.
 */
class MFEMGenericFunctionVectorMaterial : public MFEMMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericFunctionVectorMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericFunctionVectorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<FunctionName> & _prop_values;
  unsigned int _num_props;
};

#endif
