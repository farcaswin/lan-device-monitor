import sys
from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout,
    QHBoxLayout, QPushButton, QListWidget,
    QListWidgetItem, QTableWidget, QTableWidgetItem,
    QSplitter, QLabel, QStatusBar, QHeaderView,
    QAbstractItemView
)
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QFont
import api


# ---------------------------------------------------------------------------
# Background worker for the network scan
# ---------------------------------------------------------------------------

class ScanWorker(QThread):
    """Runs api.scan_network() in a background thread."""
    finished = pyqtSignal(list)   # emits list of device dicts on success
    error    = pyqtSignal(str)    # emits error message on failure

    def run(self):
        try:
            devices = api.scan_network()
            self.finished.emit(devices)
        except Exception as exc:
            self.error.emit(str(exc))


# ---------------------------------------------------------------------------
# Main window
# ---------------------------------------------------------------------------

class MainWindow(QMainWindow):
    # Packet-sniffer table column definitions (index, header label)
    SNIFFER_COLUMNS = ["No.", "Time", "Source", "Destination", "Protocol", "Length", "Info"]

    def __init__(self):
        super().__init__()
        self._scan_worker: ScanWorker | None = None
        self._setup_ui()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _setup_ui(self):
        self.setWindowTitle("FuzzNET")
        self.setMinimumSize(900, 600)
        self.resize(1200, 700)

        # Status bar
        self._status_bar = QStatusBar()
        self.setStatusBar(self._status_bar)
        self._status_bar.showMessage("Ready")

        # Central widget + outer layout
        central = QWidget()
        self.setCentralWidget(central)
        outer_layout = QVBoxLayout(central)
        outer_layout.setContentsMargins(8, 8, 8, 8)

        # Splitter: left panel | right panel
        splitter = QSplitter(Qt.Orientation.Horizontal)
        outer_layout.addWidget(splitter)

        # --- Left panel ---------------------------------------------------
        left_widget = QWidget()
        left_layout = QVBoxLayout(left_widget)
        left_layout.setContentsMargins(0, 0, 4, 0)

        self._scan_button = QPushButton("Scan")
        self._scan_button.setFixedHeight(36)
        font = QFont()
        font.setBold(True)
        self._scan_button.setFont(font)
        self._scan_button.clicked.connect(self._on_scan)
        left_layout.addWidget(self._scan_button)

        devices_label = QLabel("Devices")
        left_layout.addWidget(devices_label)

        self._device_list = QListWidget()
        self._device_list.setAlternatingRowColors(True)
        self._device_list.itemClicked.connect(self._on_device_selected)
        left_layout.addWidget(self._device_list)

        splitter.addWidget(left_widget)

        # --- Right panel --------------------------------------------------
        right_widget = QWidget()
        right_layout = QVBoxLayout(right_widget)
        right_layout.setContentsMargins(4, 0, 0, 0)

        sniffer_label = QLabel("Packet Sniffer")
        right_layout.addWidget(sniffer_label)

        self._packet_table = QTableWidget(0, len(self.SNIFFER_COLUMNS))
        self._packet_table.setHorizontalHeaderLabels(self.SNIFFER_COLUMNS)
        self._packet_table.horizontalHeader().setSectionResizeMode(
            len(self.SNIFFER_COLUMNS) - 1, QHeaderView.ResizeMode.Stretch
        )
        self._packet_table.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self._packet_table.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        self._packet_table.setAlternatingRowColors(True)
        self._packet_table.verticalHeader().setVisible(False)
        right_layout.addWidget(self._packet_table)

        splitter.addWidget(right_widget)

        # Initial split ratio: 1/3 left, 2/3 right
        splitter.setSizes([300, 700])

    # ------------------------------------------------------------------
    # Slots
    # ------------------------------------------------------------------

    def _on_scan(self):
        """Start a background network scan."""
        if self._scan_worker and self._scan_worker.isRunning():
            return  # already scanning

        self._scan_button.setEnabled(False)
        self._scan_button.setText("Scanning…")
        self._device_list.clear()
        self._status_bar.showMessage("Scanning network…")

        self._scan_worker = ScanWorker()
        self._scan_worker.finished.connect(self._on_scan_finished)
        self._scan_worker.error.connect(self._on_scan_error)
        self._scan_worker.start()

    def _on_scan_finished(self, devices: list):
        """Populate the device list with scan results."""
        self._device_list.clear()
        for device in devices:
            # Accept dicts like {"ip": "...", "mac": "...", "hostname": "..."}
            ip       = device.get("ip", "unknown")
            mac      = device.get("mac", "")
            hostname = device.get("hostname", "")
            label    = ip
            if hostname:
                label = f"{hostname} ({ip})"
            if mac:
                label += f"  [{mac}]"
            item = QListWidgetItem(label)
            item.setData(Qt.ItemDataRole.UserRole, device)
            self._device_list.addItem(item)

        count = len(devices)
        self._status_bar.showMessage(f"Scan complete – {count} device(s) found.")
        self._reset_scan_button()

    def _on_scan_error(self, message: str):
        self._status_bar.showMessage(f"Scan failed: {message}")
        self._reset_scan_button()

    def _reset_scan_button(self):
        self._scan_button.setEnabled(True)
        self._scan_button.setText("Scan")

    def _on_device_selected(self, item: QListWidgetItem):
        """Handle a device being clicked in the list."""
        device = item.data(Qt.ItemDataRole.UserRole)
        ip = device.get("ip", "unknown") if device else item.text()
        self._status_bar.showMessage(f"Selected device: {ip}")
        # Future: trigger per-device actions (e.g. start sniffing)

    # ------------------------------------------------------------------
    # Public helpers (called by the future packet-sniffer integration)
    # ------------------------------------------------------------------

    def append_packet(self, no: int, time: str, src: str, dst: str,
                      protocol: str, length: int, info: str):
        """Append one row to the packet-sniffer table."""
        row = self._packet_table.rowCount()
        self._packet_table.insertRow(row)
        for col, value in enumerate([no, time, src, dst, protocol, length, info]):
            self._packet_table.setItem(row, col, QTableWidgetItem(str(value)))

    def clear_packets(self):
        """Remove all rows from the packet-sniffer table."""
        self._packet_table.setRowCount(0)
