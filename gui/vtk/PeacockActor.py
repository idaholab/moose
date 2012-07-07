from vtk.util.colors import peacock, tomato, red, white, black

class PeacockActor:
  def __init__(self, renderer):
    self.renderer = renderer
    
    self.visible = False
    self.edges_visible = False
    self.solid = True
    self.color = white
    
  ''' Sync view options.  The view options for self will match the passed in object '''
  def sync(self, other_actor):
    if other_actor.visible:
      self.show()
    else:
      self.hide()

    if other_actor.edges_visible:
      self.showEdges()
    else:
      self.hideEdges()

    if other_actor.solid:
      self.goSolid()
    else:
      self.goWireframe()

    self.setColor(other_actor.color)
    
  def getBounds(self):
    raise NotImplementedError    

  def show(self):
    self.visible = True
    self._show()

  def hide(self):
    self.visible = False
    self._hide()

  def showEdges(self):
    self.edges_visible = True
    self._showEdges()
    
  def hideEdges(self):
    self.edges_visible = False
    self._hideEdges()

  def goSolid(self):
    self.solid = True
    self._goSolid()
    
  def goWireframe(self):
    self.solid = False
    self._goWireframe()

  def setColor(self, color):
    self.color = color
    self._setColor(color)

  def _show(self):
    raise NotImplementedError
  
  def _hide(self):
    raise NotImplementedError

  def _showEdges(self):
    raise NotImplementedError
    
  def _hideEdges(self):
    raise NotImplementedError

  def _goSolid(self):
    raise NotImplementedError
    
  def _goWireframe(self):
    raise NotImplementedError

  def _setColor(self, color):
    raise NotImplementedError
