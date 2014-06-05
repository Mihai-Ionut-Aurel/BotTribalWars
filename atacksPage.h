#ifndef ATACKSPAGE_H
#define ATACKSPAGE_H
#include "oneAtack.h"
#include "ui_atacksPage.h"
#include "atackWorker.h"
#include <QDialog>
#include <QObject>
#include <QNetworkAccessManager>
class QDateTime;
class QTimer;
class QFile;
class QThread;
class atacksPage :public QDialog,public Ui::atacksPage
{
	Q_OBJECT
public:
	//adresele urmatoarei,respectiv anterioarei, grupe de atacuri
	atacksPage * previous;
	atacksPage * next;
	oneAtack *currentAtack;
	atacksPage(QNetworkAccessManager *m_nam,QString userAgent,QString groupName,QString Account,QString world,QString nameAtt,QString valueAtt);
	//functie ce seteaza timerul pentru atac;
	void setTimer();
private:
	//threadul in care se lanseaza atacurile
	QThread *t;
	//timerul dupa care se va lansa atacul curent
	QTimer *timerLaunch;
	QTimer *repeat;
	//numele grupei
	QString group;
	//Id-ul actiunii ce se extrage in momentul lasarii atacului
	QString actionid;
	//lumea selectata
	QString world;
	//clasa ce se ocupa de lansarea atacurilor
	atackWorker *worker;
	//Numele atributului necunoscut precum si valoarea sa
	QString unknownName;
	QString unknownValue;
	//numele contului
	QString account;
	//true daca trimitem trupe setate de utilizator sau false daca trimitem trupele selectate automat de program
	bool costumTroops;
	QNetworkAccessManager *nam;
	//functie ce seteaza timerul pentru repeat;
	//void setRepeat();
private slots:
	void costum(bool);
	//sterge ataculul
	void deleteAtack(oneAtack * atac);
	//slot prin care se primeste de la worker situatia atacului
	//acest slot se executa cand atacul a fost lansat cu succes
	void launchSucces();
	//acest slot cand lansarea atacului a dat gres
	void launchFailed();
	//verificare raspuns
	void test();
	
public slots:
	void launchAtack();
	void sendRequest1(QNetworkRequest,QByteArray);
	void sendRequest2(QNetworkRequest,QByteArray);
	void betweenRequests();
	//slot ce se executa cand se doreste stergerea unui atac
	void deleteAtack(QString);
	//slot ce actualizeaza fisierele din grupa
	void refreshTroops();
signals:
	//trimite situatia atacului ferestrei principale
	void atackInfo(QString);
	void atackStatusBar(QString);
	//se logheaza inaintea atacului
	void loginBeforeAtack();
	void sendHtml(QString);
	void workerLaunch(QStringList,QString,QString,QString,int,int);//primul int reprezinta tinta catapultei si al 2-lea de cate ori se repeta atacul
	//semnalul va trimite datele necesare workerului pentru a sterge un atac
	void deleteAtackWorker(QString,QString,QString);
	void deleteGroup(QString);
	//semnal ce trimite idul workerului planificator pentru a crea requestul
	void refreshId(QString);
};
#endif
