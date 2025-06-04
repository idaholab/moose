#!/usr/bin/python
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, re, time
import moosetree
import pyhit

"""
Parser object for reading HIT formatted files
"""
class Parser:

    @staticmethod
    def checkDuplicates(root):
        """Check for duplicate blocks and/or parameters"""
        paths = set()

        for node in moosetree.iterate(root, method=moosetree.IterMethod.PRE_ORDER):
            if node.fullpath in paths:
                yield ('duplicate section "{}"'.format(node.fullpath), node)
            else:
                paths.add(node.fullpath)

            for key, _ in node.params():
                fullparam = os.path.join(node.fullpath, key)
                if fullparam in paths:
                    yield ('duplicate parameter "{}"'.format(fullparam), node, key)
                else:
                    paths.add(fullparam)

    def __init__(self, factory, warehouse, check_for_type=True):
        self.factory = factory
        self.warehouse = warehouse
        self._check_for_type = check_for_type
        self.root = None
        self.errors = []
        self.fname = ''

    def parse(self, filename, default_values = None):

        self.fname = os.path.abspath(filename)

        try:
            root = pyhit.load(self.fname)
        except Exception as err:
            self.error(err)
            return
        self.root = root(0) # make the [Tests] block the root

        self.check()
        self._parseNode(filename, self.root, default_values)

    def check(self):
        """Perform error checking on the loaded hit tree"""
        for err in Parser.checkDuplicates(self.root):
            self.error(*err)

    def error(self, msg, node=None, param=None):
        if node is not None:
            self.errors.append('{}:{}: {}'.format(self.fname, node.line(param), msg))
        else:
            self.errors.append('{}: {}'.format(self.fname, msg))

    def extractParams(self, params, getpot_node):
        if not params.isValid('_have_parse_errors'):
            params.addPrivateParam('_have_parse_errors', False)
        if not params.isValid('_params_parsed'):
            params.addPrivateParam('_params_parsed', set())
        params_parsed = params['_params_parsed']

        # Populate all of the parameters of this test node
        # using the GetPotParser.  We'll loop over the parsed node
        # so that we can keep track of ignored parameters as well
        have_err = False
        for key, value in getpot_node.params():
            param_path = os.path.join(getpot_node.fullpath, key)
            if param_path in params_parsed:
                continue
            params_parsed.add(key)
            if key not in params:
                continue

            if params.type(key) == list:
                if isinstance(value, str):
                    value = value.replace('\n', ' ')
                    params[key] = re.split(r'\s+', value)
                else:
                    params[key] = [str(value)]
            else:
                if isinstance(value, str) and re.match('".*"', value):   # Strip quotes
                    params[key] = value[1:-1]
                else:
                    if key in params.strict_types:
                        # The developer wants to enforce a specific type without setting a valid value
                        strict_type = params.strict_types[key]

                        if strict_type == time.struct_time:
                            # Dates have to be parsed
                            try:
                                params[key] = time.strptime(value, "%m/%d/%Y")
                            except ValueError:
                                self.error("invalid date value: '{}=\"{}\"'".format(child.fullpath, value), node=child)
                                have_err = True
                        elif strict_type != type(value):
                            self.error("wrong data type for parameter value: '{}=\"{}\"'".format(child.fullpath, value), node=child)
                            have_err = True
                    else:
                        # Otherwise, just do normal assignment
                        params[key] = value

        if have_err:
            params['_have_parse_errors'] = True

    def checkParams(self, params, node):
        # Check for unused parameters
        for key, _ in node.params():
            if key not in params:
                self.error(f'unused parameter "{key}"', node=node, param=key)

        # Check for missing required parameters
        params_parsed = params['_params_parsed']
        missing = params.required_keys() - params_parsed
        if len(missing) > 0:
            self.error('required missing parameter(s): ' + ', '.join(missing))
            params['_have_parse_errors'] = True

    # private:
    def _parseNode(self, filename, node, default_values):

        if 'type' in node:
            moose_type = node['type']

            # Get the valid Params for this type
            params = self.factory.validParams(moose_type)

            # Record full path of node
            params.addParam('hit_path', node.fullpath, 'HIT path to test in spec file')

            # Apply any new defaults
            for key, value in default_values.params():
                if key in params.keys():
                    if key == 'cli_args':
                      params[key].append(value)
                    else:
                      params[key] = value

            # Extract the parameters from the Getpot node
            self.extractParams(params, node)

            # Add factory and warehouse as private params of the object
            params.addPrivateParam('_factory', self.factory)
            params.addPrivateParam('_warehouse', self.warehouse)
            params.addPrivateParam('_parser', self)
            params.addPrivateParam('_root', self.root)

            # Build the object
            try:
                params = self.factory.augmentParams(moose_type, params)
                self.extractParams(params, node)
                self.checkParams(params, node)
                moose_object = self.factory.create(moose_type, node.name, params)

                # Put it in the warehouse
                self.warehouse.addObject(moose_object)
            except Exception as e:
                e_split = str(e).splitlines()
                messages = [f'Failed to create Tester: {e_split[0]}']
                if len(e_split) > 1:
                    messages += [f'  {v}' for v in e_split[1:]]
                for message in messages:
                    self.error(message)

        # Are we in a tree node that "looks" like it should contain a buildable object?
        elif node.parent.fullpath == 'Tests' and self._check_for_type and not self._looksLikeValidSubBlock(node):
            self.error('missing "type" parameter in block "{}"'.format(node.fullpath), node=node)

        # Loop over the section names and parse them
        for child in node:
            self._parseNode(filename, child, default_values)

    # This routine returns a Boolean indicating whether a given block
    # looks like a valid subblock. In the Testing system, a valid subblock
    # has a "type" and no children blocks.
    def _looksLikeValidSubBlock(self, node):
        return 'type' in node and len(node) == 0
