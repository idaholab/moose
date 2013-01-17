#ifndef NODALAREA_H
#define NODALAREA_H

#include "SideIntegralVariableUserObject.h"

class NodalArea : public SideIntegralVariableUserObject
{
public:
  NodalArea(const std::string & name, InputParameters parameters);
  virtual ~NodalArea();

  virtual void threadJoin(const UserObject & uo);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void destroy() {}

  Real nodalArea( unsigned id ) const;
  void resetCommunication()
  {
    _resetCommunication = true;
  }

protected:
  virtual Real computeQpIntegral();

  void initializeCommunication();
  void communicate();

  std::map<unsigned, Real> _node_areas;

  std::map<unsigned, unsigned> _commMap;
  std::vector<Real> _commVec;
  bool _resetCommunication;

  const VariablePhiValue & _phi;
};

template<>
InputParameters validParams<NodalArea>();

#endif
