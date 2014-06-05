#include "atackWorker.h"
#include <QtXml>
#include <QtNetwork>
atackWorker::atackWorker(QString userAgent,QString world)
{
	agent=userAgent;
	worldSelected=world;
}
void atackWorker::startAtack(QStringList troops,QString id,QString nameAtt,QString valueAtt,int target,int repeatAtack)
{
	repeat=repeatAtack;
	qDebug()<<"Lansare startAtack";
	catapultTarget=target;
	QNetworkRequest request;
	request.setUrl(QUrl(QString("http://ro")+worldSelected+QString(".triburile.ro/game.php?village=")+id+QString("&screen=place&try=confirm")));
	request.setRawHeader( "User-Agent", agent.toLatin1() );
	request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem(QUrl::toPercentEncoding(nameAtt),QUrl::toPercentEncoding(valueAtt));
	if(troops.at(0).toInt())
		params.addEncodedQueryItem("spear",QUrl::toPercentEncoding(troops.at(0)));
	else
		params.addEncodedQueryItem("spear","");

	if(troops.at(1).toInt())
		params.addEncodedQueryItem("sword",QUrl::toPercentEncoding(troops.at(1)));
	else
		params.addEncodedQueryItem("sword","");

	if(troops.at(2).toInt())
		params.addEncodedQueryItem("axe",QUrl::toPercentEncoding(troops.at(2)));
	else
		params.addEncodedQueryItem("axe","");

	if(troops.at(3).toInt())
		params.addEncodedQueryItem("archer",QUrl::toPercentEncoding(troops.at(3)));
	else
		params.addEncodedQueryItem("archer","");

	if(troops.at(4).toInt())
		params.addEncodedQueryItem("spy",QUrl::toPercentEncoding(troops.at(4)));
	else
		params.addEncodedQueryItem("spy","");

	if(troops.at(5).toInt())
		params.addEncodedQueryItem("light",QUrl::toPercentEncoding(troops.at(5)));
	else
		params.addEncodedQueryItem("light","");
	
	if(troops.at(6).toInt())
		params.addEncodedQueryItem("marcher",QUrl::toPercentEncoding(troops.at(6)));
	else
		params.addEncodedQueryItem("marcher","");
	
	if(troops.at(7).toInt())
		params.addEncodedQueryItem("heavy",QUrl::toPercentEncoding(troops.at(7)));
	else
		params.addEncodedQueryItem("heavy","");
	
	if(troops.at(8).toInt())
		params.addEncodedQueryItem("ram",QUrl::toPercentEncoding(troops.at(8)));
	else
		params.addEncodedQueryItem("ram","");
	
	if(troops.at(9).toInt())
		params.addEncodedQueryItem("catapult",QUrl::toPercentEncoding(troops.at(9)));
	else
	{
		params.addEncodedQueryItem("catapult","");
		//nu vom trimite catapulte deci nu avem nevoie de o tinta pentru catapulta
		catapultTarget=-1;
	}
	
	if(troops.at(10).toInt())
		params.addEncodedQueryItem("knight",QUrl::toPercentEncoding(troops.at(10)));
	else
		params.addEncodedQueryItem("knight","");
	
	if(troops.at(11).toInt())
		params.addEncodedQueryItem("snob",QUrl::toPercentEncoding(troops.at(11)));
	else
		params.addEncodedQueryItem("snob","");
	params.addEncodedQueryItem("x",QUrl::toPercentEncoding(troops.at(12).split(QString("|"),QString::SkipEmptyParts).at(0)));
	params.addEncodedQueryItem("y",QUrl::toPercentEncoding(troops.at(12).split(QString("|"),QString::SkipEmptyParts).at(1)));
	params.addEncodedQueryItem("attack","Atac");
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	emit sendRequest1(request,paramBytes);
}
void atackWorker::continueAtack(QString html)
{
	QDomDocument page;
	page.setContent(html);
	QDomNodeList form=page.elementsByTagName(QString("form"));
	if(form.at(0).toElement().attribute(QString("action")).contains(QString("&action=command")))
	{
		if(repeat==1)
			emit launchSucces();
	}
	else
	{
		emit launchFailed();
		return;
	}
	QNetworkRequest request;
	request.setUrl(QUrl(QString("http://ro")+worldSelected+QString(".triburile.ro")+QUrl::fromPercentEncoding(form.at(0).toElement().attribute(QString("action")).toLatin1())));
	request.setRawHeader( "User-Agent", agent.toLatin1() );
	request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QDomNode input=form.at(0).firstChildElement(QString("input"));
	QUrl params;
	while(!input.nextSiblingElement(QString("input")).isNull())
	{
		if(input.toElement().attribute(QString("type"))==QString("hidden"))
			params.addEncodedQueryItem(QUrl::toPercentEncoding(input.toElement().attribute(QString("name"))),
			QUrl::toPercentEncoding(input.toElement().attribute(QString("value"))));
		input=input.nextSiblingElement(QString("input"));
	}

	 switch (catapultTarget)
	{
	case 0:
		params.addEncodedQueryItem("building","main");
		break;
	case 1:
		params.addEncodedQueryItem("building","barracks");
		break;
	case 2:
		params.addEncodedQueryItem("building","stable");
		break;
	case 3:
		params.addEncodedQueryItem("building","garage");
		break;
	case 4:
		params.addEncodedQueryItem("building","church");
		break;
	case 5:
		params.addEncodedQueryItem("building","snob");
		break;
	case 6:
		params.addEncodedQueryItem("building","smith");
		break;
	case 7:
		params.addEncodedQueryItem("building","place");
		break;
	case 8:
		params.addEncodedQueryItem("building","statue");
		break;
	case 9:
		params.addEncodedQueryItem("building","market");
		break;
	case 10:
		params.addEncodedQueryItem("building","wood");
		break;
	case 11:
		params.addEncodedQueryItem("building","stone");
		break;
	case 12:
		params.addEncodedQueryItem("building","iron");
		break;
	case 13:
		params.addEncodedQueryItem("building","farm");
		break;
	case 14:
		params.addEncodedQueryItem("building","storage");
		break;
	case 15:
		params.addEncodedQueryItem("building","wall");
		break;
	};
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	emit sendRequest2(request,paramBytes);
}
void atackWorker::removeAtack(QString acc,QString group ,QString id)
{
	QFile file (QString("Conturi/")+acc+QString("/Lumea")+worldSelected+QString("/Atacuri/")+group+QString(".txt"));
	file.open(QIODevice::ReadOnly|QIODevice::Text);
	QFile file2 (QString("Conturi/")+acc+QString("/Lumea")+worldSelected+QString("/Atacuri/")+group+QString("1.txt"));
	QTextStream in(&file);
	file2.open(QIODevice::WriteOnly|QIODevice::Text);
	QTextStream out(&file2);
	QString word;
	while(!in.atEnd())
	{
		in>>word;
		if(word==id)//eliminam satul
		{
			
			//citim coordonatele satului
			in>>word;
			QFile vill(QString("Conturi/")+acc+QString("/Lumea")+worldSelected+QString("/Atacuri/Sate/")+word.replace("|","-")+QString(".txt"));
			vill.remove();
			for(int i=0;i<7;i++)
			{
				in>>word;
			}
		}
		else
		{
			out<<"\n"<<word;
			for(int i=0;i<8;i++)
			{
				in>>word;
				out<<" "<<word;
			}
		}
	}
	file.remove();
	file2.rename(QString("Conturi/")+acc+QString("/Lumea")+worldSelected+QString("/Atacuri/")+group+QString(".txt"));
	file2.close();
}