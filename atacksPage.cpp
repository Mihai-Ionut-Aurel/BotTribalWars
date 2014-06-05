#include "atacksPage.h"
#include <QtGui>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QObject>
#include <QtNetwork>
atacksPage::atacksPage(QNetworkAccessManager *m_nam,QString userAgent,QString groupName,QString Account,QString world,QString nameAtt,QString valueAtt)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);
	nam=m_nam;
	account=Account;
	group=groupName;
	//cream timerul dupa care se lanseaza atacul
	timerLaunch=new QTimer(this);
	connect(timerLaunch,SIGNAL(timeout()),this,SLOT(launchAtack()));
	//conectam semnalul pentru trupe costum
	connect(this->pushButtonOk,SIGNAL(clicked(bool)),this,SLOT(costum(bool)));
	timerLaunch->setSingleShot(true);
	repeat=new QTimer(this);
	connect(repeat,SIGNAL(timeout()),this,SLOT(launchAtack()));
	repeat->setSingleShot(true);
	costumTroops=false;
	this->world=world;
	unknownName=nameAtt;
	unknownValue=valueAtt;
	//worker va trimite atacurile
	worker=new atackWorker(userAgent,world);
	//cream threadul ce se va ocupa de lansarea atacurilor
	t = new QThread();
	t->start();
	worker->moveToThread(t);
	
	//conectam sloturile
	connect(this,SIGNAL(workerLaunch(QStringList,QString,QString,QString,int,int)),worker,SLOT(startAtack(QStringList,QString,QString,QString,int,int)),Qt::QueuedConnection);
	connect(this,SIGNAL(sendHtml(QString)),worker,SLOT(continueAtack(QString)),Qt::QueuedConnection);
	connect(worker,SIGNAL(launchSucces()),this,SLOT(launchSucces()),Qt::QueuedConnection);
	connect(worker,SIGNAL(launchFailed()),this,SLOT(launchFailed()),Qt::QueuedConnection);
	connect(worker,SIGNAL(sendRequest1(QNetworkRequest,QByteArray)),this,SLOT(sendRequest1(QNetworkRequest,QByteArray)),Qt::QueuedConnection);
	connect(worker,SIGNAL(sendRequest2(QNetworkRequest,QByteArray)),this,SLOT(sendRequest2(QNetworkRequest,QByteArray)),Qt::QueuedConnection);
	connect(this,SIGNAL(deleteAtackWorker(QString,QString,QString)),worker,SLOT(removeAtack(QString,QString,QString)),Qt::QueuedConnection);
	this->setWindowTitle(groupName);
	currentAtack=NULL;
	//deschidem fisierul in mod read only
	QFile atacks(QString("Conturi/")+Account+QString("/Lumea")+world+QString("/Atacuri/")+groupName+QString(".txt"));
	atacks.open(QIODevice::ReadOnly);
	
	QTextStream in(&atacks);
	QString read,second,third;
	oneAtack *lastAtack;
	int atacuri=0;
	if(in.atEnd())
	{
		emit atackInfo(QString("Au fost lansate toate atacurile din grupa \"")+group+QString("\""));
		emit deleteGroup(group);
		return;
	}
	while(!in.atEnd())
	{
		atacuri++;
		oneAtack *current = new oneAtack (this);
		in>>read;
		current->villageID=read;
		//deschidem fisierul ce contine trupele
		in>>read;
		QFile file(QString("Conturi/")+Account+QString("/Lumea")+world+QString("/Atacuri/Sate/")+read.replace(QString("|"),QString("-"))+QString(".txt"));
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream inTroops(&file);
		read.replace(QString("-"),QString("|"));
		current->labelAtackingVillage->setText(read);
		inTroops>>read;//pe prima linie fisierul va contine numele grupei din care satul face parte
		//am terminat de deschis fisierul
		in>>read;
		current->labelAtackedVillage->setText(read);
		in>>read;
		in>>second;
		in>>third;
		
		QDate date;
		date.setDate(read.toInt(),second.toInt(),third.toInt());
		
		current->dateTimeEditLaunchTime->setDate(date);
		in>>read;
		in>>second;
		in>>third;
		QTime time;
		time.setHMS(read.toInt(),second.toInt(),third.toInt());

		current->dateTimeEditLaunchTime->setTime(time);
		//citim trupele
		inTroops>>read;
		current->spinBoxSpear->setMaximum(read.toInt());
		inTroops>>read;
		current->spinBoxSword->setMaximum(read.toInt());
		inTroops>>read;
		current->spinBoxAxe->setMaximum(read.toInt());
		current->spinBoxAxe->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxArcher->setMaximum(read.toInt());
		inTroops>>read;
		current->spinBoxSpy->setMaximum(read.toInt());
		current->spinBoxSpy->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxLight->setMaximum(read.toInt());
		current->spinBoxLight->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxMarcher->setMaximum(read.toInt());
		current->spinBoxMarcher->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxHeavy->setMaximum(read.toInt());
		inTroops>>read;
		current->spinBoxRam->setMaximum(read.toInt());
		current->spinBoxRam->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxCatapult->setMaximum(read.toInt());
		current->spinBoxCatapult->setValue(read.toInt());
		inTroops>>read;
		current->spinBoxKnight->setMaximum(read.toInt());
		inTroops>>read;
		file.close();
		current->spinBoxSnob->setMaximum(read.toInt());
		
		current->nextAtack=NULL;
		current->previousAtack=NULL;
		this->verticalLayoutFrame->addWidget(current);
		//conectam butonul de lansarea a trupelor la semnalul de logare inaintea atacului
		connect(current->pushButtonLaunchNow,SIGNAL(clicked()),this,SLOT(launchAtack()));
		//stergerea atacului
		connect(current,SIGNAL(deleteAtack(QString)),this,SLOT(deleteAtack(QString)));
		//inchidem fisierele
		file.close();
		if(currentAtack==NULL)
		{
			currentAtack=current;
		}
		else
		{
			if(currentAtack->nextAtack==NULL)
			{
				current->previousAtack=currentAtack;
				currentAtack->nextAtack=current;
				lastAtack=current;
			}
			else
			{
				current->previousAtack=lastAtack;
				lastAtack->nextAtack=current;
				lastAtack=current;
			}
		}
	}
	qDebug()<<atacuri<<" atacuri";
	//afisam butonul de lansare a atacului curent
	currentAtack->pushButtonLaunchNow->show();
	//ascundem butonul de stergere
	currentAtack->pushButtonDelete->hide();
	//inchidem fisierul
	atacks.close();

	
}
void atacksPage::deleteAtack(oneAtack * atac)
{
	if(currentAtack==atac)
	{
		currentAtack=currentAtack->nextAtack;
		currentAtack->previousAtack=NULL;
		delete atac;
		atac=NULL;
	}
	else
	{
		oneAtack*aux=atac->nextAtack;
		aux->previousAtack=atac->previousAtack;
		delete atac;
	}
}
void atacksPage::launchAtack()
{
	emit atackStatusBar(QString("Lansare atac "));
	if(costumTroops)
	{
		if(currentAtack->spinBoxSpear->maximum()<this->spinBoxSpear->value())
			currentAtack->spinBoxSpear->setValue(currentAtack->spinBoxSpear->maximum());
		else
			currentAtack->spinBoxSpear->setValue(this->spinBoxSpear->value());
		if(currentAtack->spinBoxSword->maximum()<this->spinBoxSword->value())
			currentAtack->spinBoxSword->setValue(currentAtack->spinBoxSword->maximum());
		else
			currentAtack->spinBoxSword->setValue(this->spinBoxSword->value());

		if(currentAtack->spinBoxAxe->maximum()<this->spinBoxAxe->value())
			currentAtack->spinBoxAxe->setValue(currentAtack->spinBoxAxe->maximum());
		else
			currentAtack->spinBoxAxe->setValue(this->spinBoxAxe->value());

		if(currentAtack->spinBoxArcher->maximum()<this->spinBoxArcher->value())
			currentAtack->spinBoxArcher->setValue(currentAtack->spinBoxArcher->maximum());
		else
			currentAtack->spinBoxArcher->setValue(this->spinBoxArcher->value());

		if(currentAtack->spinBoxSpy->maximum()<this->spinBoxSpy->value())
			currentAtack->spinBoxSpy->setValue(currentAtack->spinBoxSpy->maximum());
		else
			currentAtack->spinBoxSpy->setValue(this->spinBoxSpy->value());

		if(currentAtack->spinBoxLight->maximum()<this->spinBoxLight->value())
			currentAtack->spinBoxLight->setValue(currentAtack->spinBoxLight->maximum());
		else
			currentAtack->spinBoxLight->setValue(this->spinBoxLight->value());

		if(currentAtack->spinBoxMarcher->maximum()<this->spinBoxMarcher->value())
			currentAtack->spinBoxMarcher->setValue(currentAtack->spinBoxMarcher->maximum());
		else
			currentAtack->spinBoxMarcher->setValue(this->spinBoxMarcher->value());

		if(currentAtack->spinBoxHeavy->maximum()<this->spinBoxHeavy->value())
			currentAtack->spinBoxHeavy->setValue(currentAtack->spinBoxHeavy->maximum());
		else
			currentAtack->spinBoxHeavy->setValue(this->spinBoxHeavy->value());
	
		if(currentAtack->spinBoxRam->maximum()<this->spinBoxRam->value())
			currentAtack->spinBoxRam->setValue(currentAtack->spinBoxRam->maximum());
		else
			currentAtack->spinBoxRam->setValue(this->spinBoxRam->value());
		if(currentAtack->spinBoxCatapult->maximum()<this->spinBoxCatapult->value())
			currentAtack->spinBoxCatapult->setValue(currentAtack->spinBoxCatapult->maximum());
		else
			currentAtack->spinBoxCatapult->setValue(this->spinBoxCatapult->value());

		if(currentAtack->spinBoxSnob->maximum()<this->spinBoxSnob->value())
			currentAtack->spinBoxSnob->setValue(currentAtack->spinBoxSnob->maximum());
		else
			currentAtack->spinBoxSnob->setValue(this->spinBoxSnob->value());
		if(currentAtack->spinBoxKnight->maximum()<this->spinBoxKnight->value())
			currentAtack->spinBoxKnight->setValue(currentAtack->spinBoxKnight->maximum());
		else
			currentAtack->spinBoxKnight->setValue(this->spinBoxKnight->value());
		this->currentAtack->comboBoxBuilding->setCurrentIndex(this->comboBoxBuilding_2->currentIndex());
	}

	QStringList trupe;
	trupe.append(this->currentAtack->spinBoxSpear->text());
	trupe.append(this->currentAtack->spinBoxSword->text());
	trupe.append(this->currentAtack->spinBoxAxe->text());
	trupe.append(this->currentAtack->spinBoxArcher->text());
	trupe.append(this->currentAtack->spinBoxSpy->text());
	trupe.append(this->currentAtack->spinBoxLight->text());
	trupe.append(this->currentAtack->spinBoxMarcher->text());
	trupe.append(this->currentAtack->spinBoxHeavy->text());
	trupe.append(this->currentAtack->spinBoxRam->text());
	trupe.append(this->currentAtack->spinBoxCatapult->text());
	trupe.append(this->currentAtack->spinBoxKnight->text());
	trupe.append(this->currentAtack->spinBoxSnob->text());
	trupe.append(this->currentAtack->labelAtackedVillage->text());
	emit workerLaunch(trupe,this->currentAtack->villageID,unknownName,unknownValue,this->currentAtack->comboBoxBuilding->currentIndex(),this->currentAtack->spinBoxAtacks->value());
}
void atacksPage::costum(bool checked)
{
	if(checked)
	{
		this->pushButtonOk->setText(QString("ON"));
		costumTroops=true;
	}
	else
	{
		this->pushButtonOk->setText(QString("OFF"));
		costumTroops=false;
	}
}
void atacksPage::sendRequest1(QNetworkRequest request,QByteArray paramBytes)
{
		QNetworkReply *reply=nam->post(request,paramBytes);
		connect(reply,SIGNAL(finished()),this,SLOT(betweenRequests()));
}
void atacksPage::betweenRequests()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	if(reply->rawHeader(QByteArray("Location"))==QByteArray("http://www.triburile.ro/sid_wrong.php"))
	{
		emit loginBeforeAtack();
		launchAtack();
		return;
	}
	QString html(reply->readAll());
	emit sendHtml(html);
}
void atacksPage::sendRequest2(QNetworkRequest request,QByteArray paramBytes)
{
	QNetworkReply *reply=nam->post(request,paramBytes);
	connect(reply,SIGNAL(finished()),this,SLOT(test()));
	if(currentAtack->spinBoxAtacks->value()>1)
	{
		repeat->setInterval(50);
		repeat->start();
	}
	currentAtack->spinBoxAtacks->setValue(currentAtack->spinBoxAtacks->value()-1);
}
void atacksPage::test()
{
	
	/*QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	
	QList<QByteArray> lista=reply->rawHeaderList();
	for(int i=0;i<lista.count();i++)
	{
		qDebug()<<lista.at(i)<<" "<<reply->rawHeader(lista.at(i));
	}
	QFile htmlf("Html.html");
	htmlf.open(QIODevice::WriteOnly|QIODevice::Text);
	QTextStream outh(&htmlf);
	outh<<reply->readAll();
	htmlf.close();*/
	
}
void atacksPage::launchSucces()
{
	oneAtack *aux = currentAtack;
	emit atackInfo(group+QString(" :Lansarea atacului de pe ")+aux->labelAtackingVillage->text()
		+QString (" spre ")+aux->labelAtackedVillage->text()+QString(" a avut loc cu succes"));
	emit atackStatusBar(QString("Lansarea atacului ")+QString("a avut loc cu succes"));
	emit deleteAtackWorker(account,group,aux->villageID);
	if(currentAtack->nextAtack!=NULL)
	{
		currentAtack=currentAtack->nextAtack;
		currentAtack->pushButtonLaunchNow->show();
		currentAtack->pushButtonDelete->hide();
		//setam timerul pentru urmatorul atac
		setTimer();
		aux->close();
	}
	else
	{
		emit atackInfo(QString("Au fost lansate toate atacurile din grupa \"")+group+QString("\""));
		emit deleteGroup(group);
		this->close();
	}
	
}
void atacksPage::launchFailed()
{
	emit atackInfo(group+QString(" :Lansarea atacului de pe ")+this->currentAtack->labelAtackingVillage->text()
		+QString (" spre ")+this->currentAtack->labelAtackedVillage->text()+QString(" a dat gres"));
	emit atackStatusBar(QString("Lansarea atacului ")+QString("a dat gres."));
	oneAtack *aux=currentAtack;
	emit deleteAtackWorker(account,group,aux->villageID);
	if(currentAtack->nextAtack!=NULL)
	{
		currentAtack=currentAtack->nextAtack;
		currentAtack->pushButtonLaunchNow->show();
		currentAtack->pushButtonDelete->hide();
		setTimer();
		aux->close();
	}
	else
	{
		emit atackInfo(QString("Au fost lansate toate atacurile din grupa \"")+group+QString("\""));
		emit deleteGroup(group);
		this->close();
	}
	
}
void atacksPage::setTimer()
{
	if((QDateTime::currentDateTime().secsTo(this->currentAtack->dateTimeEditLaunchTime->dateTime())+10)<0)
	{
		int r= QMessageBox::warning(this, tr("Optiuni atac"),
			QString("Atacul trebuia lansat. \n")+this->currentAtack->dateTimeEditLaunchTime->dateTime().toString(QString("hh:mm:ss dd.MM"))+QString("\n")+
                           QString("Yes=Lansare acum \n")+QString("No=Stergere atac"),
                        QMessageBox::Yes | QMessageBox::No);
		if(r==QMessageBox::Yes)
		{
			launchAtack();
		}
		else
		{
			launchFailed();
		}
		return;
	}
	qDebug()<<currentAtack->labelAtackingVillage->text()<<" Timer ";
	timerLaunch->setInterval(QDateTime::currentDateTime().secsTo(this->currentAtack->dateTimeEditLaunchTime->dateTime())*1000-2000);
	timerLaunch->start();
}
void atacksPage::deleteAtack(QString id)
{
	emit deleteAtackWorker(account,group,id);
}
void atacksPage::refreshTroops()
{
	emit atackInfo("Actualizare trupe");
	oneAtack *aux=currentAtack;
	while(aux->nextAtack)
	{
		emit refreshId(aux->villageID);
		aux=aux->nextAtack;
	}
}