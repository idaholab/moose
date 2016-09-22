#ifndef HEATEXCHANGECOEFFICIENTPARTITIONING_H
#define HEATEXCHANGECOEFFICIENTPARTITIONING_H

#include "GeneralUserObject.h"

class HeatExchangeCoefficientPartitioning;

template<>
InputParameters validParams<HeatExchangeCoefficientPartitioning>();

/**
 * Computes the partitioning of the interphase heat exchange coefficients
 */
class HeatExchangeCoefficientPartitioning : public GeneralUserObject
{
public:
  HeatExchangeCoefficientPartitioning(const InputParameters & parameters);
  virtual ~HeatExchangeCoefficientPartitioning();

  virtual void execute() {};
  virtual void initialize() {};
  virtual void finalize() {};

  virtual Real getPartition(Real alpha_liquid, Real dalpha_liquid_dt) const;
  virtual Real getPartitionDer(Real alpha_liquid, Real dalpha_liquid_dt, Real area) const;

protected:
  /// Upper cut-off limit
  Real _lower;
  /// Lower cut-off limit
  Real _upper;
};


#endif /* HEATEXCHANGECOEFFICIENTPARTITIONING_H */
