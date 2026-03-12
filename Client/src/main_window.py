import api
from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QListWidget, QListWidgetItem,
    QTableWidget, QTableWidgetItem, QSplitter,
    QLabel, QStatusBar, QHeaderView, QAbstractItemView,
    QFrame
)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QSize
from PyQt6.QtGui import QFont


# ── Workers ───────────────────────────────────────────────────────

class ScanWorker(QThread):
    finished = pyqtSignal(dict)
    error    = pyqtSignal(str)

    def __init__(self, subnet: str = ""):
        super().__init__()
        self.subnet = subnet

    def run(self):
        try:
            self.finished.emit(api.scan_network(self.subnet))
        except Exception as e:
            self.error.emit(str(e))


class DeviceDetailWorker(QThread):
    finished = pyqtSignal(dict)
    error    = pyqtSignal(str)

    def __init__(self, device_id: str):
        super().__init__()
        self.device_id = device_id

    def run(self):
        try:
            self.finished.emit(api.get_device(self.device_id))
        except Exception as e:
            self.error.emit(str(e))


class SetTargetWorker(QThread):
    finished = pyqtSignal(dict)
    error    = pyqtSignal(str)

    def __init__(self, device_id: str):
        super().__init__()
        self.device_id = device_id

    def run(self):
        try:
            self.finished.emit(api.set_target(self.device_id))
        except Exception as e:
            self.error.emit(str(e))


# ── Device Item Widget ────────────────────────────────────────────

