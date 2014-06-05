#ifndef DIALOGPLANNEWATACK_H
#define DIALOGPLANNEWATACK_H
#include <QDialog>
#include <QNetworkAccessManager>
#include <QString>
#include "ui_dialogPlanNewAtack.h"
#include "planWorker.h"
class dialogPlanNewAtack :public QDialog,public Ui::dialogPlanNewAtack
{
	Q_OBJECT
public:
	explicit dialogPlanNewAtack(QNetworkAccessManager *nam,QWidget *parent=0);
	planWorker *worker;
signals:
	//semnal ce se emite pentru a face logarea inaintea planificarii
	void login();
	//semnal ce trimite datele necesare workerului
	void workerStartPlan(QString ,QString ,QString,int ,int ,QDateTime);
	//semnal ce trimite codul pietii centrale a satului
	void sendVillage(QString);
public slots:
	//slot cu ajutorul caruia se va seta userAgentul,lumea folosita si numele folosit la logare
	void setUserAgent(QString userAgent,QString world,QString accName);
	//slot ce incepe planuirea atacului dupa logare
	void planAtack();
	//slot ce primeste requestul
	void sendRequest(QNetworkRequest request);
	//slotul se executa atunci cand s-a terminat lansarea atacului
	void planningFinished();
private slots:
	//slot ce primeste raspunsul la cerere trimisa pentru piata centrala
	void receiveReply();
private:
	
	QThread *workerThread;
	QString htmlTwMentor;
	QNetworkAccessManager * m_nam;
	QString worldSelected;
	QString userAgent;
	QString accountName;
};
#endif