/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "DebugDialog.h"
#include "ui_DebugDialog.h"
#include "DayzServerIp.h"

DebugDialog::DebugDialog(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::DebugDialog)
{
   ui->setupUi(this);
}

DebugDialog::~DebugDialog()
{
   delete ui;
}

void DebugDialog::on_pbSubmit_clicked()
{
    DayzServerIp* dayzServerIp = (DayzServerIp*)parentWidget();
    dayzServerIp->updateRemoteInfo(ui->leMessage->text(), true);
}
