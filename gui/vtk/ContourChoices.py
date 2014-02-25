class ContourRangeType:
  current  = 0
  all_time = 1
  custom   = 2

class ContourChoices:
  def __init__(self):
    self.component = None
    self.min_type = ContourRangeType.current
    self.min_custom_text = ''
    self.max_type = ContourRangeType.current
    self.max_custom_text = ''

  def save(self, widget):
    self.component = widget.current_component

    if widget.min_current_radio.isChecked():
      self.min_type = ContourRangeType.current
    elif widget.min_global_radio.isChecked():
      self.min_type = ContourRangeType.all_time
    elif widget.min_custom_radio.isChecked():
      self.min_type = ContourRangeType.custom

    if widget.max_current_radio.isChecked():
      self.max_type = ContourRangeType.current
    elif widget.max_global_radio.isChecked():
      self.max_type = ContourRangeType.all_time
    elif widget.max_custom_radio.isChecked():
      self.max_type = ContourRangeType.custom

    self.min_custom_text = widget.min_custom_text.displayText()
    self.max_custom_text = widget.max_custom_text.displayText()

  def restore(self, widget):
    if self.component:
      found_index = widget.variable_component.findText(self.component)
      widget.variable_component.setCurrentIndex(found_index)
    else:
      found_index = widget.variable_component.findText('Magnitude')
      widget.variable_component.setCurrentIndex(found_index)

    if self.min_type == ContourRangeType.current:
      widget.min_current_radio.setChecked(True)
    elif self.min_type == ContourRangeType.all_time:
      widget.min_global_radio.setChecked(True)
    elif self.min_type == ContourRangeType.custom:
      widget.min_custom_text.setText(self.min_custom_text)
      widget.min_custom_text.setCursorPosition(0)
      widget.min_custom_radio.setChecked(True)

    if self.max_type == ContourRangeType.current:
      widget.max_current_radio.setChecked(True)
    elif self.max_type == ContourRangeType.all_time:
      widget.max_global_radio.setChecked(True)
    elif self.max_type == ContourRangeType.custom:
      widget.max_custom_text.setText(self.max_custom_text)
      widget.max_custom_text.setCursorPosition(0)
      widget.max_custom_radio.setChecked(True)

    widget._updateContours()
