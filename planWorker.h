#ifndef PLANWORKER_H
#define PLANWORKER_H
#include <QNetworkRequest>
#include <QObject>
#include <QDateTime>
class planWorker: public QObject 
{
	Q_OBJECT
public:
	//numele grupei
	QString groupName;
	//numele contului
	QString accName;
	//lumea selectata
	QString world;
	//userAgentul
	QString userAgent;
public slots:
	//primeste textul cu satele tinta si satele folosite pentru atac si numele grupei pentru a crea fisierul cu planul grupei
	void planAtack(QString atacking,QString defending,QString group,int atackingTroops,int atacksPerTarget,QDateTime arrivalTime);
	//sterge grupa curent apoi emite semnalul de terminare
	void deleteGroup(QString);
	void villageTroops(QString html);
	void refreshTroops(QString html);
	void buildRequest(QString id);
signals:
	void villageRequest(QNetworkRequest);
	void planAtackLog(QString);
	void planAtackStatus(QString);
	//se trimite ferestrei de planuire a atacului
	void planFinished();
	//trimit numele grupei pentru a crea o noua grupa
	void planAtackFinish(QString);
	void refreshRequest(QNetworkRequest );
private:
	//vom numara atacurile ramase pentru a sti cand sa emitem semnalul de creare a noii grupe
	int atacksRem;
};
#endif //PLANATACKWORKER_H