/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>

namespace Ui {
class DebugDialog;
}

class DebugDialog : public QDialog
{
   Q_OBJECT

public:
   explicit DebugDialog(QWidget *parent = 0);
   ~DebugDialog();

private slots:
   void on_pbSubmit_clicked();

private:
   Ui::DebugDialog *ui;
};

#endif // DEBUGDIALOG_H
