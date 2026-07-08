# SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL


cmake_minimum_required(VERSION 3.16)

project(kfritz)

set(PROJECT_VERSION "0.1.1")

set(QT_MIN_VERSION "6.7.0")
set(KF6_MIN_VERSION "6.10.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQmlModule)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Qml
    Network
    Xml
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    CoreAddons
    Notifications
)

ki18n_install(translate
    INSTALL_DESTINATION ${KDE_INSTALL_LOCALEDIR}
)

install(FILES
    ${CMAKE_SOURCE_DIR}/package/metadata/notifyrc/kfritz.notifyrc
    DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR}
)

add_subdirectory(package)

# Only ship the plasmoid metadata + QML/config, not the C++ plugin sources or
# the package/metadata/ subdir (notifyrc/appdata.xml — those install to their
# own proper locations below already). A blanket install(DIRECTORY package/)
# was shipping raw .cpp/.h/CMakeLists.txt as inert data files alongside the
# actually-used compiled plugin (which ecm_add_qml_module installs separately
# into the Qt QML plugin dir).
install(FILES package/metadata.json
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/de.agundur.kfritz)

install(DIRECTORY package/contents/
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/de.agundur.kfritz/contents)

install(FILES package/metadata/de.agundur.kfritz.appdata.xml
        DESTINATION ${KDE_INSTALL_DATADIR}/metainfo)
