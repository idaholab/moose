#ifndef GLOBALPARAMSACTION_H_
#define GLOBALPARAMSACTION_H_

#include "Action.h"
#include "Moose.h"

class GlobalParamsAction: public Action
{
public:
  GlobalParamsAction(const std::string & name, InputParameters params);

  virtual void act();

  template <typename T>
  inline
  T & setScalarParam(const std::string &name)
  {
    return parameters().set<T>(name);
  }

  template <typename T>
  inline
  std::vector<T> & setVectorParam(const std::string &name)
  {
    return parameters().set<std::vector<T> >(name);
  }

  template <typename T>
  inline
  std::vector<std::vector<T> > & setTensorParam(const std::string &name)
  {
    return parameters().set<std::vector<std::vector<T> > >(name);
  }
};

template<>
InputParameters validParams<GlobalParamsAction>();


#endif //GLOBALPARAMSACTION_H
