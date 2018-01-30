#!/usr/bin/python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function

import os, re, time, sys

try:
    import hit
except:
    print('failed to import hit - try running "make hit" in the $MOOSE_DIR/test directory.', file=sys.stderr)
    sys.exit(1)

class DupWalker(object):
    def __init__(self, fname):
        self.have = {}
        self.dups = {}
        self.errors = []
        self._fname = fname

    def _duperr(self, node):
        if node.type() == hit.NodeType.Section:
            ntype = 'section'
        elif node.type() == hit.NodeType.Field:
            ntype = 'parameter'
        self.errors.append('{}:{}: duplicate {} "{}"'.format(self._fname, node.line(), ntype, node.fullpath()))

    def walk(self, fullpath, path, node):
        if node.type() != hit.NodeType.Field and node.type() != hit.NodeType.Section:
            return

        if fullpath in self.have:
            if fullpath not in self.dups:
                self._duperr(self.have[fullpath])
                self.dups[fullpath] = True
            self._duperr(node)
        else:
            self.have[fullpath] = node


"""
Parser object for reading GetPot formatted files
"""
class Parser:
    def __init__(self, factory, warehouse, check_for_type=True):
        self.factory = factory
        self.warehouse = warehouse
        self.params_parsed = set()
        self._check_for_type = check_for_type
        self.root = None
        self.errors = []
        self.fname = ''

    def parse(self, filename):
        with open(filename, 'r') as f:
            data = f.read()

        self.fname = os.path.abspath(filename)

        try:
            root = hit.parse(os.path.abspath(filename), data)
        except Exception as err:
            self.errors.append('{}'.format(err))
            return
        self.root = root

        w = DupWalker(os.path.abspath(filename))
        root.walk(w, hit.NodeType.All)
        self.errors.extend(w.errors)

        self._parseNode(filename, root)

    def error(self, msg, node=None):
        if node:
            self.errors.append('{}:{}: {}'.format(self.fname, node.line(), msg))
        else:
            self.errors.append('{}: {}'.format(self.fname, msg))

    def extractParams(self, filename, params, getpot_node):
        have_err = False

        # Populate all of the parameters of this test node
        # using the GetPotParser.  We'll loop over the parsed node
        # so that we can keep track of ignored parameters as well
        local_parsed = set()
        for child in getpot_node.children(node_type=hit.NodeType.Field):
            key = child.path()
            value = child.raw()

            self.params_parsed.add(child.fullpath())
            local_parsed.add(key)
            if key in params:
                if params.type(key) == list:
                    value = value.replace('\n', ' ')
                    params[key] = re.split('\s+', value)
                else:
                    if re.match('".*"', value):   # Strip quotes
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
                                    self.error("invalid date value: '{}=\"{}\"'".format(child.fullpath(), value), node=child)
                                    have_err = True
                            elif strict_type != type(value):
                                self.error("wrong data type for parameter value: '{}=\"{}\"'".format(child.fullpath(), value), node=child)
                                have_err = True
                        elif child.kind() == hit.FieldKind.Bool:
                            # Otherwise, just do normal assignment
                            params[key] = child.param()
                        else:
                            params[key] = value
            else:
                self.error('unused parameter "{}"'.format(child.fullpath()), node=child)

        # Make sure that all required parameters are supplied
        required_params_missing = params.required_keys() - local_parsed
        if len(required_params_missing) > 0:
            self.error('required missing parameter(s): ' + ', '.join(required_params_missing))
            have_err = True

        params['have_errors'] = have_err

    # private:
    def _parseNode(self, filename, node):
        if node.find('type'):
            moose_type = node.param('type')

            # Get the valid Params for this type
            params = self.factory.validParams(moose_type)

            # Extract the parameters from the Getpot node
            self.extractParams(filename, params, node)

            # Add factory and warehouse as private params of the object
            params.addPrivateParam('_factory', self.factory)
            params.addPrivateParam('_warehouse', self.warehouse)
            params.addPrivateParam('_parser', self)
            params.addPrivateParam('_root', self.root)

            # Build the object
            try:
                moose_object = self.factory.create(moose_type, node.path(), params)

                # Put it in the warehouse
                self.warehouse.addObject(moose_object)
            except Exception as e:
                self.error('failed to create Tester: {}'.format(e))

        # Are we in a tree node that "looks" like it should contain a buildable object?
        elif node.parent().fullpath() == 'Tests' and self._check_for_type and not self._looksLikeValidSubBlock(node):
            self.error('missing "type" parameter in block "{}"'.format(node.fullpath()), node=node)

        # Loop over the section names and parse them
        for child in node.children(node_type=hit.NodeType.Section):
            self._parseNode(filename, child)

    # This routine returns a Boolean indicating whether a given block
    # looks like a valid subblock. In the Testing system, a valid subblock
    # has a "type" and no children blocks.
    def _looksLikeValidSubBlock(self, node):
        fields = [n.path() for n in node.children()]
        return 'type' in fields and len(node.children(node_type=hit.NodeType.Section)) == 0
