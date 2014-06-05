#include "mainwindow.h"
#include <QMdiArea>
#include "atacksPage.h"
#include "dialoglogin.h"
#include "dialogplannewatack.h"
#include <QObject>
#include <QDebug>
#include "statusbar.h"
#include <QDir>
#include <QtXml>
#include <QtNetwork>
#include <QtGui>
mainWindow::mainWindow()
{
	setupUi(this);
	
	this->plainTextEditLogs->hide();
	this->actionGrupa_noua->setEnabled(false);
	baraStatus=new appStatusBar();
	baraStatus->labelTime->setText(QDateTime::currentDateTime().time().toString());
	QTimer* timer=new QTimer(this);
	connect(timer,SIGNAL(timeout()), this, SLOT(setDate()));
	timer->start(1000);
	this->statusBarMain->addWidget(baraStatus,2);
	connect(baraStatus->pushButton,SIGNAL(toggled(bool)),this->plainTextEditLogs,SLOT(setShown(bool)));
	nam=new QNetworkAccessManager(this);
	loginGUI=new dialogLogin(this);
	loginGUI->hide();
	//cateva setari dialogului pentru logare
	loginGUI->lineEditName->setText(QString("tarabostes22"));
	loginGUI->lineEditPassword->setText(QString("091182dfc"));
	loginGUI->lineEditUserAgent->setText("Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.162 Safari/535.19");
	//sloturi loginGUI
	//se face logarea
	connect(loginGUI->pushButtonLog,SIGNAL(clicked()),this,SLOT(getLicense()));
	fereastraPlanuireAtac=new dialogPlanNewAtack(nam,this);
	fereastraPlanuireAtac->hide();
	//sloturi fereastra planuire atac
	//trimite user agentul,numele folosit pentru logare si lumea selectata
	connect(this,SIGNAL(sendUserAgent(QString,QString,QString)),fereastraPlanuireAtac,SLOT(setUserAgent(QString,QString,QString)));
	//daca userul de click pentru a incepe planul facem logarea
	connect(fereastraPlanuireAtac->pushButtonStartPlanningAtack,SIGNAL(clicked()),this,SLOT(beforePlanningLogin()));
	//dupa ce s-a facut logarea se poate incepe lansarea atacului
	connect(this,SIGNAL(startPlanning()),fereastraPlanuireAtac,SLOT(planAtack()));
	//sloturi si semnale de la planWorker.Toate vor fie queued deoarece workerul va fi in alt thread
	connect(fereastraPlanuireAtac->worker,SIGNAL(planAtackLog(QString)),this,SLOT(outputLog(QString)),Qt::QueuedConnection);
	connect(fereastraPlanuireAtac->worker,SIGNAL(planAtackStatus(QString)),this,SLOT(setStatus(QString)),Qt::QueuedConnection);
	connect(fereastraPlanuireAtac->worker,SIGNAL(planAtackFinish(QString)),this,SLOT(newAtackGroup(QString)),Qt::QueuedConnection);
	connect(this,SIGNAL(deleteGroup(QString)),fereastraPlanuireAtac->worker,SLOT(deleteGroup(QString)),Qt::QueuedConnection);
	connect(this,SIGNAL(refreshHtml(QString)),fereastraPlanuireAtac->worker,SLOT(refreshTroops(QString)),Qt::QueuedConnection);
	connect(fereastraPlanuireAtac->worker,SIGNAL(refreshRequest(QNetworkRequest )),this,SLOT(refreshRequest(QNetworkRequest)),Qt::QueuedConnection);
	//meniul de actiuni
	connect(this->actionFereastra_ant,SIGNAL(triggered()),this->mdiAreaMainWindow,SLOT(activatePreviousSubWindow()));
	connect(this->actionFereastra_urm,SIGNAL(triggered()),this->mdiAreaMainWindow,SLOT(activateNextSubWindow()));
	connect(this->actionGrupa_noua,SIGNAL(triggered()),fereastraPlanuireAtac,SLOT(show()));
	connect(this->actionSterge_grupa,SIGNAL(triggered()),this,SLOT(deleteGroupAction()));
	connect(this->actionLogin,SIGNAL(triggered()),loginGUI,SLOT(show()));
}
void mainWindow::setDate()
{
	baraStatus->labelTime->setText(QDateTime::currentDateTime().time().toString());
}

