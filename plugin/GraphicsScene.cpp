/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "GraphicsScene.h"
#include "DebugDialog.h"

GraphicsScene::GraphicsScene(QObject* parent) :
   QGraphicsScene(parent)
{
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (mouseEvent->button() == Qt::LeftButton)
   {
      DebugDialog* debugDialog = new DebugDialog(
               dynamic_cast<QWidget*>(this->parent()));
      debugDialog->show();
   }
}
