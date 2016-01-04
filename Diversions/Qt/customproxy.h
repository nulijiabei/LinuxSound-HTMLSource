customproxy.h


#ifndef CUSTOMPROXY_H
#define CUSTOMPROXY_H

#include <QGraphicsProxyWidget>

class CustomProxy : public QGraphicsProxyWidget
{
Q_OBJECT
public:
CustomProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

QRectF boundingRect() const;


protected:
void resizeEvent(QGraphicsSceneResizeEvent *event);

private slots:

private:

};

#endif
