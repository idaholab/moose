/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALEBSD_H
#define POLYCRYSTALEBSD_H

#include "PolycrystalUserObjectBase.h"

// Forward Declarations
class PolycrystalEBSD;
class EBSDReader;

template <>
InputParameters validParams<PolycrystalEBSD>();

class PolycrystalEBSD : public PolycrystalUserObjectBase
{
public:
  PolycrystalEBSD(const InputParameters & parameters);

  virtual unsigned int getGrainBasedOnPoint(const Point & point) const override;
  virtual unsigned int getNumGrains() const override;

protected:
  const EBSDReader & _ebsd_reader;
  const bool _consider_phase;
  const unsigned int _phase;
};

#endif // POLYCRYSTALEBSD_H
