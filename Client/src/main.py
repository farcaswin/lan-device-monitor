import sys
from PyQt6.QtWidgets import QApplication, QWidget
from main_window import MainWindow

def main():
    app = QApplication([])

    window = MainWindow()
    window.show()

    sys.exit(app.exec())

if __name__ == "__main__":
    main()
