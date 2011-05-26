#ifndef GAPHEATTRANSFER_H
#define GAPHEATTRANSFER_H

#include "IntegratedBC.h"

//Forward Declarations
class GapHeatTransfer;

template<>
InputParameters validParams<GapHeatTransfer>();

class GapHeatTransfer : public IntegratedBC
{
public:

  GapHeatTransfer(const std::string & name, InputParameters parameters);

  virtual ~GapHeatTransfer(){}

protected:
/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real h_conduction();
  virtual Real h_contact();
  virtual Real h_radiation();
  virtual Real dh_conduction();
  virtual Real dh_contact();
  virtual Real dh_radiation();
  virtual Real gapK();
  virtual Real gapLength();

  virtual Real computeSlaveFluxContribution(Real grad_t);

  NumericVector<Number> & _slave_flux;

  VariableValue & _gap_distance;
  VariableValue & _gap_temp;

  const Real _gap_conductivity;

  Real _min_gap;
  Real _max_gap;
};

#endif //GAPHEATTRANSFER_H