void mainWindow::outputLog(QString message)
{
	this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\">")+message+QString("<\\span>"));
}
void mainWindow::setStatus(QString operatiune)
{
	this->baraStatus->label->setText(operatiune);
}



//de la authenticator
//intai verificam daca userul detine o licenta
void mainWindow::getLicense()
{
	QNetworkRequest request;
	//setam url-ul cererii
    request.setUrl(QUrl("http://tw-planner.tk/1235923558220523.txt"));
	request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply=nam->get(request);
	connect(reply,SIGNAL(finished()),this,SLOT(receiveLicense()));
}
void mainWindow::receiveLicense()
{
	 QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	 QString source(reply->readAll());

	 if(source.contains(QString("<")+loginGUI->lineEditName->text()+QString(">"),Qt::CaseInsensitive))
	 {
		  postSid();
	 }
	 else
	 {
		 this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\"> Nu detineti o licenta pentru contul :")+this->loginGUI->lineEditName->text()
		 +QString("Putenti achizitiona una la adresa <a href= \" http://tw-planner.tk/index/licenta/0-10 \"" )+QString("<\\span>\n"));
		 int r = QMessageBox::warning(this, tr("Licenta"),
                        tr("Nu detineti o licenta pentru acest cont.\n"
                           "Doriti sa cumparati una sau sa testati programul o saptamana?"),
                        QMessageBox::Yes | QMessageBox::No
                        );
		 if(r== QMessageBox::Yes)
			  QDesktopServices::openUrl(QUrl("http://tw-planner.tk/index/licenta/0-10"));
		 else 
		 {
			 QMessageBox::information(this,tr("Licenta"),tr("Ne pare rau dar nu puteti folosi acest program fara o licenta"), QMessageBox::Ok);
			 return;
		 }
	 }

}
void mainWindow::postSid()
{
	this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\">Logare pe contul: ")+loginGUI->lineEditName->text()+
		QString(" Lumea : ")+loginGUI->comboBoxWorld->currentText()+QString("<\\span>"));
	this->baraStatus->label->setText(QString("Inceperea operatiunii de logare"));
	//ne asiguram ca QString html este gol pentru a putea salva in el codul sursa
	html.clear();

	QNetworkRequest request;
	//setam url-ul cererii
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_list=1&show_server_selection=1"));
	//seteam headerul cu user agentul
	request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
	//setam tipul continutului
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	//codam userul si parola pentru a fi transmise site-ului
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding(this->loginGUI->lineEditName->text()));
	params.addEncodedQueryItem( "password", QUrl::toPercentEncoding(this->loginGUI->lineEditPassword->text() ) );
	params.addEncodedQueryItem( "clear", QUrl::toPercentEncoding( "true" ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );//un bug.../ nu este codat ca /
	//trimitem cererea siteului
	QNetworkReply *reply=nam->post(request,paramBytes);
	connect( reply, SIGNAL( finished() ),
             SLOT(homepagesc()));
}
void mainWindow::homepagesc()
{	
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	//citim raspunsul siteului
	QString info2 (reply->readAll());
	//extragem parola din raspuns
	extractPassword(info2);
	//verificam daca parola este corecta
	if(info2==QString("error:Parolu0103 incorectu010"))
	{
		this->baraStatus->label->setText(QString("Parola incorecta"));
		this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\">Parola este gresita ")+QString("<\\span>"));
		return;
	}
	if(info2==QString("error:Cont inexisten"))
	{
		this->baraStatus->label->setText(QString("User inexistent"));
		this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\">Nu exista un cont cu numele dat ")+QString("<\\span>"));
		return;
	}
	this->loginGUI->lineEditPassword->setText(info2);
	QNetworkRequest request;
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_ro"+this->loginGUI->comboBoxWorld->currentText()));
    request.setRawHeader( "User-Agent",this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding( this->loginGUI->lineEditName->text() ) );
	params.addEncodedQueryItem( "password", QUrl::toPercentEncoding( info2 ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	QNetworkReply *reply2=nam->post(request,paramBytes);
	connect( reply2, SIGNAL( finished() ),
             SLOT(action()));
}

void mainWindow::action()
{

	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	QNetworkRequest request;
	QString location(reply->rawHeader(QByteArray("Location")));
	if(location==QString("http://www.triburile.ro/create_account.php?server_id=ro")+this->loginGUI->comboBoxWorld->currentText())
	{
		this->baraStatus->label->setText(QString("Nu aveti creat cont pe lumea ")+loginGUI->comboBoxWorld->currentText());
		this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\">Nu aveti creat cont pe lumea ")+loginGUI->comboBoxWorld->currentText()+QString(" .Selectati o alta lume<\\span>"));
		return;
	}

	//logarea a avut loc cu succes
	this->plainTextEditLogs->appendHtml(QString("<span style=\"color:#ff0000;\">")+QDateTime::currentDateTime().time().toString()+QString("<\\span> <span style=\"color:#ffffff;\"> Logarea pe contul :")+this->loginGUI->lineEditName->text()+QString(" a avut loc cu succes<\\span>\n"));
	//daca parola si userul sunt  corecte trimitem si user Agentul planificatorului de atacuri
	emit sendUserAgent(this->loginGUI->lineEditUserAgent->text(),this->loginGUI->comboBoxWorld->currentText(),this->loginGUI->lineEditName->text());
	this->actionLogin->setEnabled(false);
	this->loginGUI->hide();

	request.setUrl(QUrl(location));
    request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=nam->get(request);
	connect( reply2, SIGNAL( finished() ),
             SLOT(getSid()));
}
void mainWindow::getSid()
{
	QNetworkRequest request;
	request.setUrl(QUrl(QString("http://ro")+loginGUI->comboBoxWorld->currentText()
		+QString(".triburile.ro/game.php?screen=overview_villages&intro")));
	request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=nam->get(request);
	connect( reply2, SIGNAL( finished() ),
             SLOT(loginSucces()));
	connect(reply2,SIGNAL(readyRead()),SLOT(buildHtml()));
}

