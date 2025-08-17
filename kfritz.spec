%ifarch aarch64
%undefine source_date_epoch_from_changelog
%endif

Name:           kfritz
Version:        0.1.0
Release:        1%{?dist}
Summary:        A KDE Plasma 6 callmonitor plasmoid for the AVM FRITZ!Box.

License:        GPL-3.0-or-later
URL:            https://github.com/Agundur-KDE/kfritz


BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires: gettext

BuildRequires: qt6-base-devel
BuildRequires: qt6-declarative-devel
BuildRequires: kf6-extra-cmake-modules
BuildRequires: qt6-tools-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  kf6-kpackage-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires: cmake(KF6Config)
BuildRequires: cmake(KF6KCMUtils)
BuildRequires: cmake(KF6Notifications)
BuildRequires: cmake(KF6NotifyConfig)
BuildRequires: cmake(KF6GlobalAccel)
BuildRequires: cmake(KF6GuiAddons)
BuildRequires: cmake(KF6WidgetsAddons)
BuildRequires: cmake(KF6IconThemes)
BuildRequires: cmake(KF6Codecs)
BuildRequires: cmake(KF6XmlGui)
BuildRequires:  pkgconfig(libbrotlidec)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libffi)
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  pkgconfig(libnghttp2)
BuildRequires:  pkgconfig(libidn2)
BuildRequires:  pkgconfig(libpsl)
BuildRequires:  pkgconfig(libssh)
#BuildRequires: pkgconfig(Qt6NetworkInformation)




Requires: plasma6-workspace
# Requires: libQt6NetworkInformation6-devel


%description
KFritz is a KDE Plasma 6 Plasmoid that connects to your AVM Fritz!Box and displays real-time incoming calls. It shows the caller name and number, maintains a history of recent calls, and integrates with the KDE notification system for alerts.


Source0: _service

%prep

rm -rf ./*

shopt -s nullglob
picked=""
for d in %{_sourcedir}/kfritz-* %{_sourcedir}/kfritz-* %{_sourcedir}/kfritz ; do
  if [ -d "$d" ] && [ -f "$d/CMakeLists.txt" ]; then
    picked="$d"
    break
  fi
done

if [ -n "$picked" ]; then
  # Inhalt des Quellordners (inkl. Dotfiles) in den Build-Root kopieren
  cp -a "$picked"/. .
else
  # Flacher Checkout: aus SOURCES kopieren, aber Packaging-/Service-Dateien AUSLASSEN
  for f in %{_sourcedir}/* ; do
    base="$(basename "$f")"
    case "$base" in
      *.spec|*.dsc|*.changes|*.obsinfo|_service|service_attic|screenshot|*.patch)
        continue ;;
    esac
    cp -a "$f" .
  done
fi

%build
%cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=%{_prefix}
%cmake_build

%install
%cmake_install

install -Dm644 package/metadata/de.agundur.kfritz.appdata.xml \
  %{buildroot}%{_datadir}/metainfo/de.agundur.kfritz.appdata.xml

%files

%{_datadir}/metainfo/de.agundur.kfritz.appdata.xml
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents/icons/*.*
%{_datadir}/knotifications6/kfritz.notifyrc

%dir %{_libdir}/qt6/qml/de/agundur/kfritz
%{_libdir}/qt6/qml/de/agundur/kfritz/*

%dir %{_qt6_qmldir}/de
%dir %{_qt6_qmldir}/de/agundur
%{_qt6_qmldir}/de/agundur/kfritz/
%dir %{_datadir}/plasma/plasmoids/de.agundur.kfritz
%dir %{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents
%dir %{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents/ui
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/metadata.json
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents/ui/main.qml
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/CMakeLists.txt
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/CMakeLists.txt
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzCallMonitor.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzCallMonitor.h
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzDeviceInfoFetcher.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzDeviceInfoFetcher.h
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzPhonebookFetcher.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/FritzPhonebookFetcher.h
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/KFritzCorePlugin.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/KFritzCorePlugin.h
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/PhonebookCache.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/PhonebookCache.h
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/RecentCallsModel.cpp
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin/RecentCallsModel.h
%dir %{_datadir}/plasma/plasmoids/de.agundur.kfritz/plugin
%dir %{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents/icons
%{_datadir}/plasma/plasmoids/de.agundur.kfritz/contents/icons/*
%{_datadir}/locale/*/LC_MESSAGES/plasma_applet_de.agundur.kfritz.mo
%changelog
* Mon Jul 07 2025 Alec <info@agundur.de> - 0.1.0 beta
- 0.1.0 beta with translation and multiple bug fixes