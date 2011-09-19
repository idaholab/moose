import os

class ExcelWriter:
  ''' This class writes Excel data from a mostly generic nested data structure'''
  def __init__(self, file_name):
    import xlwt

    self.sheets = []
    self.file_name = file_name
    self.wb = xlwt.Workbook()


  def writeExcel(self, sheet_name, struct):
    # Excel has a 31 character limit on tab names
    sheet_name = sheet_name[:31]

    if sheet_name in self.sheets:
      self.curr_ws = self.wb.get_sheet(self.sheets.index(sheet_name))
      row = self.curr_ws.last_used_row + 5
      col = 0
    else:
      self.curr_ws = self.wb.add_sheet(sheet_name)
      self.sheets.append(sheet_name)
      row, col = (0, 0)
    self.writeData(row, col, struct)


  def writeData(self, row, col, struct):
    if type(struct) is str:
      self.curr_ws.write(row, col, struct)
    elif type(struct) is list:
      for item in struct:
        (row, col) = self.writeData(row, col+1, item)
    elif type(struct) is dict:
      # Make sure the "Event" key comes out first - everything else can remain unsorted
      for key, value in sorted(struct.iteritems(), cmp=lambda a, b: (1, -1)[a[0] == 'Event']):
        self.curr_ws.write(row, col, key)
        # see if the value is complex (i.e. a nested structure or not)
        if type(value) is str:
          self.writeData(row, col+1, value)
        else:
          (row, garbage) = self.writeData(row+1, col, value)
        row+=1
    return (row, col)


  def close(self):
    if os.path.exists(self.file_name):
      # clobber the file
      os.remove(self.file_name);
    self.wb.save(self.file_name)

