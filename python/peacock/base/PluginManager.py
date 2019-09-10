#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
from PyQt5 import QtWidgets
import mooseutils
from .MooseWidget import MooseWidget
from .Plugin import Plugin
from .PreferenceWidget import PreferenceWidget

class PluginManager(MooseWidget):
    """
    A MooseWidget for managing plugins.

    A manager creates plugins and places then in a layout automatically. It also
    connects signals and slots automatically between the plugins. If a signal is named 'mySignal'
    it will automatically connect to a slot named 'onMySignal'. A plugin will not connect to itself, to help minimize cyclic dependencies.

    A plugin is added to a layout by name, as specified by the Plugin object (see Plugin.py). Thus,
    when creating child PluginManager the layout member names (e.g., 'MainLayout' or 'LeftLayout')
    must be specified in the Plugin object. When a plugin is added, it is also added as a member variable according to the class name, if multiple plugins are added of the same type the member variable will be a list of the plugins rather than the plugin itself.

    This is a MooseWidget, thus the setup() method must be called by derived classes.

    Args:
        plugins[list]: A list of Plugin object classes (not instances) to be managed by this object.

    @see Plugin.py
    """

    def __init__(self, plugins=[], plugin_base=Plugin):
        super(PluginManager, self).__init__()

        # Check that self is a QWidget
        if not isinstance(self, QtWidgets.QWidget):
            mooseutils.MooseException("{} objects must also be a QWidget.".format(self.__class__.__name__))

        # A list of plugin classes (see setup()), this is private because child classes
        # shouldn't be messing with the classes.
        self._plugin_classes = plugins

        # The base class that this manager is allowed to manage
        self._plugin_base = plugin_base

        # An OrderedDict (to maintain the order plugins are added) for storing plugin objects
        self._plugins = collections.OrderedDict()

        # The tab index for this plugin
        self._index = None

        # List of all plugins for connecting signal
        self._all_plugins = []
        self._pref_widget = None

    def __contains__(self, item):
        """
        Provide "in" access into the list of plugins.
        """
        return item in self._plugins

    def __getitem__(self, item):
        """
        Provide operator[] access to plugins.
        """
        return self._plugins[item]

    def addObject(self, widget):
        """
        Method for adding a widget to a layout.

        Args:
            widget[QWidget]: The widget to add.

        NOTE: This method exists so that derived classes can customize how items are added.
        """
        if not hasattr(self, widget.mainLayoutName()):
            mooseutils.mooseError("Unknown layout name '{}' returned when adding plugin '{}', the plugin is being skipped.".format(widget.mainLayoutName(), widget.__class__.__name__))
        else:
            layout = getattr(self, widget.mainLayoutName())
            layout.addWidget(widget)

    def setTabIndex(self, index, signal=None):
        """
        Set the Peacock tab index.
        """
        self._index = index
        for plugin in self._all_plugins:
            plugin.setTabIndex(index, signal=signal)

    def setup(self):
        """
        Call widget setup methods and connect signals and slots from plugins.
        """
        super(PluginManager, self).setup()

        # Create the plugins
        for plugin_class in self._plugin_classes:

            # Create the widget instance
            widget = plugin_class()

            # Check the type
            if not isinstance(widget, self._plugin_base):
                mooseutils.MooseException("The supplied widget is of type '{}' but must be a direct child of a '{}'".format(widget.__class__.__name__, self._plugin_base.__name__))

            # Define the widget name
            name = widget.__class__.__name__

            # Store widget in a list if more than one exist
            if name in self._plugins:
                if not isinstance(self._plugins[name], list):
                    self._plugins[name] = [self._plugins[name]]
                self._plugins[name].append(widget)

            # Store widget directly if only one
            else:
                self._plugins[name] = widget

            # Set the parent
            widget.setParent(self)

            # Add the widget
            self.addObject(widget)

            # Set the class attribute base on plugin name
            setattr(self, name, self._plugins[name])
            mooseutils.mooseDebug('Adding plugin as member: {}'.format(name))

            # Store in the temporary flat list
            self._all_plugins.append(widget)

        # Connect signal/slots of plugins
        for plugin0 in self._all_plugins:
            plugin0._plugin_manager = self
            for plugin1 in self._all_plugins:
                plugin0.connect(plugin1)

    def write(self, filename):
        """
        Write the python script.
        """
        with open(filename, 'w') as fid:
            string = '"""\npython {}\n"""\n'.format(filename)
            string += self.repr()
            fid.write(string)

    def repr(self):
        """
        Return a string containing a script to reproduce the plugins.
        """
        return ''

    def call(self, method, *args, **kwargs):
        """
        If a method is present on the plugin call it with the given arguments.

        see ExodusViewer.onJobStart/onInputFileChanged
        """
        for plugin in self._all_plugins:
            if hasattr(plugin, method):
                attr = getattr(plugin, method)
                attr(*args, **kwargs)

    def preferencesWidget(self):
        """
        Returns an instance of a widget to set preferences for all the plugins.
        """
        if not self._pref_widget:
            self._pref_widget = PreferenceWidget(self._all_plugins)
        return self._pref_widget

    def onPreferencesSaved(self):
        """
        This will be called when the preferences have been saved.
        """
        for p in self._all_plugins:
            p.onPreferencesSaved()

    def fixLayoutWidth(self, layout):
        # Set the width of the left-side widgets to that the VTK window gets the space
        width = 0
        for child in self._plugins.values():
            if child.mainLayoutName() == layout:
                width = max(child.sizeHint().width(), width)
        for child in self._plugins.values():
            if child.mainLayoutName() == layout:
                child.setFixedWidth(width)
