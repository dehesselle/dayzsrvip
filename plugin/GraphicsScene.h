/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class GraphicsScene : public QGraphicsScene
{
   Q_OBJECT

public:
   GraphicsScene(QObject* parent = Q_NULLPTR);

   void setParentWidget(QWidget* widget);
   virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);

private:
   QWidget* m_parent;
};

#endif // GRAPHICSSCENE_H
