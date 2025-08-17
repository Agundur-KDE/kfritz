Name:           kfritz
Version:        0.1.0

Release:        1%{?dist}
URL:            https://github.com/Agundur-KDE/kfritz
Summary:        A KDE Plasma 6 callmonitor plasmoid for the AVM FRITZ!Box on your desktop.
License:        GPL-3.0-or-later
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz#/kfritz-%{version}.tar.gz

# --- Build requirements (C++/CMake + Qt6 + KF6/Plasma 6)
BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-kpackage-devel
BuildRequires:  libplasma-devel
BuildRequires:  kf6-kcmutils-devel
BuildRequires:  kf6-knotifications-devel
BuildRequires:  kf6-knotifyconfig-devel
BuildRequires:  kf6-kglobalaccel-devel
BuildRequires:  kf6-kguiaddons-devel
BuildRequires:  kf6-kwidgetsaddons-devel
BuildRequires:  kf6-kcodecs-devel
BuildRequires:  kf6-kiconthemes-devel
BuildRequires:  kf6-kxmlgui-devel


# --- Runtime
Requires:       plasma-workspace >= 6


%description
kfritz A KDE Plasma 6 callmonitor plasmoid for the AVM FRITZ!Box on your desktop.

%prep
%autosetup -n kfritz-%{version}

%build
%cmake -S . \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
  -DKDE_INSTALL_QMLDIR=%{_qt6_qmldir} \
  -DKDE_INSTALL_PLUGINDIR=%{_qt6_plugindir}
%cmake_build


%install
%cmake_install


%files
%license LICENSE*
%doc README*
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/
%{_libdir}/qt6/qml/de/agundur/kfritz*/**
%{_datadir}/knotifications6/kfritz.notifyrc
%{_datadir}/metainfo/de.agundur.kfritz.appdata.xml
%{_datadir}/locale/*/LC_MESSAGES/plasma_applet_de.agundur.kfriz.mo

%changelog
* Wed Aug 13 2025 Agundur <info@agundur.de> - 0.1.0
-1
- beta 1 with translations
