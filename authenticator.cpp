#include "authenticator.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QUrl>
#include <QByteArray>
#include "mainwindow.h"
#include "dialoglogin.h"
Authenticator::Authenticator(QNetworkAccessManager* nam,QObject* parent ):
QObject( parent )
{
	this->setParent(parent);
	//initializam managerul de acces
	m_nam= nam;
}
void Authenticator::postSid(QString userName,QString password,QString userAgent,QString world)
{
	emit loginStatus(QString("Inceperea operatiunii de logare"));
	//ne asiguram ca string este gol pentru a putea salva in el codul sursa
	html.remove(0,html.count());
	m_world=world;
	m_password=password;
	m_userName=userName;
	m_userAgent=userAgent;
	QNetworkRequest request;
	//setam url-ul cererii
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_list=1&show_server_selection=1"));
	//seteam headerul cu user agentul
	request.setRawHeader( "User-Agent", m_userAgent.toLatin1() );
	//setam tipul continutului
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	//codam userul si parola pentru a fi transmise site-ului
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding(m_userName));
    params.addEncodedQueryItem( "password", QUrl::toPercentEncoding(m_password ) );
	params.addEncodedQueryItem( "clear", QUrl::toPercentEncoding( "true" ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );//un bug.../ nu este codat ca /
	//trimitem cererea siteului
	QNetworkReply *reply=m_nam->post(request,paramBytes);
	connect( reply, SIGNAL( finished() ),
             SLOT(homepagesc()));
}
void Authenticator::homepagesc()
{	
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	//citim raspunsul siteului
	QByteArray info2 =reply->readAll();
	//extragem parola din raspuns
	extractPassword(info2);
	//verificam daca parola este corecta
	if(m_password==QString("error:Parolu0103 incorectu010"))
	{
		emit loginStatus(QString("Parola incorecta"));
		return;
	}
	if(m_password==QString("error:Cont inexisten"))
	{
		emit loginStatus(QString("User inexistent"));
		return;
	}
	//daca parola si userul sunt  corecte trimitem si user Agentul planificatorului de atacuri
	emit sendUserAgent(m_userAgent,m_world,m_userName);
	//incepem logarea
	QNetworkRequest request;
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_ro"+m_world));
    request.setRawHeader( "User-Agent",m_userAgent.toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding( m_userName ) );
    params.addEncodedQueryItem( "password", QUrl::toPercentEncoding( m_password ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	QNetworkReply *reply2=m_nam->post(request,paramBytes);
	connect( reply2, SIGNAL( finished() ),
             SLOT(action()));
}
void Authenticator::beforeAtackLogin()
{
	emit loginStatus(QString("Logare inaintea atacului"));
	QNetworkRequest request;
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_ro"+m_world));
    request.setRawHeader( "User-Agent",m_userAgent.toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding( m_userName ) );
    params.addEncodedQueryItem( "password", QUrl::toPercentEncoding( m_password ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	QNetworkReply *reply2=m_nam->post(request,paramBytes);
	connect( reply2, SIGNAL( finished() ),
             SLOT(action()));
}
void Authenticator::action()
{

	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	QNetworkRequest request;
	qDebug()<<reply->rawHeader(QByteArray("Location"));
	request.setUrl(QUrl::fromEncoded(reply->rawHeader(QByteArray("Location"))));
    request.setRawHeader( "User-Agent", m_userAgent.toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=m_nam->get(request);
	connect( reply2, SIGNAL( finished() ),
             SLOT(getSid()));
}
void Authenticator::getSid()
{
	QNetworkRequest request;
	request.setUrl(QUrl(QString("http://ro")+m_world+QString(".triburile.ro/game.php?screen=overview_villages&intro")));
	request.setRawHeader( "User-Agent", m_userAgent.toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=m_nam->get(request);
	connect( reply2, SIGNAL( finished() ),
             SLOT(generalView()));
	connect(reply2,SIGNAL(readyRead()),SLOT(buildHtml()));
}
void Authenticator::generalView()
{
	emit loginStatus(QString("Logarea a avut loc cu succes"));
	emit loginSucces(html);
}

void Authenticator::extractPassword(QByteArray & form)
{
	QString password(form);
	password.remove('"');
	password.remove('{');
	password.remove('}');
	password.remove( '\\');
	password.remove("res:<form action=index.php?action=login method=post class=server-form id=server_select_list>nt<input name=user type=hidden value="+m_userName+" />nt<input name=password type=hidden value=",Qt::CaseInsensitive);
	password.remove(password.indexOf(" />",0,Qt::CaseInsensitive),password.size()-password.indexOf(" />",0,Qt::CaseInsensitive));
	m_password=password;
}
void Authenticator::buildHtml()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	html.append(reply->readAll());
}