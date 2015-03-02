/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef NODALFLOODCOUNTAUX_H
#define NODALFLOODCOUNTAUX_H

#include "AuxKernel.h"

//Forward Declarations
class NodalFloodCountAux;
class NodalFloodCount;

template<>
InputParameters validParams<NodalFloodCountAux>();

/**
 * Function auxiliary value
 */
class NodalFloodCountAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NodalFloodCountAux(const std::string & name, InputParameters parameters);

  virtual ~NodalFloodCountAux() {}

protected:
  virtual Real computeValue();

  /// Function being used to compute the value of this kernel
  const NodalFloodCount & _flood_counter;

  const unsigned int _var_idx;
  const MooseEnum _field_display;
  bool _var_coloring;
};

#endif // NODALFLOODCOUNTAUX_H
