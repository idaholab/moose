/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RADIATIONSOURCE_H
#define RADIATIONSOURCE_H

#include "Kernel.h"

//Forward Declarations
class RadiationSource;

template<>
InputParameters validParams<RadiationSource>();

/**
 * Vacancy or interstitial source term
 */
class RadiationSource : public Kernel
{
public:
  RadiationSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual void subdomainSetup();

private:
  /// Type of defect created due to radiation (vacancy or interstitial)
  MooseEnum _defect_type;

  /// Material property providing incease in vacancy concentration due to radiation
  const MaterialProperty<Real> & _vacancy_increase;
  /// Material property providing incease in interstitial concentration due to radiation
  const MaterialProperty<Real> & _interstitial_increase;
};

#endif //RADIATIONSOURCE_H