class DeviceItemWidget(QWidget):
    target_requested  = pyqtSignal(str)
    details_requested = pyqtSignal(str)

    def __init__(self, device: dict, parent=None):
        super().__init__(parent)
        self.device    = device
        self.device_id = device.get("id", "")
        self.expanded  = False

        # Referinta la QListWidgetItem — setata din exterior dupa addItem
        # Necesara pentru a actualiza sizeHint la expand/collapse
        self._list_item: QListWidgetItem | None = None

        self._setup_ui()

    def set_list_item(self, item: QListWidgetItem):
        """Stocheaza referinta la item pentru resize dinamic."""
        self._list_item = item

    def _setup_ui(self):
        self._layout = QVBoxLayout(self)
        self._layout.setContentsMargins(6, 4, 6, 4)
        self._layout.setSpacing(0)

        # ── Header ────────────────────────────────────────────────
        header_widget = QWidget()
        header = QHBoxLayout(header_widget)
        header.setContentsMargins(0, 0, 0, 0)

        ip     = self.device.get("ip", "N/A")
        vendor = self.device.get("vendor", "")
        name   = self.device.get("name", "")

        label_text = name if name else ip
        if name:
            label_text += f"  ({ip})"
        if vendor:
            label_text += f"  [{vendor}]"

        self._label = QLabel(label_text)
        f = QFont()
        f.setBold(True)
        self._label.setFont(f)
        header.addWidget(self._label)
        header.addStretch()

        self._expand_btn = QPushButton("▼")
        self._expand_btn.setFixedSize(28, 28)
        self._expand_btn.setFlat(True)
        self._expand_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._expand_btn.clicked.connect(self._toggle_expand)
        header.addWidget(self._expand_btn)

        self._layout.addWidget(header_widget)

        # ── Detail panel (ascuns initial) ─────────────────────────
        self._detail_panel = QFrame()
        self._detail_panel.setFrameShape(QFrame.Shape.StyledPanel)
        self._detail_panel.setContentsMargins(0, 0, 0, 0)

        detail_layout = QVBoxLayout(self._detail_panel)
        detail_layout.setContentsMargins(10, 6, 10, 6)
        detail_layout.setSpacing(4)

        mac = self.device.get("mac_address", "N/A")
        os  = self.device.get("os_version",  "N/A")

        for lbl_text, val_text in [
            ("IP",     self.device.get("ip", "N/A")),
            ("MAC",    mac),
            ("Vendor", vendor or "N/A"),
            ("OS",     os),
        ]:
            row = QHBoxLayout()
            row.setSpacing(8)
            lbl = QLabel(f"{lbl_text}:")
            lbl.setFixedWidth(55)
            lbl.setStyleSheet("color: #888888;")
            val = QLabel(val_text)
            val.setTextInteractionFlags(
                Qt.TextInteractionFlag.TextSelectableByMouse
            )
            row.addWidget(lbl)
            row.addWidget(val)
            row.addStretch()
            detail_layout.addLayout(row)

        # Separator
        sep = QFrame()
        sep.setFrameShape(QFrame.Shape.HLine)
        sep.setStyleSheet("color: #444444;")
        detail_layout.addWidget(sep)

        # Ports label
        self._ports_label = QLabel("Ports: loading...")
        self._ports_label.setStyleSheet("color: #888888; font-style: italic;")
        self._ports_label.setWordWrap(True)
        detail_layout.addWidget(self._ports_label)

        # Buton Select Target
        self._target_btn = QPushButton("◎  Select as Target")
        self._target_btn.setFixedHeight(32)
        self._target_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._target_btn.clicked.connect(
            lambda: self.target_requested.emit(self.device_id)
        )
        detail_layout.addWidget(self._target_btn)

        self._detail_panel.setVisible(False)
        self._layout.addWidget(self._detail_panel)

        # Forteaza dimensiunea initiala (doar header-ul)
        self.setFixedHeight(self._collapsed_height())

    def _collapsed_height(self) -> int:
        return 40  # inaltimea header-ului

    def _expanded_height(self) -> int:
        # Header + detail panel
        self._detail_panel.adjustSize()
        return self._collapsed_height() + self._detail_panel.sizeHint().height() + 8

    # ── Toggle expand ─────────────────────────────────────────────

    def _toggle_expand(self):
        self.expanded = not self.expanded
        self._detail_panel.setVisible(self.expanded)
        self._expand_btn.setText("▲" if self.expanded else "▼")

        if self.expanded:
            # Cere detalii de la server (porturi etc.)
            self.details_requested.emit(self.device_id)
            new_height = self._expanded_height()
        else:
            new_height = self._collapsed_height()

        # Redimensioneaza widget-ul
        self.setFixedHeight(new_height)

        # ── FIX PRINCIPAL ─────────────────────────────────────────
        # Actualizeaza sizeHint-ul QListWidgetItem
        # Fara asta, QListWidget nu stie ca item-ul si-a schimbat dimensiunea
        if self._list_item is not None:
            self._list_item.setSizeHint(QSize(self.width(), new_height))

    # ── Update ports ─────────────────────────────────────────────

    def update_ports(self, ports: list):
        if not ports:
            self._ports_label.setText("Ports: none found")
            self._ports_label.setStyleSheet("color: #888888;")
        else:
            port_strs = [
                f"{p['port_number']}/{p['protocol']} ({p.get('service','?')})"
                for p in ports
            ]
            self._ports_label.setText("Ports: " + "  |  ".join(port_strs))
            self._ports_label.setStyleSheet("color: #2ecc71;")

        # Dupa update ports, recalculam inaltimea daca e expandat
        if self.expanded and self._list_item is not None:
            new_height = self._expanded_height()
            self.setFixedHeight(new_height)
            self._list_item.setSizeHint(QSize(self.width(), new_height))

    def set_as_target(self, is_target: bool):
        if is_target:
            self._label.setStyleSheet("color: #e74c3c; font-weight: bold;")
            self._target_btn.setText("✓  Current Target")
            self._target_btn.setEnabled(False)
        else:
            self._label.setStyleSheet("")
            self._target_btn.setText("◎  Select as Target")
            self._target_btn.setEnabled(True)


# ── Main Window ───────────────────────────────────────────────────

