#ifndef NODALAREAAUX_H
#define NODALAREAAUX_H

#include "AuxKernel.h"

class NodalArea;

class NodalAreaAux : public AuxKernel
{
public:

  NodalAreaAux(const std::string & name, InputParameters parameters);

  virtual ~NodalAreaAux();

protected:
  virtual Real computeValue();

  const NodalArea & _nodal_area;
};

template<>
InputParameters validParams<NodalAreaAux>();

#endif
