#ifndef BARRIERFUNCTIONMATERIAL_H
#define BARRIERFUNCTIONMATERIAL_H

#include "OrderParameterFunctionMaterial.h"

// Forward Declarations
class BarrierFunctionMaterial;

template<>
InputParameters validParams<BarrierFunctionMaterial>();

/**
 * Material class to provide the double well function \f$ g(\eta) \f$ for
 * the KKS system.
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 */
class BarrierFunctionMaterial : public OrderParameterFunctionMaterial
{
public:
  BarrierFunctionMaterial(const std::string & name,
                          InputParameters parameters);

protected:
  virtual void computeQpProperties();

  /// Polynomial order of the switching function \f$ h(\eta) \f$
  MooseEnum _g_order;
};

#endif //BARRIERFUNCTIONMATERIAL_H
