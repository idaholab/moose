#ifndef GLOBALPARAMSBLOCK_H_
#define GLOBALPARAMSBLOCK_H_

#include "ParserBlock.h"
#include "Moose.h"

class GlobalParamsBlock;

template<>
InputParameters validParams<GlobalParamsBlock>();

class GlobalParamsBlock: public ParserBlock
{
public:
  GlobalParamsBlock(const std::string & name, InputParameters params);

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
