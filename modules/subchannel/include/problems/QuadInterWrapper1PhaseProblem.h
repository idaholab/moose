/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "InterWrapper1PhaseProblem.h"

class QuadInterWrapper1PhaseProblem;

/**
 * Quadrilateral interwrapper solver
 */
class QuadInterWrapper1PhaseProblem : public InterWrapper1PhaseProblem
{
public:
  QuadInterWrapper1PhaseProblem(const InputParameters & params);

protected:
  virtual Real computeFrictionFactor(Real Re) override;

public:
  static InputParameters validParams();
};
