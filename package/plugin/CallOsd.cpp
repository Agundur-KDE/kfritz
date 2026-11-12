// SPDX-License-Identifier: GPL-3.0-or-later
#include "CallOsd.h"

#include <KLocalizedString>
#include <LayerShellQt/Window>
#include <QGuiApplication>
#include <algorithm>

CallOsd::CallOsd(QObject *parent)
    : QObject(parent)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(8000);
    connect(&m_timeout, &QTimer::timeout, this, &CallOsd::hide);
}

bool CallOsd::supported() const
{
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"));
}

void CallOsd::hide()
{
    m_timeout.stop();
    m_view.reset();
}

void CallOsd::showCall(const QString &name, const QString &number, const QFont &font, int position, const QColor &textColor)
{
    if (!supported())
        return;

    // Recreate the surface so screen selection and position apply to every call.
    hide();
    m_view = std::make_unique<QQuickView>();
    m_view->setColor(Qt::transparent);
    m_view->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
    m_view->setResizeMode(QQuickView::SizeViewToRootObject);

    auto *layer = LayerShellQt::Window::get(m_view.get());
    layer->setScope(QStringLiteral("kfritz-call-osd"));
    layer->setLayer(LayerShellQt::Window::LayerOverlay);
    layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer->setExclusiveZone(-1);
#if defined(KFRITZ_LAYERSHELL_ACTIVE_SCREEN) && KFRITZ_LAYERSHELL_ACTIVE_SCREEN
    layer->setWantsToBeOnActiveScreen(true);
#else
    layer->setScreenConfiguration(LayerShellQt::Window::ScreenFromCompositor);
#endif
    position = std::clamp(position, 0, 8);
    LayerShellQt::Window::Anchors anchors;
    if (position / 3 == 0) anchors |= LayerShellQt::Window::AnchorTop;
    if (position / 3 == 2) anchors |= LayerShellQt::Window::AnchorBottom;
    if (position % 3 == 0) anchors |= LayerShellQt::Window::AnchorLeft;
    if (position % 3 == 2) anchors |= LayerShellQt::Window::AnchorRight;
    layer->setAnchors(anchors);
    layer->setMargins(QMargins(24, 24, 24, 24));

    m_view->setInitialProperties({
        {QStringLiteral("callerName"), name.isEmpty() ? i18nd("plasma_applet_de.agundur.kfritz", "Unknown caller") : name},
        {QStringLiteral("callerNumber"), number.isEmpty() ? i18nd("plasma_applet_de.agundur.kfritz", "Withheld number") : number},
        {QStringLiteral("heading"), i18nd("plasma_applet_de.agundur.kfritz", "Incoming call")},
        {QStringLiteral("callerFont"), font},
        {QStringLiteral("textColor"), textColor.isValid() ? textColor : QColor(Qt::white)},
    });
    m_view->setSource(QUrl(QStringLiteral("qrc:/kfritz/OsdCard.qml")));
    if (m_view->status() == QQuickView::Error) {
        hide();
        return;
    }
    m_view->show();
    m_timeout.start();
}
