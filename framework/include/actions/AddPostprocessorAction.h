#ifndef ADDPOSTPROCESSORACTION_H_
#define ADDPOSTPROCESSORACTION_H_

#include "MooseObjectAction.h"

class AddPostprocessorAction: public MooseObjectAction
{
public:
  AddPostprocessorAction(const std::string & name, InputParameters params);

  virtual void act();

protected:
  Moose::PostprocessorType _pps_type;
};

template<>
InputParameters validParams<AddPostprocessorAction>();  

#endif //ADDPOSTPROCESSORACTION_H_
