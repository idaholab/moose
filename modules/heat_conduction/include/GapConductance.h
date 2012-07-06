#ifndef GAPCONDUCTANCE_H
#define GAPCONDUCTANCE_H

#include "Material.h"

class GapConductance : public Material
{
public:

  GapConductance(const std::string & name, InputParameters parameters);

  virtual ~GapConductance(){}

  static  Real gapLength(Real distance, Real min_gap, Real max_gap);

protected:
/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */

  virtual void computeQpProperties();

  virtual Real h_conduction();
  virtual Real dh_conduction();
  virtual Real gapK();

  Real _gap_temp;
  const VariableValue & _gap_distance;
  MaterialProperty<Real> & _gap_conductance;
  MaterialProperty<Real> & _gap_conductance_dT;

  const Real _gap_conductivity;

  Real _min_gap;
  Real _max_gap;

private:

};

template<>
InputParameters validParams<GapConductance>();

#endif //GAPCONDUCTANCE_H
