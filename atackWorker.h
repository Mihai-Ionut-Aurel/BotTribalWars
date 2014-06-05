#ifndef ATACKWORKER_H
#define ATACKWORKER_H
#include <QObject>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
class atackWorker : public QObject 
{
	Q_OBJECT
public:
	atackWorker(QString userAgent,QString world);
signals:
	//semnal ce trimite requestul ce trebuie transmis pentru atac
	void sendRequest1(QNetworkRequest,QByteArray);
	//se trimite pentru a finaliza lanseara atacului
	void sendRequest2(QNetworkRequest,QByteArray);
	//semnal ce va emite daca atacul a fost lansat cu succes
	void launchSucces();
	//semnal ce se emite daca lansarea atacului a dat gres
	void launchFailed();

public slots:
	void startAtack(QStringList troops,QString id,QString nameAtt,QString valueAtt,int target,int atacks);
	void continueAtack(QString html);
	//acest slot va sterge atacul din fisierul ce contine datele planului precum si fisierul cu trupele
	//primul string ca contine numele contului,al 2-lea numele grupei si al 3-lea id-ul satului ce trebuie sters
	void removeAtack(QString,QString,QString);
	
private:
	QString agent;
	QString worldSelected;
	int catapultTarget;
	int repeat;
};
#endif