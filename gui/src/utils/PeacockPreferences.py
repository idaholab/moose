import os, pickle

##
# A class for storing and loading preferences
class PeacockPreferences(object):
  def __init__(self):
    self._filename =  os.path.join(os.getenv('HOME'), '.peacock')

    # Read the existing preferences
    if os.path.exists(self._filename):
      fid = open(self._filename, 'r')
      self._prefs = pickle.load(fid)
      fid.close()
    else:
      self._prefs = dict()

  ##
  # Declare a preference and default value
  def declare(self, name, default_value):
    if name not in self._prefs:
      self._prefs[name] = default_value
      self._write()

    return self._prefs[name]

  ##
  # Set preference to the given value via the [] opertor
  def __setitem__(self, key, value):
    if key not in self._prefs:
      print 'Error: Invalid preference', key, ', the preference must be declared'
      return

    self._prefs[key] = value
    self._write()

  ##
  # Retrieve the preference vial the [] operator
  def __getitem__(self, key):
    return self._prefs[key]

  ##
  # Writes the preferences to the .peacock file
  def _write(self):
    fid = open(self._filename, 'w')
    pickle.dump(self._prefs, fid)
    fid.close()
