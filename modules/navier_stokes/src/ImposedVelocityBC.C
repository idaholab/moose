#include "ImposedVelocityBC.h"

// Full specialization of the validParams function for this object
template<>
InputParameters validParams<ImposedVelocityBC>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NodalBC>();

  // Declare some coupled variables...
  params.addRequiredCoupledVar("p", "");
  params.addRequiredCoupledVar("pu", "");
  params.addRequiredCoupledVar("pv", "");
  params.addCoupledVar("pw", "");
  
  // And add some of our own parameters...
  // params.set<unsigned>("component"); // raw libmesh syntax
  // params.set<Real>("desired_velocity"); // raw libmesh syntax

  // New style.  Note, addRequiredParam is a templated function, templated
  // on the type of the parameter being added. (Second string argument is optional documentation.)
  params.addRequiredParam<unsigned>("component", "");// make it required!
  params.addRequiredParam<Real>("desired_velocity", "");// make it required!
  
  
  return params;
}




// Constructor, be sure to call the base class constructor!
ImposedVelocityBC::ImposedVelocityBC(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters),
   _p(coupledValue("p")),
   _pu(coupledValue("pu")),
   _pv(coupledValue("pv")),
   _pw(_dim == 3 ? coupledValue("pw") : _zero),
   _component(getParam<unsigned>("component")),
   _desired_velocity(getParam<Real>("desired_velocity"))
  {
    // _component = 0,1,2 corresponds to whether we are setting x, y, or z Dirichlet values, respectively.
    // Should be set in the input file.
    if (_component > 2)
      {
	std::cout << "Must select a component<2 for ImposedVelocityBC" << std::endl;
	libmesh_error();
      }
  }



// Specialization of the computeQpResidual function for this class
Real ImposedVelocityBC::computeQpResidual()
{
  // The momentum vector.  Could also do this with an if-test over _component's.
  RealVectorValue momentum(_pu[_qp], _pv[_qp], _pw[_qp]);
  
  // Return the difference between the current momentum and the desired value
  return momentum(_component) - (_p[_qp] * _desired_velocity);
}

