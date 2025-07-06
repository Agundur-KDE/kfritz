# KFritz

KFritz is a KDE Plasma Plasmoid that connects to your AVM Fritz!Box and displays real-time incoming call notifications. It shows the caller name and number, maintains a history of recent calls, and integrates with the KDE notification system for alerts.

Designed for modern Plasma 6 environments, KFritz supports:
- Real-time call monitoring via the FritzBox CallMonitor (port 1012)
- Caller name resolution from the FritzBox phonebook via TR-064
- Recent call history with timestamps
- Compact and full Plasma representations
- KDE notifications using KNotification

---

## 🛠 Installation

### From source

```bash
git clone https://github.com/yourusername/kfritz.git
cd kfritz
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
plasmoidviewer -a de.agundur.kfritz  # or restart plasmashell
```
