customproxy.cpp


#include "customproxy.h"

#include <QtGui>

CustomProxy::CustomProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
: QGraphicsProxyWidget(parent, wFlags)
{
setCacheMode(NoCache);
}

QRectF CustomProxy::boundingRect() const
{
return QGraphicsProxyWidget::boundingRect().adjusted(0, 0, 0, 0);
}

void CustomProxy::resizeEvent(QGraphicsSceneResizeEvent *event)
{
//
}
