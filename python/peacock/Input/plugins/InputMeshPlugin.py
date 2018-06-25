#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from peacock.ExodusViewer.plugins.MeshPlugin import MeshPlugin

class InputMeshPlugin(MeshPlugin):
    """
    A MeshPlugin setup to work with the --mesh-only output.
    """

    def __init__(self, **kwargs):
        super(InputMeshPlugin, self).__init__(**kwargs)
        self.setMainLayoutName('RightLayout')

    def _setupRepresentation(self, qobject):
        super(InputMeshPlugin, self)._setupRepresentation(qobject)
        qobject.setCurrentIndex(1)
