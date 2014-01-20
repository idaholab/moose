#ifndef CRACKFRONTDEFINITION_H
#define CRACKFRONTDEFINITION_H

#include "NodalUserObject.h"
#include <set>

class CrackFrontDefinition;
class AuxiliarySystem;

template<>
InputParameters validParams<CrackFrontDefinition>();

/**
 * Works on top of NodalNormalsPreprocessor
 */
class CrackFrontDefinition : public NodalUserObject
{
public:
  CrackFrontDefinition(const std::string & name, InputParameters parameters);
  virtual ~CrackFrontDefinition();

  virtual void initialize();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

  void orderCrackFrontNodes();
  void orderEndNodes(std::vector<unsigned int> &end_nodes);

protected:
  AuxiliarySystem & _aux;

  std::set<unsigned int> _nodes;
  std::vector<unsigned int> _ordered_crack_front_nodes;
};


#endif /* CRACKFRONTDEFINITION_H */
