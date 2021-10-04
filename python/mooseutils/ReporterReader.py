#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import glob
import json

from . import message

class ReporterReader(object):
    """
    A Reader for MOOSE Reporter data.

    Args:

    - file\[str\]: JSON file containing reporter data, can be blob pattern if there is one file per timestep.

    MOOSE outputs Reporter data in two different ways: Every timestep in a single file or
    separate files for each timestep, using the timestep as
    a prefix. For example: file_000.json, file_001.json, etc.

    Therefore, a pattern acceptable for use with the python glob package may be supplied. For the
    above files, "file_*.json" should be supplied.

    This object manages the loading and unloading of data and should always be in a valid state,
    regardless of the existence of a file. It will also append new data and remove old/deleted data
    on subsequent calls to "update()".

    ReporterReader uses tuples as keys for indexing values. Here is an example usage:

    ```python
    data = mooseutils.ReporterReader('file.json')
    if ('object_name', 'value_name') in data:
        print('value_name in object object_name was found!')
        value = data[('object_name', 'value_name')]
    else:
        print('value_name in object object_name was NOT found!')
    ```
    """
    def __init__(self, file):
        self._filename = file
        self._data = dict(time_steps=[])
        self._time = -1
        self._index = 0
        self.update()

    @property
    def data(self):
        """
        Get the raw data from reading the JSON file(s)
        """
        return self._data

    @property
    def filename(self):
        """
        The file name inputted when constructing the object
        """
        return self._filename

    def partFilename(self, part):
        """
        Gets the filename assocated with the given part

        Args:

        - part\[int\]: The part
        """
        if part >= self.numParts():
            message.mooseError('Cannot load part {}; only {} parts exist'.format(part, self.numParts()))
        if part == 0:
            return self._filename
        return self._filename + '.' + '{}'.format(part).zfill(len(str(self.numParts())))

    def __getitem__(self, keys):
        """
        Operator[] returns the data for the current time.

        Args:

        - keys\[tuple(str, str)|list\]: The reporter value(s) (object name, value name) to return.
        """
        if isinstance(keys, list):
            vals = []
            for key in keys:
                obj, val = key
                vals.append(self._data['time_steps'][self._index][obj][val])
            return vals
        else:
            obj, val = keys
            return self._data['time_steps'][self._index][obj][val]

    def __bool__(self):
        """
        Allows this object to be used in boolean cases.

        ```python
            data = ReporterReader('file.json')
            if not data:
                print 'No data found!'
        ```
        """
        return self._index < len(self._data['time_steps'])

    def __contains__(self, variable):
        """
        Returns true if the variable exists in the data structure.

        Args:

        - variable\[tuple(str, str)\]: The reporter name (object name, value name)

        ```python
            data = ReporterReader('file.json')
            if ('obj_name', 'value_name') in data:
                print('Reporter value found!')
        ```
        """
        obj, val = variable
        if 'reporters' in self._data:
            if obj in self._data['reporters']:
                if val in self._data['reporters'][obj]['values']:
                    return True
        return False

    def times(self):
        """
        Returns the list of available time indices contained in the data.
        """
        return [ts['time'] for ts in self._data['time_steps']]

    def numParts(self):
        """
        Returns the number of parts available
        """
        return self._data.get('number_of_parts', 1)

    def clear(self):
        """
        Remove all data.
        """
        self._data = dict(time_steps=[])
        self._index = 0
        self._time = -1

    def variables(self):
        """
        Return a list of reporter names as tuples listed in the reader.

        ```python
            data = ReporterReader('file.json')
            for var in data.variables():
                print('Found value with object name "{}" and value name "{}"'.format(var[0], var[1]))
        ```
        """
        rnames = []
        if 'reporters' in self._data:
            for obj in self._data['reporters']:
                for val in self._data['reporters'][obj]['values']:
                    rnames.append((obj, val))
        return rnames

    def info(self, key=None):
        """
        Return a information within json file.

        Args:

        - key\[None|str|tuple(str, str)\]: The information to return. Can be:

          - None for system information
          - object name for reporter object information
          - (object name, value name) for reporter value information

        ```python
            data = ReporterReader('file.json')
            print('Data created at ' + data.info()['executable_time'])
            print(obj_name + ' is a ' + data.info(obj_name)['type'] + ' object.')
            print(val_name + ' from ' + obj_name + ' is of C++ type ' + data.info((obj_name, val_name))['type'])
        ```
        """

        info = dict()
        if key is None:
            for k in self._data:
                if k != 'time_steps' and k != 'reporters':
                    info[k] = self._data[k]

        elif isinstance(key, str):
            for k in self._data['reporters'][key]:
                if k != 'values':
                    info[k] = self._data['reporters'][key][k]

        elif isinstance(key, tuple):
            obj, val = key
            info = self._data['reporters'][obj]['values'][val]

        return info

    def update(self, time=None, part=0):
        """
        Update data by reading/re-reading files.

        Args:

        - time\[float\]: The time at which the data should be returned.
        - part\[int\]: The part of which the data should be returned.

        ```python
            data = ReporterReader('file.json')
            df = dict(time=[], object_name=[], value_name=[], value=[])
            for time in data.times():
                data.update(time)
                for var in data.variables():
                    df['time'].append(time)
                    df['object_name'].append(var[0])
                    df['value_name'].append(var[1])
                    df['value'].append(data[var])

        ```
        """

        filename = self.partFilename(part)
        if '*' in filename:
            filenames = glob.glob(filename)
            if not len(filenames):
                self.clear()
                message.mooseDebug("Could not find any json files with pattern {}.".format(filename))

            for fname in sorted(filenames):
                with open(fname, 'r', encoding='utf-8') as fid:
                    tmp = json.load(fid)
                    for key in tmp:
                        if key != 'time_steps':
                            self._data[key] = tmp[key]
                    self._data['time_steps'].extend(tmp['time_steps'])

        elif not os.path.exists(filename):
            self.clear()
            message.mooseDebug("Could not json file {}.".format(filename))

        else:
            with open(filename, 'r', encoding='utf-8') as fid:
                self._data = json.load(fid)

        self._data['time_steps'].sort(key=lambda ts : ts['time'])

        # Update the time
        if time is not None:
            self._time = time
        else:
            self._time = self.times()[-1]
        self._index = self.times().index(self._time)

    def repr(self):
        """
        Return components for building script.

        Returns:

        - (output, imports) The necessary script and include statements to re-create data load.
        """

        imports = ['import mooseutils']
        output = ['\n# Read Reporter Data']
        output += ['data = mooseutils.ReporterReader({})'.format(repr(self._filename))]
        return output, imports
