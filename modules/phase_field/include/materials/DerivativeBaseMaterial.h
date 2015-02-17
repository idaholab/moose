#ifndef DERIVATIVEBASEMATERIAL_H
#define DERIVATIVEBASEMATERIAL_H

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations
class DerivativeBaseMaterial;

template<>
InputParameters validParams<DerivativeBaseMaterial>();

/**
 * %Material base class central to compute the a phase free energy and
 * its derivatives. Classes derived from this base class are central to
 * the KKS system. The calculation of free energies is centralized here
 * as the results are used in multiple kernels (KKSPhaseChemicalPotential
 * and \ref KKSCHBulk).
 *
 * A DerivativeBaseMaterial provides numerous material properties which contain
 * the free energy and its derivatives. The material property names are
 * constructed dynamically by the helper functions propertyNameFirst(),
 * propertyNameSecond(), and propertyNameThird() in DerivativeMaterialInterface.
 *
 * A derived class needs to implement the computeF(), computeDF(),
 * computeD2F(), and (optionally) computeD3F() methods.
 *
 * Note that DerivativeParsedMaterial provides a material for which a mathematical
 * free energy expression can be provided in the input file (with the
 * derivatives being calculated automatically).
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 * \see DerivativeParsedMaterial
 * \see DerivativeMaterialInterface
 */
class DerivativeBaseMaterial : public DerivativeFunctionMaterialBase
{
public:
  DerivativeBaseMaterial(const std::string & name, InputParameters parameters);
};

#endif //DERIVATIVEBASEMATERIAL_H
