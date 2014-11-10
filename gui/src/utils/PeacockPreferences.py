import os, pickle

class PeacockPreferences(object):
  def __init__(self):
    self._filename =  os.path.join(os.getenv('HOME'), '.peacock')

    if os.path.exists(self._filename):
      fid = open(self._filename, 'r')
      self._prefs = pickle.load(fid)
      print self._prefs
      fid.close()
    else:
      self._prefs = dict()


  def declare(self, name, default_value):
    if name not in self._prefs:
      self._prefs[name] = default_value
      self._write()

    return self._prefs[name]

  def set(self, name, value):
    if name not in self._prefs:
      print 'Error: Invalid preference', name, ', the preference must be declared'
      return

    self._prefs[name] = value
    self._write()


  def get(self, name):
    return self._prefs[name]

  def __getitem__(self, key):
    return self.get(key)

  def __setitem__(self, key, value):
    self.set(key, value)

  def _write(self):
    fid = open(self._filename, 'w')
    pickle.dump(self._prefs, fid)
    fid.close()