class MainWindow(QMainWindow):

    PORT_COLUMNS = ["Port", "Protocol", "Service"]

    def __init__(self):
        super().__init__()
        self._scan_worker:   ScanWorker        | None = None
        self._target_worker: SetTargetWorker   | None = None
        self._detail_workers: list[DeviceDetailWorker] = []

        self._device_widgets:    dict[str, DeviceItemWidget] = {}
        self._current_target_id: str | None = None

        self._setup_ui()

    def _setup_ui(self):
        self.setWindowTitle("FuzzNET")
        self.setMinimumSize(900, 600)
        self.resize(1280, 720)

        self._status_bar = QStatusBar()
        self.setStatusBar(self._status_bar)
        self._status_bar.showMessage("Ready")

        central = QWidget()
        self.setCentralWidget(central)
        outer = QVBoxLayout(central)
        outer.setContentsMargins(8, 8, 8, 8)

        splitter = QSplitter(Qt.Orientation.Horizontal)
        outer.addWidget(splitter)

        # ── Panoul stang ──────────────────────────────────────────
        left = QWidget()
        left_layout = QVBoxLayout(left)
        left_layout.setContentsMargins(0, 0, 4, 0)

        self._scan_btn = QPushButton("⟳  Scan Network")
        self._scan_btn.setFixedHeight(36)
        f = QFont(); f.setBold(True)
        self._scan_btn.setFont(f)
        self._scan_btn.clicked.connect(self._on_scan)
        left_layout.addWidget(self._scan_btn)

        self._network_label = QLabel("Network: detecting...")
        self._network_label.setStyleSheet("color: gray; font-size: 11px;")
        left_layout.addWidget(self._network_label)

        left_layout.addWidget(QLabel("Devices"))

        self._device_list = QListWidget()
        self._device_list.setSpacing(2)
        self._device_list.setUniformItemSizes(False)   # ← obligatoriu pentru height dinamic
        left_layout.addWidget(self._device_list)

        splitter.addWidget(left)

        # ── Panoul drept ──────────────────────────────────────────
        right = QWidget()
        right_layout = QVBoxLayout(right)
        right_layout.setContentsMargins(4, 0, 0, 0)

        self._target_label = QLabel("Target: none selected")
        f2 = QFont(); f2.setBold(True)
        self._target_label.setFont(f2)
        right_layout.addWidget(self._target_label)

        right_layout.addWidget(QLabel("Open Ports"))

        self._port_table = QTableWidget(0, len(self.PORT_COLUMNS))
        self._port_table.setHorizontalHeaderLabels(self.PORT_COLUMNS)
        self._port_table.horizontalHeader().setSectionResizeMode(
            QHeaderView.ResizeMode.Stretch
        )
        self._port_table.setEditTriggers(
            QAbstractItemView.EditTrigger.NoEditTriggers
        )
        self._port_table.setSelectionBehavior(
            QAbstractItemView.SelectionBehavior.SelectRows
        )
        self._port_table.setAlternatingRowColors(True)
        self._port_table.verticalHeader().setVisible(False)
        right_layout.addWidget(self._port_table)

        splitter.addWidget(right)
        splitter.setSizes([380, 900])

        self._fetch_network_info()

    # ── Network info ──────────────────────────────────────────────

    def _fetch_network_info(self):
        class NetInfoWorker(QThread):
            done = pyqtSignal(dict)
            fail = pyqtSignal(str)
            def run(self):
                try:    self.done.emit(api.get_network_info())
                except Exception as e: self.fail.emit(str(e))

        self._net_worker = NetInfoWorker()
        self._net_worker.done.connect(
            lambda info: self._network_label.setText(
                f"Network: {info.get('subnet','?')}  ({info.get('interface','?')})"
            )
        )
        self._net_worker.fail.connect(
            lambda e: self._network_label.setText(f"Network: {e}")
        )
        self._net_worker.start()

    # ── Scan ──────────────────────────────────────────────────────

    def _on_scan(self):
        if self._scan_worker and self._scan_worker.isRunning():
            return

        self._scan_btn.setEnabled(False)
        self._scan_btn.setText("Scanning…")
        self._device_list.clear()
        self._device_widgets.clear()
        self._status_bar.showMessage("Scanning network…")

        self._scan_worker = ScanWorker()
        self._scan_worker.finished.connect(self._on_scan_finished)
        self._scan_worker.error.connect(self._on_scan_error)
        self._scan_worker.start()

    def _on_scan_finished(self, result: dict):
        for device in result.get("devices", []):
            self._add_device_item(device)

        count = result.get("total_found", len(result.get("devices", [])))
        self._status_bar.showMessage(f"Found {count} device(s).")
        self._scan_btn.setEnabled(True)
        self._scan_btn.setText("⟳  Scan Network")

    def _on_scan_error(self, message: str):
        self._status_bar.showMessage(f"Scan failed: {message}")
        self._scan_btn.setEnabled(True)
        self._scan_btn.setText("⟳  Scan Network")

    # ── Device list ───────────────────────────────────────────────

    def _add_device_item(self, device: dict):
        widget = DeviceItemWidget(device)
        widget.target_requested.connect(self._on_target_requested)
        widget.details_requested.connect(self._fetch_device_details)

        item = QListWidgetItem(self._device_list)

        # Dimensiunea initiala = doar header-ul
        initial_size = QSize(self._device_list.viewport().width(),
                             widget._collapsed_height())
        item.setSizeHint(initial_size)

        self._device_list.addItem(item)
        self._device_list.setItemWidget(item, widget)

        # ── FIX PRINCIPAL ─────────────────────────────────────────
        # Pasam referinta la item catre widget
        # Astfel widget-ul poate apela item.setSizeHint() la toggle
        widget.set_list_item(item)

        device_id = device.get("id", "")
        if device_id:
            self._device_widgets[device_id] = widget

    # ── Device details on expand ──────────────────────────────────

    def _fetch_device_details(self, device_id: str):
        worker = DeviceDetailWorker(device_id)
        worker.finished.connect(self._on_device_details_ready)
        worker.error.connect(
            lambda e: self._status_bar.showMessage(f"Detail fetch failed: {e}")
        )
        worker.start()
        self._detail_workers.append(worker)

    def _on_device_details_ready(self, device: dict):
        device_id = device.get("id", "")
        widget = self._device_widgets.get(device_id)
        if widget:
            widget.update_ports(device.get("open_ports", []))

        # Daca e target curent, actualizeaza si tabela din dreapta
        if device_id == self._current_target_id:
            self._populate_port_table(device.get("open_ports", []))

    # ── Target ────────────────────────────────────────────────────

    def _on_target_requested(self, device_id: str):
        if self._target_worker and self._target_worker.isRunning():
            return

        self._status_bar.showMessage(f"Setting target: {device_id}…")
        self._target_worker = SetTargetWorker(device_id)
        self._target_worker.finished.connect(self._on_target_set)
        self._target_worker.error.connect(
            lambda e: self._status_bar.showMessage(f"Set target failed: {e}")
        )
        self._target_worker.start()

    def _on_target_set(self, result: dict):
        device    = result.get("device", {})
        device_id = device.get("id", "")

        if self._current_target_id:
            old = self._device_widgets.get(self._current_target_id)
            if old:
                old.set_as_target(False)

        self._current_target_id = device_id

        widget = self._device_widgets.get(device_id)
        if widget:
            widget.set_as_target(True)

        ip = device.get("ip", device_id)
        self._target_label.setText(f"Target: {ip}")
        self._status_bar.showMessage(
            result.get("message", f"Target set to {ip}")
        )
        self._populate_port_table(device.get("open_ports", []))

    def _populate_port_table(self, ports: list):
        self._port_table.setRowCount(0)
        for p in ports:
            row = self._port_table.rowCount()
            self._port_table.insertRow(row)
            self._port_table.setItem(
                row, 0, QTableWidgetItem(str(p.get("port_number", "")))
            )
            self._port_table.setItem(
                row, 1, QTableWidgetItem(p.get("protocol", ""))
            )
            self._port_table.setItem(
                row, 2, QTableWidgetItem(p.get("service", ""))
            )