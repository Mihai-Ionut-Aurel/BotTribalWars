#include "planWorker.h"
#include <QtXml>
#include <QtNetwork>
#include <QtGui>
#include "matrix.h"
#include "munkres.h"
#include "atackStruct.h"
void planWorker::planAtack(QString atacking,QString defending,QString group,int atackingTroops,int atacksPerTarget,QDateTime arrivalTime)
{
	qDebug()<<"Workerul incepe"
	groupName=group;
	QDir currentDir(QDir::currentPath());
	currentDir.mkpath(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri"));
	QFile atacks(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/")+groupName+QString(".txt"));
	if(atacks.exists())
	{
		emit planAtackStatus(QString("Deja exista o grupa cu numele ")+groupName);
		emit planAtackLog(QString("Deja exista o grupa cu numele \"")+groupName+QString("\".Incercati un alt nume"));
		emit planFinished();
		return;
	}

	currentDir.mkpath(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/Sate"));
	atacks.open(QIODevice::WriteOnly);

	QRegExp rx("\\d?(\\d(\\d)?)\\|\\d?(\\d(\\d)?)");
	
	//cream listele cu coordonatele satelor ce ataca si a celor care sunt tinta
	int pos = 0;
	QStringList atacksList;
	QStringList defendList;
	while ((pos = rx.indexIn(atacking, pos)) != -1) {
		atacksList.append( rx.cap(0));
		pos += rx.matchedLength();
	}
	pos=0;
	while ((pos = rx.indexIn(defending, pos)) != -1) {
		defendList.append( rx.cap(0));
		pos += rx.matchedLength();
	}
	atacksList.removeDuplicates();
	qDebug()<<atacksList.count()<<" sate ce ataca";
	qDebug()<<defendList.count()<<" sate tinta";
	if(atacksList.count()<defendList.count()*atacksPerTarget)
	{
		emit planAtackStatus(QString("Prea multe sate tinta."));
		emit planAtackLog(QString("Prea multe sate tinta.Doriti sa atacati ")+QString::number(defendList.count()*atacksPerTarget)+QString(" cu ")+QString::number(atacksList.count())+QString (" sate"));
		emit planFinished();
		return;
	}
	//cream matricea pe care vom executa operatiile
	//va fi de tipul double deoarece distantele sunt numere reale
	int n=defendList.count()*atacksPerTarget;
	atacksRem=n;
	atacksList=atacksList.mid(0,n);
	Matrix<double> matrix(n,n);
	for(int i=0;i<n;i++)
	{
		int sat=0;//satul tinta ce va fi adaugat in matrice
		for(int j=0;j<n;j++)
		{
			int x=atacksList.at(i).split("|",QString::SkipEmptyParts).at(0).toInt()-defendList.at(sat).split("|",QString::SkipEmptyParts).at(0).toInt();
			int y=atacksList.at(i).split("|",QString::SkipEmptyParts).at(1).toInt()-defendList.at(sat).split("|",QString::SkipEmptyParts).at(1).toInt();
			sat++;
			matrix(i,j)=qSqrt(x*x+y*y);
			if(sat==defendList.count())
				sat=0;
		}
	}
	//apelam functia ce simuleaza algoritmul Munken
	Munkres m;
	m.solve(matrix);
	//cream un sir de numere 
	//cream o lista cu atacuri in care vom adauga satele ce ataca precum si tintele
	QList<atack> lista;
	for ( int row = 0 ; row < n ; row++ )
	{
		int sat=0;
		for ( int col = 0 ; col < n ; col++ )
		{
			//daca elementul din matrice are valoare 0 il adaugan in lista
			if(matrix(row,col)==0)
			{
				atack t;
				t.sender=&atacksList.at(row);
				t.target=&defendList.at(sat);
				lista.append(t);
			}
			//resetam counterul
			sat++;
			if(sat==defendList.count())
				sat=0;
		}
	}
	qSort(lista);
	//am sortat lista acum introducem in fisier datele coresponzatoare
	//deschidem visierul cu setarile lumii si le salvam
	QFile setarileLumii(QString("Setari/Setarile-lumii/")+world+QString(".txt"));
	if(!setarileLumii.exists())
	{
		emit planAtackStatus(QString("Nu exista fisierul cu setarile lumii"));
		return;
	}

	setarileLumii.open(QIODevice::ReadOnly|QIODevice::Text);
	QTextStream stream(&setarileLumii);
	float troopSpeed,worldSpeed;
	stream>>worldSpeed;
	stream>>troopSpeed;
	//inchidem fisierul
	setarileLumii.close();
	//setam streamul pe fisierul grupei
	stream.setDevice(&atacks);
	int troopPerSq;
	//setam viteza trupei celei mai lente
	switch (atackingTroops)
	{
	case 0:
		qDebug()<<" cazul cu nobilii";
		troopPerSq=35;
		break;
	case 1:
		troopPerSq=30;
		break;
	case 2:
		troopPerSq=22;
		break;
	case 3:
		troopPerSq=18;
		break;
	case 4:
		troopPerSq=11;
		break;
	case 5:
		troopPerSq=10;
		break;
	case 6:
		troopPerSq=9;
		break;
	default:
		//punem viteza celei mai lente unitati posibile
		troopPerSq=35;
		break;
	}


	QFile debug("debug.txt");
	

	for(int i=0;i<lista.count();i++)
	{
		//cream data la care trebuie lansat atacul
		qint32 sec;
		int x1=lista.at(i).target->split("|",QString::SkipEmptyParts).at(0).toInt();
		int y1=lista.at(i).target->split("|",QString::SkipEmptyParts).at(1).toInt();
		int x2=lista.at(i).sender->split("|",QString::SkipEmptyParts).at(0).toInt();
		int y2=lista.at(i).sender->split("|",QString::SkipEmptyParts).at(1).toInt();
		
		//la sfarsit inmultim cu 60 pentru a obtine numarul de secunde
		sec=qSqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))*troopPerSq/worldSpeed/troopSpeed*60;
		sec=sec*(-1);
		qDebug()<<sec<<" secunde ";
		
		QDateTime launchTime=arrivalTime.addSecs(sec);


		//deschidem fisierul cu lista satelor contului
		QFile villagesList(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Lista-sate.txt"));
		villagesList.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream in(&villagesList);
		bool gasit=false;
		//verificam daca acest sat se afla in lista
		QString coordonates,id;
		while(!in.atEnd()&&gasit==false)
		{
			
			in>>coordonates;
			in>>id;
			if(*lista.at(i).sender==coordonates)
			{

				gasit=true;
				
				
				QFile sat(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/")+QString("Sate/")+coordonates.replace("|","-")+QString(".txt"));
				if(!sat.exists())
				{
					stream<<"\n"<<id<<" "<<*lista.at(i).sender<<" "<<*lista.at(i).target<<" "<<launchTime.date().year();
					stream<<" "<<launchTime.date().month()<<" "<<launchTime.date().day()<<" "<<launchTime.time().hour()<<" "<<launchTime.time().minute()<<" "<<launchTime.time().second();
					//construim fisierul cu trupele satului
					QNetworkRequest request;
					//setam url-ul cererii
					request.setUrl(QUrl( QString("http://ro"+world+QString(".triburile.ro/game.php?village=")+id+QString("&screen=place"))));
					//seteam headerul cu user agentul
					request.setRawHeader( "User-Agent", userAgent.toLatin1() );
					//setam tipul continutului
					request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
					emit villageRequest(request);
					qDebug()<<request.url().toString();
				}
				else
				{
					atacksRem--;//am gasit un atac ce deja apartine unei alte grupe deci il eliminam
					sat.open(QIODevice::ReadOnly | QIODevice::Text);
					QTextStream satIn(&sat);
					satIn>>id;
					sat.close();
					emit planAtackLog(QString("Satul ")+*lista.at(i).sender+QString(" este folosit deja in grupa \"")+id+QString("\""));
				}
			}
		}
		villagesList.close();
		if(gasit==false)
		{
			
			atacksRem--;//eliminam acest atac...satul nu se afla in lista de sate
			//emitem semnalul
			emit planAtackLog(QString("Satul ")+*lista.at(i).sender+QString(" nu se afla in lista satelor"));
		}
	}

	
		emit planAtackLog(QString("Planul grupei \"")+groupName+("\" a fost creat"));
		emit planAtackStatus(QString("Planul grupei \"")+groupName+("\" a fost creat"));

}

void planWorker::villageTroops(QString html)
{
	QDomDocument page;
	page.setContent(html);
	//aflam coordonatele satului curent
	QDomNodeList b=page.elementsByTagName(QString("b"));
	qDebug()<<b.count();
	QString coordonate=b.at(0).toElement().text();
	qDebug()<<coordonate;
	coordonate.remove(0,1).remove(7,5);
	qDebug()<<coordonate;
	QFile file(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/")+QString("Sate/")+coordonate.replace("|","-")+QString(".txt"));
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
	out<<groupName;
	QDomNodeList tr=page.elementsByTagName(QString("a"));
	for(int i=0;i<tr.count();i++)
	{
		if(tr.at(i).toElement().attribute(QString("class"))==QString("unit_link"))
		{
			out<<" "<<tr.at(i).nextSibling().nextSibling().toElement().text().remove("(").remove(")");
		}
	}
	
	file.close();
	atacksRem--;
	if(atacksRem==0)
	{
		emit planFinished();
		emit planAtackFinish(groupName);
	}
}

void planWorker::deleteGroup(QString group)
{
	//deschidem fisierul ce contine toate grupele
	QFile accGroups(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Grupe.txt"));
	accGroups.open(QIODevice::ReadWrite|QIODevice::Text);
	//citim tot continutul si apoi eliminam grupa curenta
	QTextStream accIO(&accGroups);
	QStringList continut;
	QString one;
	//citim continutul fisierului
	while(!accIO.atEnd())
	{
		accIO>>one;
		if(one!=group)
		{
			continut.append(one);
		}
	}
	//am eliminat grupa ce trebuie stearsa,acum golim fisierul si inseram continutul
	accGroups.resize(0);
	for(int i=0;i<continut.count();i++)
		accIO<<"\n"<<continut.at(i);
	accGroups.close();
	//deschidem fisierul ce contine atacurile grupei
	QFile groupOrg(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/")+group+QString(".txt"));
	groupOrg.open(QIODevice::ReadOnly|QIODevice::Text);
	accIO.setDevice(&groupOrg);
	while(!accIO.atEnd())
	{
		accIO>>one;//primul e id-ul
		accIO>>one;//acesta este satul atacat si satul ce contine trupele
		one.replace("|","-");
		QFile ville(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/Sate/")+one+QString(".txt"));
		ville.remove();
		for(int i=0;i<7;i++)
			accIO>>one;
	}
	groupOrg.remove();
}
//slot ce actualizeaza fisierul cu trupe
void planWorker::refreshTroops(QString html)
{
	QDomDocument page;
	page.setContent(html);
	//aflam coordonatele satului curent
	QDomNodeList b=page.elementsByTagName(QString("b"));
	qDebug()<<b.count();
	QString coordonate=b.at(0).toElement().text();
	qDebug()<<coordonate;
	coordonate.remove(0,1).remove(7,5);
	qDebug()<<coordonate;
	QFile file(QString("Conturi/")+accName+QString("/Lumea")+world+QString("/Atacuri/")+QString("Sate/")+coordonate.replace("|","-")+QString(".txt"));
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
	out<<"temporar";
	QDomNodeList tr=page.elementsByTagName(QString("a"));
	for(int i=0;i<tr.count();i++)
	{
		if(tr.at(i).toElement().attribute(QString("class"))==QString("unit_link"))
		{
			out<<" "<<tr.at(i).nextSibling().nextSibling().toElement().text().remove("(").remove(")");
		}
	}
	
	file.close();
}
void planWorker::buildRequest(QString id)
{
	QNetworkRequest request;
	//setam url-ul cererii
	request.setUrl(QUrl( QString("http://ro"+world+QString(".triburile.ro/game.php?village=")+id+QString("&screen=place"))));
	//seteam headerul cu user agentul
	request.setRawHeader( "User-Agent", userAgent.toLatin1() );
	//setam tipul continutului
	request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	emit refreshRequest(request);
}