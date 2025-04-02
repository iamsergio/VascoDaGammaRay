// SPDX-License-Identifier: MIT

#include <QQuickView>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QQuickView view;
    view.setSource(QUrl("qrc:/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(500, 500);
    view.show();

    app.exec();
}
