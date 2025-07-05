# SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL


ecm_add_qml_module(kfritzplugin
URI de.agundur.kfritz
VERSION 0.1
GENERATE_PLUGIN_SOURCE
  )
target_sources(kfritzplugin PRIVATE
    FritzCallMonitor.cpp
    FritzDeviceInfoFetcher.cpp
    FritzPhonebookFetcher.cpp
    KFritzCorePlugin.cpp
    PhonebookCache.cpp
    RecentCallsModel.cpp
  )

target_link_libraries(kfritzplugin PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Network
    KF6::CoreAddons
    Qt6::Xml
    KF6::I18n
    KF6::Notifications
  )
ecm_finalize_qml_module(kfritzplugin)
