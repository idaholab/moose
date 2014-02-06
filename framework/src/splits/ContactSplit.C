/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ContactSplit.h"
#include "InputParameters.h"
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
template<>
InputParameters validParams<ContactSplit>()
{
  InputParameters params = validParams<Split>();
  params.addParam<std::vector<std::string> >("contact_master", "Master surface list for included contacts");
  params.addParam<std::vector<std::string> >("contact_slave",  "Slave surface list for included contacts");
  params.addParam<std::vector<bool> >("contact_displaced", "List of indicators whether displaced mesh is used to define included contact");
  params.addParam<std::vector<std::string> >("uncontact_master", "Master surface list for excluded contacts");
  params.addParam<std::vector<std::string> >("uncontact_slave",  "Slave surface list for excluded contacts");
  params.addParam<std::vector<bool> >("uncontact_displaced", "List of indicators whether displaced mesh is used to define excluded contact");
  return params;
}

ContactSplit::ContactSplit (const std::string & name, InputParameters params) :
    Split(name, params),
    _contact_master(getParam<std::vector<std::string> >("contact_master")),
    _contact_slave(getParam<std::vector<std::string> >("contact_slave")),
    _contact_displaced(getParam<std::vector<bool> >("contact_displaced")),
    _uncontact_master(getParam<std::vector<std::string> >("uncontact_master")),
    _uncontact_slave(getParam<std::vector<std::string> >("uncontact_slave")),
    _uncontact_displaced(getParam<std::vector<bool> >("uncontact_displaced"))
{
  if (_contact_master.size() != _contact_slave.size()) {
    std::ostringstream err;
    err << "Master and slave contact lists must have matching sizes: " << _contact_master.size() << " != " << _contact_slave.size();
    mooseError(err.str());
  }
  if (_contact_displaced.size() && _contact_master.size() != _contact_displaced.size()) {
    std::ostringstream err;
    err << "Master and displaced contact lists must have matching sizes: " << _contact_master.size() << " != " << _contact_displaced.size();
    mooseError(err.str());
  }
  if (!_contact_displaced.size()) _contact_displaced.resize(_contact_master.size());

  if (_uncontact_master.size() != _uncontact_slave.size()) {
    std::ostringstream err;
    err << "Master and slave uncontact lists must have matching sizes: " << _uncontact_master.size() << " != " << _uncontact_slave.size();
    mooseError(err.str());
  }
  if (_uncontact_displaced.size() && _uncontact_master.size() != _uncontact_displaced.size()) {
    std::ostringstream err;
    err << "Master and displaced uncontact lists must have matching sizes: " << _uncontact_master.size() << " != " << _uncontact_displaced.size();
    mooseError(err.str());
  }
  if (!_uncontact_displaced.size()) _uncontact_displaced.resize(_uncontact_master.size());

}

void
ContactSplit::setup(const std::string& prefix)
{
  PetscErrorCode ierr;
  std::string    dmprefix = prefix+"dm_moose_", opt, val;

  // contacts options
  if (_contact_master.size()) {
    opt = dmprefix+"ncontacts";
    {
      std::ostringstream oval;
      oval << _contact_master.size();
      val  =  oval.str();
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    for (unsigned int j = 0;  j < _contact_master.size(); ++j) {
      std::ostringstream oopt;
      oopt << dmprefix << "contact_" << j;
      opt = oopt.str();
      val = _contact_master[j]+","+_contact_slave[j];
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      if (_contact_displaced[j]) {
  opt = opt + "_displaced";
  val = "yes";
  ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
      }
    }
  }
  // uncontacts options
  if (_uncontact_master.size()) {
    opt = dmprefix+"nuncontacts";
    {
      std::ostringstream oval;
      oval << _uncontact_master.size();
      val  =  oval.str();
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    for (unsigned int j = 0;  j < _uncontact_master.size(); ++j) {
      std::ostringstream oopt;
      oopt << dmprefix << "uncontact_" << j;
      opt = oopt.str();
      val = _uncontact_master[j]+","+_uncontact_slave[j];
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      if (_uncontact_displaced[j]) {
  opt = opt + "_displaced";
  val = "yes";
  ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
      }
    }
  }
  Split::setup(prefix);
}
#endif
