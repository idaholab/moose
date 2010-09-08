#ifndef CONVECTIONDIFFUSIONBLOCK_H
#define CONVECTIONDIFFUSIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class ConvectionDiffusionBlock;

template<>
InputParameters validParams<ConvectionDiffusionBlock>();

class ConvectionDiffusionBlock: public ParserBlock
{
public:
  ConvectionDiffusionBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::vector<std::string> _boundary_list;
};

#endif //CONVECTIONDIFFUSIONBLOCK_H
