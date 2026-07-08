%ifarch aarch64
%undefine source_date_epoch_from_changelog
%endif

Name:           kfritz
Version:        0.1.1
Release:        1%{?dist}
Summary:        KDE Plasma 6 callmonitor plasmoid for the AVM FRITZ!Box

License:        GPL-3.0-or-later
URL:            https://github.com/Agundur-KDE/kfritz


BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  extra-cmake-modules
BuildRequires:  qt6-base-devel
BuildRequires:  qt6-declarative-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-knotifications-devel


Requires:       plasma6-workspace

%description
KFritz is a KDE Plasma 6 Plasmoid that connects to your AVM FRITZ!Box and
displays real-time incoming calls. It shows the caller name and number,
maintains a history of recent calls, and integrates with the KDE
notification system for alerts.

Source0: _service

%prep

rm -rf ./*

shopt -s nullglob
picked=""
for d in %{_sourcedir}/kfritz-* %{_sourcedir}/KFritz-* %{_sourcedir}/kfritz ; do
  if [ -d "$d" ] && [ -f "$d/CMakeLists.txt" ]; then
    picked="$d"
    break
  fi
done

if [ -n "$picked" ]; then
  cp -a "$picked"/. .
else
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
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \
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
%{_qt6_qmldir}/de/agundur/kfritz/
%{_datadir}/knotifications6/kfritz.notifyrc
%{_datadir}/metainfo/de.agundur.kfritz.appdata.xml
%{_datadir}/locale/*/LC_MESSAGES/plasma_applet_de.agundur.kfritz.mo

%changelog
* Wed Jul 08 2026 Alec <info@agundur.de> - 0.1.1
- First automated OBS release via GitHub Actions obs-submit.yml (was
  maintained by hand until now — spec had drifted to include unused
  BuildRequires like libcurl/libssh/libsystemd and was shipping raw
  plugin/*.cpp,*.h as installed data files)
- Fixed %files to match the corrected CMake install rules (source files
  no longer installed anywhere)
- Pruned BuildRequires to what the plugin actually links against
