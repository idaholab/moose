/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#ifndef GrandPotentialTensorMaterial_H
#define GrandPotentialTensorMaterial_H

#include "PolycrystalDiffusivityTensorBase.h"
#include "DerivativeMaterialPropertyNameInterface.h"

// Forward Declarations
class GrandPotentialTensorMaterial;

template <>
InputParameters validParams<GrandPotentialTensorMaterial>();

/**
 * Calculates mobilities for grand potential model. The potential mobility (\chi*D)
 * is a tensor, while the Allen Cahn mobilities for the solid and void phases are
 * scalars.
 */
class GrandPotentialTensorMaterial : public PolycrystalDiffusivityTensorBase
{
public:
  GrandPotentialTensorMaterial(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  std::string _D_name;  /// mobility tensor
  MaterialProperty<RealTensorValue> & _chiD;
  MaterialProperty<RealTensorValue> & _dchiDdc;
  std::string _Ls_name; /// grain boundary mobility name
  std::string _Lv_name; /// void mobility name
  MaterialProperty<Real> & _Ls;
  MaterialProperty<Real> & _Lv;
  MaterialProperty<Real> & _Dmag; /// magnitude of mobility tensor
  const MaterialProperty<Real> & _sigma_s; /// surface energy

  Real _int_width; /// interface width
  const MaterialPropertyName _chi_name; /// susceptibility
  const MaterialProperty<Real> & _chi;
  const MaterialProperty<Real> & _dchidc;
  Real _GBMobility;
  Real _GBmob0;
  const Real _Q;
  std::vector<NonlinearVariableName> _vals_name; /// solid phase order parameters
  std::vector<const MaterialProperty<Real> *> _dchideta;
  std::vector<MaterialProperty<RealTensorValue> *> _dchiDdeta;
};

#endif // GrandPotentialTensorMaterial_H
