#include "oneAtack.h"
#include <QObject>
oneAtack::oneAtack(QWidget *parent): QDialog(parent)
{
	 setupUi(this);
	 //ascundem fereastra cu setarile trupelor
	 groupBoxTroopSettings->hide();
	 gridLayoutMain->setSizeConstraint(QLayout::SetFixedSize);
	 this->comboBoxBuilding->setCurrentIndex(7);
	 this->pushButtonLaunchNow->hide();
	 this->setAttribute(Qt::WA_DeleteOnClose);
	 connect(this->pushButtonDelete,SIGNAL(clicked()),this,SLOT(closeButton()));
	 this->setAttribute(Qt::WA_DeleteOnClose);
	 
}
void oneAtack::closeButton()
{
	emit deleteAtack(this->villageID);
	if(this->nextAtack!=NULL)
	{
		if(this->previousAtack!=NULL) 
			this->previousAtack->nextAtack=this->nextAtack;
		else
			this->nextAtack->previousAtack=NULL;
	}
	else
		this->previousAtack->nextAtack=NULL;
	this->close();
}