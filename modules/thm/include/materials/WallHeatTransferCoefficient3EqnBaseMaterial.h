#ifndef WALLHEATTRANSFERCOEFFICIENT3EQNBASEMATERIAL_H
#define WALLHEATTRANSFERCOEFFICIENT3EQNBASEMATERIAL_H

#include "Material.h"
#include "PipeBase.h"

class WallHeatTransferCoefficient3EqnBaseMaterial;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<WallHeatTransferCoefficient3EqnBaseMaterial>();

/**
 * Base class for computing wall heat transfer coefficient for pipe and rod bundle geometry for
 * single phase flow
 */
class WallHeatTransferCoefficient3EqnBaseMaterial : public Material
{
public:
  WallHeatTransferCoefficient3EqnBaseMaterial(const InputParameters & parameters);

protected:
  MaterialProperty<Real> & _Hw;
  PipeBase::EConvHeatTransGeom _ht_geom;
  const Real & _PoD;
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _vel;
  const VariableValue & _D_h;
  const MaterialProperty<Real> & _v;
  const MaterialProperty<Real> & _e;
  const MaterialProperty<Real> & _T;
  const MaterialProperty<Real> & _pressure;
  bool _has_q_wall;
  const VariableValue * _q_wall;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* WALLHEATTRANSFERCOEFFICIENT3EQNBASEMATERIAL_H */
