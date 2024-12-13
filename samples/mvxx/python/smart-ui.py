import sys
from PyQt5 import QtCore, QtGui

class Counter(QtGui.QPushButton):
    def __init__(self, *args, **kwargs):
        super(Counter, self).__init__(*args, **kwargs)
        self._value = 0
        self._update()

    def mouseReleaseEvent(self, event):
        super(Counter, self).mouseReleaseEvent(event)
        self._value += 1
        self._update()

    def _update(self):
        self.setText(unicode(self._value))

app = QtGui.QApplication(sys.argv)
counter = Counter()
counter.show()
app.exec_()
