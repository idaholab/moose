#ifndef HEATCONDUCTIONBC_H
#define HEATCONDUCTIONBC_H

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
  virtual RealGradient computeQpFluxResidual() = 0;
  virtual RealGradient computeQpFluxJacobian() = 0;

  MaterialProperty<Real> & _k;
};


#endif /* HEATCONDUCTIONBC_H */
