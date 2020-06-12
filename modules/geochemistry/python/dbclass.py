#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class ThermoDB(object):
    """
    Class containing data objects for a thermodynamic database
    for geochemical modelling
    """

    def __init__(self):
        self._format = None
        self._filename = None
        self._header = None
        self._activity_model = None
        self._fugacity_model = None
        self._logk_model = None
        self._logk_model_eqn = None
        self._temperatures = None
        self._pressures = None
        self._elements = None
        self._basis_species = None
        self._secondary_species = None
        self._free_electron = None
        self._mineral_species = None
        self._gas_species = None
        self._redox_couples = None
        self._adh = None
        self._bdh = None
        self._bdot = None
        self._neutral_species = None
        self._sorbing_minerals = None
        self._surface_species = None

    # Original DB format
    @property
    def format(self):
        return self._format

    @format.setter
    def format(self, format):
        self._format = format

    # Original DB header information
    @property
    def header(self):
        return self._header

    @header.setter
    def header(self, header):
        self._header = header

    # Original DB activity model
    @property
    def activity_model(self):
        return self._activity_model

    @activity_model.setter
    def activity_model(self, activity_model):
        self._activity_model = activity_model

    # Original DB fugacity model
    @property
    def fugacity_model(self):
        return self._fugacity_model

    @fugacity_model.setter
    def fugacity_model(self, fugacity_model):
        self._fugacity_model = fugacity_model

    # Original DB equilibrium constant model
    @property
    def logk_model(self):
        return self._logk_model

    @logk_model.setter
    def logk_model(self, logk_model):
        self._logk_model = logk_model

    # Original DB equilibrium constant model in equation form
    @property
    def logk_model_eqn(self):
        return self._logk_model_eqn

    @logk_model_eqn.setter
    def logk_model_eqn(self, logk_model_eqn):
        self._logk_model_eqn = logk_model_eqn

    # Original DB temperature points
    # (for reaction equilibrium constants)
    @property
    def temperatures(self):
        return self._temperatures

    @temperatures.setter
    def temperatures(self, temperatures):
        self._temperatures = temperatures

    # Original DB pressure points
    @property
    def pressures(self):
        return self._pressures

    @pressures.setter
    def pressures(self, pressures):
        self._pressures = pressures

    # Element data
    @property
    def elements(self):
        return self._elements

    @elements.setter
    def elements(self, elements):
        self._elements = elements

    # Basis species data
    @property
    def basis_species(self):
        return self._basis_species

    @basis_species.setter
    def basis_species(self, basis_species):
        self._basis_species = basis_species

    # Secondary species data
    @property
    def secondary_species(self):
        return self._secondary_species

    @secondary_species.setter
    def secondary_species(self, secondary_species):
        self._secondary_species = secondary_species

    # Free electron data
    @property
    def free_electron(self):
        return self._free_electron

    @free_electron.setter
    def free_electron(self, free_electron):
        self._free_electron = free_electron

    # Mineral species data
    @property
    def mineral_species(self):
        return self._mineral_species

    @mineral_species.setter
    def mineral_species(self, mineral_species):
        self._mineral_species = mineral_species

    # Gas species data
    @property
    def gas_species(self):
        return self._gas_species

    @gas_species.setter
    def gas_species(self, gas_species):
        self._gas_species = gas_species

    # Redox couples data
    @property
    def redox_couples(self):
        return self._redox_couples

    @redox_couples.setter
    def redox_couples(self, redox_couples):
        self._redox_couples = redox_couples

    # Debye-Huckel a data
    @property
    def adh(self):
        return self._adh

    @adh.setter
    def adh(self, adh):
        self._adh = adh

    # Debye-Huckel b data
    @property
    def bdh(self):
        return self._bdh

    @bdh.setter
    def bdh(self, bdh):
        self._bdh = bdh

    # Debye-Huckel bdot data
    @property
    def bdot(self):
        return self._bdot

    @bdot.setter
    def bdot(self, bdot):
        self._bdot = bdot

    # Neutral species data
    @property
    def neutral_species(self):
        return self._neutral_species

    @neutral_species.setter
    def neutral_species(self, neutral_species):
        self._neutral_species = neutral_species

    # Sorbing minerals
    @property
    def sorbing_minerals(self):
        return self._sorbing_minerals

    @sorbing_minerals.setter
    def sorbing_minerals(self, sorbing_minerals):
        self._sorbing_minerals = sorbing_minerals

    # surface species
    @property
    def surface_species(self):
        return self._surface_species

    @surface_species.setter
    def surface_species(self, surface_species):
        self._surface_species = surface_species