void mainWindow::loginSucces()
{
	this->baraStatus->label->setText(QString("Logarea a avut loc cu succes"));
	this->actionGrupa_noua->setEnabled(true);
	this->loginGUI->hide();
	//blocam fereastra de login pentru a nu fi modificate datele
	/*this->loginGUI->pushButtonLog->setEnabled(false);
	this->loginGUI->lineEditName->setReadOnly(true);
	this->loginGUI->lineEditPassword->setReadOnly(true);
	this->loginGUI->lineEditUserAgent->setReadOnly(true);*/
	//salvam lista satelor...cream directorul contului
	QDir currentDir(QDir::currentPath());
	currentDir.mkpath(QString("Conturi/")+this->loginGUI->lineEditName->text()+QString("/Lumea")+this->loginGUI->comboBoxWorld->currentText());
	QFile file(QString("Conturi/")+this->loginGUI->lineEditName->text()+QString("/Lumea")+this->loginGUI->comboBoxWorld->currentText()+QString("/Lista-sate.txt"));
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);
	QRegExp rx("<span\\sid=\"label_text_(\\d*)?\">[^<]*K\\d{1,2}</span>");
	int pos = 0;rx.indexIn(html,0);
	QString id=rx.cap(1);
	qDebug()<<id<<" id";
	//setam url-ul cererii
	QNetworkRequest request;
	request.setUrl(QUrl( QString("http://ro")+this->loginGUI->comboBoxWorld->currentText()+QString(".triburile.ro/game.php?village=")+id+QString("&screen=place")));
	//seteam headerul cu user agentul
	request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
	//setam tipul continutului
	request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply=nam->get(request);
	connect( reply, SIGNAL( finished() ),this,
    SLOT(getAttribute()));
	while ((pos = rx.indexIn(html, pos)) != -1) 
	{
		QRegExp coord("\\d{1,3}\\|\\d{1,3}");
		coord.indexIn(rx.cap(0));
		out<<coord.cap(0)<< " "<< rx.cap(1)<< "\n";
		pos += rx.matchedLength();
	}
	file.close();
	//acum vedem daca exista grupe create anterior
	QFile grupe(QString("Conturi/")+loginGUI->lineEditName->text()+QString("/Lumea")+
		loginGUI->comboBoxWorld->currentText()+QString("/Grupe.txt"));
	if(!grupe.exists())
	{
		return;
	}
	grupe.open(QIODevice::ReadOnly|QIODevice::Text);
	QTextStream in(&grupe);
	while(!in.atEnd())
	{
		QString numeGrupa;
		in>>numeGrupa;
		
		QFile plan(QString("Conturi/")+loginGUI->lineEditName->text()+QString("/Lumea")+
			loginGUI->comboBoxWorld->currentText()+QString("/Atacuri/")+numeGrupa+QString(".txt"));
		
		if(plan.exists())
		{
			
			atacksPage *grupa=new atacksPage(nam,this->loginGUI->lineEditUserAgent->text(),numeGrupa,
				this->loginGUI->lineEditName->text(),loginGUI->comboBoxWorld->currentText(),inputName,inputValue);
			this->mdiAreaMainWindow->addSubWindow(grupa, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);
			//conectam sloturile si semnalele
			connect(grupa,SIGNAL(loginBeforeAtack()),this,SLOT(beforeAtackLogin()));
			connect(grupa,SIGNAL(deleteGroup(QString)),fereastraPlanuireAtac->worker,SLOT(deleteGroup(QString)),Qt::QueuedConnection);
			connect(grupa,SIGNAL(atackInfo(QString)),this,SLOT(outputLog(QString)));
			connect(grupa,SIGNAL(atackStatusBar(QString)),this,SLOT(setStatus(QString)));
			connect(grupa,SIGNAL(refreshId(QString)),fereastraPlanuireAtac->worker,SLOT(buildRequest(QString)),Qt::QueuedConnection);
			connect(this->actionActualizare_trupe,SIGNAL(triggered()),grupa,SLOT(refreshTroops()));
			grupa->show();
			//dupa ce grupa a fost creata dam start cronometrului de lansare
			grupa->setTimer();
		}
	}
	grupe.close();
	
}
void mainWindow::getAttribute()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	QDomDocument page;
	page.setContent(reply->readAll());
	QDomNodeList form=page.elementsByTagName(QString("form"));
	inputName=form.at(0).firstChild().toElement().attribute(QString("name"));
	inputValue=form.at(0).firstChild().toElement().attribute(QString("value"));
	qDebug()<<"name : "<<inputName<< "value : "<<inputValue;
}
void mainWindow::extractPassword(QString & password)
{
	password.remove('"');
	password.remove('{');
	password.remove('}');
	password.remove( '\\');
	password.remove("res:<form action=index.php?action=login method=post class=server-form id=server_select_list>nt<input name=user type=hidden value="+this->loginGUI->lineEditName->text()+" />nt<input name=password type=hidden value=",Qt::CaseInsensitive);
	password.remove(password.indexOf(" />",0,Qt::CaseInsensitive),password.size()-password.indexOf(" />",0,Qt::CaseInsensitive));
}
void mainWindow::buildHtml()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	html.append(reply->readAll());
}
void mainWindow::beforeAtackLogin()
{
	QNetworkRequest request;
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_ro"+loginGUI->comboBoxWorld->currentText()));
    request.setRawHeader( "User-Agent",this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding( this->loginGUI->lineEditName->text() ) );
	params.addEncodedQueryItem( "password", QUrl::toPercentEncoding( this->loginGUI->lineEditPassword->text() ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	QNetworkReply *reply2=nam->post(request,paramBytes);
	connect( reply2, SIGNAL( finished() ),
             SLOT(atackAction()));
}
void mainWindow::atackAction()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );

	QNetworkRequest request;
	request.setUrl(QUrl::fromEncoded(reply->rawHeader(QByteArray("Location"))));
    request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=nam->get(request);
	
}
void mainWindow::beforePlanningLogin()
{
	this->baraStatus->label->setText(QString("Creare plan de atac nou"));
	QNetworkRequest request;
    request.setUrl(QUrl( "http://www.triburile.ro/index.php?action=login&server_ro"+loginGUI->comboBoxWorld->currentText()));
    request.setRawHeader( "User-Agent",this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QUrl params;
	params.addEncodedQueryItem( "user", QUrl::toPercentEncoding( this->loginGUI->lineEditName->text() ) );
	params.addEncodedQueryItem( "password", QUrl::toPercentEncoding( this->loginGUI->lineEditPassword->text() ) );
	QByteArray paramBytes = params.toString().mid( 1 ).toLatin1();
	paramBytes.replace( "/", "/" );
	QNetworkReply *reply2=nam->post(request,paramBytes);
	connect( reply2, SIGNAL( finished() ),
             SLOT(planAction()));
}
void mainWindow::planAction()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	QNetworkRequest request;
	request.setUrl(QUrl::fromEncoded(reply->rawHeader(QByteArray("Location"))));
    request.setRawHeader( "User-Agent", this->loginGUI->lineEditUserAgent->text().toLatin1() );
    request.setHeader( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
	QNetworkReply *reply2=nam->get(request);
	connect( reply2, SIGNAL( finished() ),
             SIGNAL(startPlanning()));
}
//slotul de creare a unei noi grupe
void mainWindow::newAtackGroup(QString group)
{
	atacksPage *grupa=new atacksPage(nam,this->loginGUI->lineEditUserAgent->text(),group,
		this->loginGUI->lineEditName->text(),loginGUI->comboBoxWorld->currentText(),inputName,inputValue);
	this->mdiAreaMainWindow->addSubWindow(grupa, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);
	grupa->show();
	//conectam sloturile si semnalele
	connect(grupa,SIGNAL(loginBeforeAtack()),this,SLOT(beforeAtackLogin()));
	//connect(this,SIGNAL(launchAtack()),grupa,SLOT(launchAtack()));
	connect(grupa,SIGNAL(atackInfo(QString)),this,SLOT(outputLog(QString)));
	connect(grupa,SIGNAL(atackStatusBar(QString)),this,SLOT(setStatus(QString)));
	connect(grupa,SIGNAL(deleteGroup(QString)),fereastraPlanuireAtac->worker,SLOT(deleteGroup(QString)),Qt::QueuedConnection);
	connect(grupa,SIGNAL(refreshId(QString)),fereastraPlanuireAtac->worker,SLOT(buildRequest(QString)),Qt::QueuedConnection);
	connect(this->actionActualizare_trupe,SIGNAL(triggered()),grupa,SLOT(refreshTroops()));
	//adaugam grupa in fisierul cu toate grupele
	QFile accGroups(QString("Conturi/")+this->loginGUI->lineEditName->text()+QString("/Lumea")+loginGUI->comboBoxWorld->currentText()+QString("/Grupe.txt"));
	accGroups.open(QIODevice::Append|QIODevice::Text);
	QTextStream out(&accGroups);
	out<<"\n"<<group;
	accGroups.close();
	//dupa ce grupa a fost creata dam start cronometrului de lansare
	grupa->setTimer();
}
void mainWindow::deleteGroupAction()
{
	int r = QMessageBox::warning(this, tr("Stergere grupa"),
                        tr("Sunteti sigur ca doriti sa stergeti aceasta grupa?"),
                        QMessageBox::Yes | QMessageBox::No
                        );
	if(r=QMessageBox::Yes)
	{
		emit deleteGroup(mdiAreaMainWindow->currentSubWindow()->windowTitle());
		this->mdiAreaMainWindow->currentSubWindow()->close();
	}
	else
		return;
}
void mainWindow::refreshRequest(QNetworkRequest request)
{
	QNetworkReply *reply =nam->get(request);
	connect(reply,SIGNAL(finished()),this,SLOT(afterRefreshRequest()));
}
void mainWindow::afterRefreshRequest()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender() );
	emit refreshHtml(reply->readAll());
}