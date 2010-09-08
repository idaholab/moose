/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef GLOBALPARAMSBLOCK_H
#define GLOBALPARAMSBLOCK_H

#include "ParserBlock.h"
#include "Moose.h"

class GlobalParamsBlock;

template<>
InputParameters validParams<GlobalParamsBlock>();

class GlobalParamsBlock: public ParserBlock
{
public:
  GlobalParamsBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

  template <typename T>
  inline
  T & setScalarParam(const std::string &name)
  {
    return getBlockParams().set<T>(name);
  }

  template <typename T>
  inline
  std::vector<T> & setVectorParam(const std::string &name)
  {
    return getBlockParams().set<std::vector<T> >(name);
  }

  template <typename T>
  inline
  std::vector<std::vector<T> > & setTensorParam(const std::string &name)
  {
    return getBlockParams().set<std::vector<std::vector<T> > >(name);
  }
};

#endif //GLOBALPARAMSBLOCK_H
