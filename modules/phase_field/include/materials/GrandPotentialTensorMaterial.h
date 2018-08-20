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

#ifndef GRANDPOTENTIALTENSORMATERIAL_H
#define GRANDPOTENTIALTENSORMATERIAL_H

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
  /// mobility tensor
  std::string _D_name;
  MaterialProperty<RealTensorValue> & _chiD;
  MaterialProperty<RealTensorValue> & _dchiDdc;

  /// grain boundary mobility
  std::string _Ls_name;
  MaterialProperty<Real> & _Ls;

  /// void mobility
  std::string _Lv_name;
  MaterialProperty<Real> & _Lv;

  /// magnitude of mobility tensor
  MaterialProperty<Real> & _Dmag;

  /// surface energy
  const MaterialProperty<Real> & _sigma_s;

  /// interface width
  Real _int_width;

  /// susceptibility
  const MaterialPropertyName _chi_name;
  const MaterialProperty<Real> & _chi;
  const MaterialProperty<Real> & _dchidc;
  std::vector<const MaterialProperty<Real> *> _dchideta;
  std::vector<MaterialProperty<RealTensorValue> *> _dchiDdeta;

  Real _GBMobility;
  Real _GBmob0;
  const Real _Q;

  /// solid phase order parameters
  std::vector<NonlinearVariableName> _vals_name;
};

#endif // GRANDPOTENTIALTENSORMATERIAL_H
