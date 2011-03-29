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
 * Generic gap heat transfer model, with h_gap = h_radiation + h_conduction + h_contact
 */

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real h_conduction();
  virtual Real h_contact();
  virtual Real h_radiation();
  virtual Real gapK();
  virtual Real gapLength();
  

  NumericVector<Number> & _slave_flux;

  VariableValue & _gap_distance;
  VariableValue & _gap_temp;
  
  const Real _gap_conductivity;
  const Real _roughness_1;
  const Real _roughness_2;

  Real _max_gap;
};

#endif //GAPHEATTRANSFER_H
