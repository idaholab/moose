#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from scipy.optimize import curve_fit
import numpy as np

def linearFit(T, k):
    """
    Linear least-squares fit to data
    """
    assert len(T) > 1, 'Input lists must have at least two elements'
    assert len(T) == len(k), 'Input lists must be equal length'

    def func(T, a0, a1):
        return a0 + a1 * T

    popt, pcov = curve_fit(func, T, k)

    def outputFunc(temperatures):
        return func(np.asarray(temperatures), *popt)

    return outputFunc

def fourthOrderFit(T, k):
    """
    Fourth-order polynomial least-squares fit to data
    """
    assert len(T) > 4, 'Input lists must have at least five elements'
    assert len(T) == len(k), 'Input lists must be equal length'

    def func(T, a0, a1, a2, a3, a4):
        return a0 + a1 * T + a2 * T**2 + a3 * T**3 + a4 * T**4

    popt, pcov = curve_fit(func, T, k)

    def outputFunc(temperatures):
        return func(np.asarray(temperatures), *popt)

    return outputFunc

def maierKellyFit(T, k):
    """
    Maier-Kelly least-squares fit to data
    """
    assert len(T) > 4, 'Input lists must have at least five elements'
    assert len(T) == len(k), 'Input lists must be equal length'
    assert T[0] > 0, 'Temperature cannot be zero'

    def func(T, a0, a1, a2, a3, a4):
        return a0 * np.log(T) + a1 + a2 * T + a3 / T + a4 / T**2

    popt, pcov = curve_fit(func, T, k)

    def outputFunc(temperatures):
        return func(np.asarray(temperatures), *popt)

    return outputFunc

def fillMissingValues(temperatures, values, fit_type, missing_value):
    """
    Generate values to replace missing values in the database, primarily
    for equilibrium constants defined at temperature points.

    If a data point is missing (default value of 500.00000), then use the
    functional form for the database to replace missing values with fitted
    values.

    An important consideration is where too few values are present to allow
    a fit to the given function (for example, the fourth-order polynomial used
    in the GWB database requires five data points). In these cases, a linear fit
    is used if there are at least two values in the array. If only one value is
    defined, then it is copied to all remaining temperature points.

    For species where missing values have been filled in using a fit, a note is
    added as a seperate field named 'note'.

    For example, consider the species Fe(OH)3 in the original GWB database
    *****
    Fe(OH)3
         charge=  0      ion size=  4.0 A      mole wt.=  106.8689 g
         3 species in reaction
           -3.000 H+              1.000 Fe+++           3.000 H2O
            13.7601     12.0180      9.9003      7.8005
             5.4494      3.1992    500.0000    500.0000
    *****

    In this case, the two missing values are filled using a fourth-order polynomial
    fit (fit used in GWB database), so that the species data in the MOOSE JSON
    database is
    *****
    "Fe(OH)3": {
      "species": {
        "H+": -3.0,
        "Fe+++": 1.0,
        "H2O": 3.0
      },
      "charge": 0.0,
      "radius": 4.0,
      "molecular weight": 106.8689,
      "logk": [
        13.7601,
        12.018,
        9.9003,
        7.8005,
        5.4494,
        3.1992,
        0.9064,
        -1.4986
      ],
      "note": "Missing array values in original database have been filled using a fourth-order fit. Original values are [13.7601, 12.0180, 9.9003, 7.8005, 5.4494, 3.1992, 500.0000, 500.0000]"
    },
    *****
    """

    # Remove missing_value from values
    vals = []
    temp = []
    note = ""
    filled_values = values
    dplaces = 4

    if missing_value in values:

        # Number of valid values
        numvals = len(values) - values.count(missing_value)

        if numvals == 1:
            fit_type = "constant"

        elif numvals > 1 and numvals < 5:
            fit_type = "linear"

        else:
            fit_type = fit_type

        # Add note with original data
        strlist = ", ".join([str(item) for item in values])
        note = "Missing array values in original database have been filled using a " + fit_type + " fit. Original values are [" + strlist + "]"

        for i in range(len(temperatures)):
            if values[i] != missing_value:
                temp.append(float(temperatures[i]))
                vals.append(float(values[i]))

        # If there is only one value, fill the filled values array with that value
        if fit_type == 'constant':
            for i in range(len(temperatures)):
                if filled_values[i] == missing_value:
                    filled_values[i] = vals[0]

        # If there is between two and four values, use a linear fit to fill array
        elif fit_type == 'linear':
            fit = linearFit(temp, vals)

            for i in range(len(temperatures)):
                if filled_values[i] == missing_value:
                    filled_values[i] = round(fit(temperatures[i]), dplaces)

        else:
            if fit_type == 'fourth-order':
                fit = fourthOrderFit(temp, vals)

            elif fit_type == 'maier-kelly':
                fit = maierKellyFit(temp, vals)

            else:
                raise ValueError("fit_type " + fit_type + " not supported")

            for i in range(len(temperatures)):
                if filled_values[i] == missing_value:
                    filled_values[i] = round(fit(temperatures[i]), dplaces)

    return [float(val) for val in filled_values], note
