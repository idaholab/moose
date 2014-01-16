#ifndef HEATCONDUCTIONBC_H
#define HEATCONDUCTIONBC_H

#include "FluxBC.h"

class HeatConductionBC;

template<>
InputParameters validParams<HeatConductionBC>();

/**
 *
 */
class HeatConductionBC : public FluxBC
{
public:
  HeatConductionBC(const std::string & name, InputParameters parameters);
  virtual ~HeatConductionBC();

protected:
  virtual RealGradient computeQpFluxResidual();
  virtual RealGradient computeQpFluxJacobian();

  MaterialProperty<Real> & _k;
};


#endif /* HEATCONDUCTIONBC_H */
