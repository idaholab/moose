/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef EXTRAQPPROVIDER_H
#define EXTRAQPPROVIDER_H

class ExtraQPProvider;

template <>
InputParameters validParams<ExtraQPProvider>();

/**
 * Provide a list of extra QPs to be evaluated using the XFEMMMaterialManager
 */
class ExtraQPProvider
{
public:
  virtual const std::map<dof_id_type, std::vector<Point>> & getExtraQPMap() const = 0;
};

#endif // EXTRAQPPROVIDER_H
