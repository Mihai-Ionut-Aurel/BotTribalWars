#include <QtNetwork>
#include <QtGui>
#include <QRegExp>
#include <QString>
#include "dialogplannewatack.h"
#include <QtXml>
dialogPlanNewAtack::dialogPlanNewAtack(QNetworkAccessManager *nam,QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	m_nam= nam;
	worker=new planWorker;
	workerThread=new QThread;
	worker->moveToThread(workerThread);

	workerThread->start();
	this->dateTimeEditAtackLanding->setDateTime(QDateTime::currentDateTime());
	//conectam semnalele si sloturile
	connect(this,SIGNAL(workerStartPlan(QString ,QString ,QString,int ,int ,QDateTime)),worker,SLOT(planAtack(QString ,QString ,QString,int ,int ,QDateTime)),Qt::QueuedConnection);
	connect(worker,SIGNAL(villageRequest(QNetworkRequest)),this,SLOT(sendRequest(QNetworkRequest )),Qt::QueuedConnection);
	connect(this,SIGNAL(sendVillage(QString)),worker,SLOT(villageTroops(QString)),Qt::QueuedConnection);
	connect(worker,SIGNAL(planFinished()),this,SLOT(planningFinished()),Qt::QueuedConnection);
}
void dialogPlanNewAtack::planAtack()
{
	this->lineEditGroupName->setText(this->lineEditGroupName->text().replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ),QString( "_" ) ));
	//emitem semnalul in cu care incercam sa planuim un nou atac
	//dezactivam butonul de planuire a atacului pana cand acest lucru nu va avea loc
	this->pushButtonStartPlanningAtack->setEnabled(false);
	qDebug()<<"emiterea semnalului de planuire atac la worker";
	emit workerStartPlan(this->plainTextEditYourVillages->toPlainText(),this->plainTextEditEnemyVillages->toPlainText(),this->lineEditGroupName->text(),this->comboBoxSlowestUnit->currentIndex(),this->spinBoxAtacksPerVill->value(),this->dateTimeEditAtackLanding->dateTime());
}
void dialogPlanNewAtack::setUserAgent(QString userAgent,QString world,QString accName)
{
	this->userAgent=userAgent;
	worldSelected=world;
	accountName=accName;
	worker->world=world;
	worker->accName=accName;
	worker->userAgent=userAgent;
}
void dialogPlanNewAtack::sendRequest(QNetworkRequest request)
{
	QNetworkReply *reply=m_nam->get(request);
	connect(reply,SIGNAL(finished()),this,SLOT(receiveReply()));
}
void dialogPlanNewAtack::receiveReply()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
/*
	QFile villagesList("planAtac.txt");
	villagesList.open(QIODevice::Append| QIODevice::Text);
	QTextStream out(&villagesList);
	QList<QByteArray> headere=reply->rawHeaderList();
	for(int i=0;i<headere.count();i++)
	{
		out<<headere.at(i)<<": "<<reply->rawHeader(headere.at(i))<<"\n";
	}
	out<<reply->readAll();
*/
	emit sendVillage(QString(reply->readAll()));
}
void dialogPlanNewAtack::planningFinished()
{
	this->pushButtonStartPlanningAtack->setEnabled(true);
}