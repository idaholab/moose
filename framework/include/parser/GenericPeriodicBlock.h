#ifndef GENERICPERIODICBLOCK_H_
#define GENERICPERIODICBLOCK_H_

#include "ParserBlock.h"
#include "FunctionPeriodicBoundary.h"

class GenericPeriodicBlock : public ParserBlock
{
public:
  void stupid();
  
  GenericPeriodicBlock(const std::string & name, InputParameters params);
  
  virtual void execute();

  void setPeriodicVars(PeriodicBoundary & p, const std::vector<std::string> & var_names);
  
protected:
  std::string _type;
};

template<>
InputParameters validParams<GenericPeriodicBlock>();


#endif //GENERICPERIODICBLOCK_H_
