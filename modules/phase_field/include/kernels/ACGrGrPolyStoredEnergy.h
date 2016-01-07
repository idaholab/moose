/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACBulk.h"
#include "GrainTrackerInterface.h"

#ifndef ACGRGRPOLYSTOREDENERGY_H
#define ACGRGRPOLYSTOREDENERGY_H

//Forward Declarations
class ACGrGrPolyStoredEnergy;

template<>
InputParameters validParams<ACGrGrPolyStoredEnergy>();

/**
 * This kernel adds the residual contribution for stored (dislocation) energy
 * in a polycrystaline system. The free nergy term is \f$ E_{\text{stored}}(3\eta^2 - 2\eta^3) \f$
 */
class ACGrGrPolyStoredEnergy : public ACBulk
{
public:
  ACGrGrPolyStoredEnergy(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int);

  std::vector<Real> _stored_energy;
  const GrainTrackerInterface & _grain_tracker;
  const unsigned int _op_index;
};

#endif //ACGRGRPOLYSTOREDENERGY_H
