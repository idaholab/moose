#ifndef GENERICPOSTPROCESSORBLOCK_H_
#define GENERICPOSTPROCESSORBLOCK_H_

#include "ParserBlock.h"
#include "Moose.h"

//Forward Declarations
class GenericPostprocessorBlock;

template<>
InputParameters validParams<GenericPostprocessorBlock>();

class GenericPostprocessorBlock: public ParserBlock
{
public:
  GenericPostprocessorBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  Moose::PostprocessorType _pps_type;

  std::string _type;
};

  

#endif //GENERICPOSTPROCESSORBLOCK_H_
