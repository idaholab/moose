/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef GENERICVARIABLEBLOCK_H
#define GENERICVARIABLEBLOCK_H

#include "InputParameters.h"
#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;

template<>
InputParameters validParams<GenericVariableBlock>();

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

  bool restartRequired() const;
  bool autoResizeable() const;
  std::pair<std::string, unsigned int> initialValuePair() const;

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
  unsigned int _timestep_to_read;
};

#endif //GENERICVARIABLEBLOCK_H
