#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui_mainWindow.h"
#include <QMainWindow>
#include <QObject>
#include <QNetworkAccessManager>
#include "dialogPlanNewAtack.h"
#include "dialogLogin.h"
#include "statusbar.h"
#include <QDateTime>
class mainWindow: public QMainWindow,Ui::MainWindow
{
	Q_OBJECT	
private://obiectele de care fereastra principala are nevoie pentru functionare
	dialogPlanNewAtack *fereastraPlanuireAtac;
	dialogLogin *loginGUI;
	appStatusBar *baraStatus;
	QString html;//html privire generala
	//fiecare cont are in form-ul de trimitere a trupelor un <input type="hidden" name="xxxxxx" value="xxxxx"> ce trebuie trimise ca request
	QString inputName;
	QString inputValue;
	//authenticator
	//extrage parola(SID) din raspunsul serverului
	void extractPassword(QString & password);
	QNetworkAccessManager *nam; 
private slots:
	void setDate();
	void getAttribute();
	//trimite cererea pentru a verificat daca userul detine o licenta
	void getLicense();
	//primeste raspunsul cererii
	void receiveLicense();
	//de la authenticator
	//slotul ce construieste codul sursa al paginii 
	void buildHtml();
	//cerere http
	void homepagesc();
	void action();
	void getSid();
	//logarea inainte de un atac sau de planuire a unui atac se face in 2 pasi...aceste requesturi fiind al 2-lea pas
	void atackAction();
	void planAction();
	void afterRefreshRequest();
public:
	mainWindow();
public slots:
	//acest slot se va executa atunci cand a avut loc o operatiune
	//el va pasa textul primit logoului intr-un format: data operatiune
	void outputLog(QString message);
	//se va executa cand programul executa o anumita operatiune ca lansarea unui atac
	void setStatus(QString operatiune);
	//se executa cand logarea a avut cu succes
	//slotul va salva codul html al privirii generala si va genera ouputul corespunzator pentru bara de status
	void loginSucces();
	//din authenticator
	void postSid();
	//slot cu care se incepe logarea inainte de un atac
	void beforeAtackLogin();
	//slot cu care se incepe logarea inainte de planuirea unui atac
	void beforePlanningLogin();
	//slot ce se va executa pentru a crea grupa de atacuri
	void newAtackGroup(QString);//stringul va contine numele grupei
	//slot ce va sterge grupa
	void deleteGroupAction();
	
	void refreshRequest(QNetworkRequest);
signals:
	void refreshHtml(QString);
	//se emite cand se poate planifica atacul
	void startPlanning();
	//semnal ce se emite cand se poate lansa atacul(dupa logare)
	void launchAtack();
	//de la authenticator
	//semnal ce trimite user agentul daca logarea a avut loc cu succes planificatorului de atacuri
	void sendUserAgent(QString,QString,QString);
	//semnal ce se emite pentru a elimina o grupa
	void deleteGroup(QString);//stringul va contine numele grupei
};
#endif