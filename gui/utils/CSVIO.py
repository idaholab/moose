import csv

class CSVIO(object):
  def __init__(self, filename):

    # Read the file
    reader = csv.reader(filename)

    # Extract the data into a dictionary
    self._data = dict()
    on_header = True

    # Loop through the rows
    for row in reader:

      # Store the header and initialize the data dictionary
      if on_header:
        headers = row
        for h in headers:
          self._data[h] = []
        on_header = False


      # Extract the data
      else:
        for idx in xrange(len(row)):
          self._data[headers[idx]].append(row[idx])




  def.__getitem__(self, key):

    try:
      return self._data[key]

    except KeyError:

      print "No data..."
      # We need to come up with a way of creating error dialogs that would work with free, non MooseWidget functions/classes
      #peacockError()

      return None
