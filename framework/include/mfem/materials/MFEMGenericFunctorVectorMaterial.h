#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMaterial.h"
#include "MFEMContainers.h"

/**
 * Declares material properties based on names and functions prescribed by input parameters.
 *
 * This is identical in function to the GenericFunctionVectorMaterial in Moose.
 */
class MFEMGenericFunctorVectorMaterial : public MFEMMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericFunctorVectorMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericFunctorVectorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<MFEMVectorCoefficientName> & _prop_values;
};

#endif
