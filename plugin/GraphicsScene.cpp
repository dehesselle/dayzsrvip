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
      DebugDialog* debugDialog = new DebugDialog(m_parent);
      debugDialog->show();
   }
}

void GraphicsScene::setParentWidget(QWidget *widget)
{
   m_parent = widget;
}
