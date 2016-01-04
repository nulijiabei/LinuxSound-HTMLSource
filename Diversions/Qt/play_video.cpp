/* From http://www.qtcentre.org/archive/index.php/t-39496.html
*/
#include "customproxy.h"

#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/videowidget.h>


#include <QtGui>
#include <QtDebug>
#include <QAbstractFileEngine>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

int main(int argc, char *argv[])
{
Q_INIT_RESOURCE(videowall);
QApplication app(argc, argv);

QGraphicsScene scene;

Phonon::MediaObject *media = new Phonon::MediaObject();
Phonon::VideoWidget *video = new Phonon::VideoWidget();
CustomProxy *proxy = new CustomProxy(0, 0);
Phonon::createPath(media, video);
proxy->setWidget(video);
QRectF rect = proxy->boundingRect();
proxy->setPos(0, 0);
proxy->show();
scene.addItem(proxy);

media->setCurrentSource(QString("output22.avi"));

media->play();

QGraphicsTextItem *titem = scene.addText("Bla-bla-bla");
titem->setPos(rect.width()/2, rect.height()/2);


QGraphicsView view(&scene);
#ifndef QT_NO_OPENGL
view.setViewport(new QGLWidget);
#endif
view.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
view.setViewportUpdateMode(QGraphicsView::Bounding RectViewportUpdate);
view.show();
view.setWindowTitle("Eternal fire");
// view.showFullScreen();

return app.exec();
}
