#include "serverwindow.h"
#include "ui_serverwindow.h"
#include "chatserver.h"

#include <QMessageBox>

ServerWindow::ServerWindow( QWidget * parent ):
    QWidget( parent ),
    m_ui( new Ui::ServerWindow ),
    m_chatServer( new ChatServer( this ) )
{
    m_ui->setupUi( this );

    connect( m_ui->startStopButton, &QPushButton::clicked, this, &ServerWindow::toggleStartServer );
    connect( m_chatServer, &ChatServer::logMessage, this, &ServerWindow::logMessage );
}

ServerWindow::~ServerWindow()
{
    delete m_ui;
}

void ServerWindow::toggleStartServer()
{
    if( m_chatServer->isListening() )
    {
        m_chatServer->stopServer();
        m_ui->startStopButton->setText( tr( "Start Server" ) );
        logMessage( QStringLiteral( "Server Stopped" ) );
    }
    else
    {
        if( !m_chatServer->listen( QHostAddress::Any, 3333 ) )
        {
            QMessageBox::critical( this, tr( "Error" ), tr( "Unable to start the server" ) );
            return;
        }
        logMessage( QStringLiteral( "Server Started" ) );
        m_ui->startStopButton->setText( tr( "Stop Server" ) );
    }
}

void ServerWindow::logMessage( QString const & msg )
{
    m_ui->logEditor->appendPlainText( msg + QLatin1Char( '\n' ) );
}
