#include "chatserver.h"
#include "serverworker.h"
#include <QThread>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QTimer>

ChatServer::ChatServer( QObject * parent ):
    QTcpServer(parent ),
    m_idealThreadCount( qMax( QThread::idealThreadCount(), 1 ) )
{
    m_availableThreads.reserve( m_idealThreadCount );
    m_threadsLoad.reserve( m_idealThreadCount );
}

ChatServer::~ChatServer()
{
    for( QThread * singleThread : m_availableThreads )
    {
        singleThread->quit();
        singleThread->wait();
    }
}
void ChatServer::incomingConnection( qintptr socketDescriptor )
{
    ServerWorker *worker = new ServerWorker;
    if( !worker->setSocketDescriptor( socketDescriptor ) )
    {
        worker->deleteLater();
        return;
    }
    int threadIdx = m_availableThreads.size();
    if( threadIdx < m_idealThreadCount )
    {
        m_availableThreads.append( new QThread( this ) );
        m_threadsLoad.append( 1 );
        m_availableThreads.last()->start();
    }
    else
    {
        // find the thread with the least amount of clients and use it
        threadIdx = std::distance( m_threadsLoad.cbegin(), std::min_element( m_threadsLoad.cbegin(), m_threadsLoad.cend() ) );
        ++m_threadsLoad[ threadIdx ];
    }
    worker->moveToThread( m_availableThreads.at( threadIdx ) );
    connect( m_availableThreads.at( threadIdx ), &QThread::finished, worker, &QObject::deleteLater );
    connect( worker, &ServerWorker::disconnectedFromClient, this, std::bind( &ChatServer::userDisconnected, this, worker, threadIdx ) );
    connect( worker, &ServerWorker::error, this, std::bind(&ChatServer::userError, this, worker));
    connect( worker, &ServerWorker::jsonReceived, this, std::bind( &ChatServer::jsonReceived, this, worker, std::placeholders::_1) );
    connect(this, &ChatServer::stopAllClients, worker, &ServerWorker::disconnectFromClient );
    m_clients.append( worker );
    emit logMessage( QStringLiteral( "New client Connected" ) );
}

void ChatServer::sendJson(ServerWorker *destination, const QJsonObject &message)
{
    Q_ASSERT( destination );
    QTimer::singleShot( 0, destination, std::bind( &ServerWorker::sendJson, destination, message ) );
}

void ChatServer::broadcast( QJsonObject message, ServerWorker * exclude )
{
    for(ServerWorker * worker : m_clients )
    {
        Q_ASSERT( worker );
        if( worker == exclude )
            continue;

        sendJson( worker, message );
    }
}

ServerWorker * ChatServer::getWorkerByName( QString const & name )
{
    for( auto const worker : m_clients )
    {
        if( worker->userName() == name )
            return worker;
    }
    return nullptr;
}

void ChatServer::jsonReceived( ServerWorker *sender, QJsonObject const & json )
{
    Q_ASSERT( sender );
    emit logMessage( QStringLiteral( "JSON received " ) + QString::fromUtf8( QJsonDocument( json ).toJson() ) );
    if( sender->userName().isEmpty() )
        return jsonFromLoggedOut( sender, json );

    auto const jsonType = json[ "type" ].toString();

    if( jsonType == QStringLiteral( "newuser" ) )
        broadcast( json, sender );
    else if( jsonType == QStringLiteral( "message" ) )
    {
        auto const recieverName = json[ QStringLiteral( "reciever" ) ].toString();
        auto const receiverWorker = getWorkerByName( recieverName );
        if( receiverWorker )
            sendJson( receiverWorker, json );
        else
            sendJson( sender, QJsonObject() );
    }
}

void ChatServer::userDisconnected( ServerWorker * sender, int threadIdx )
{
    --m_threadsLoad[ threadIdx ];
    m_clients.removeAll( sender );
    QString const & userName = sender->userName();
    if( !userName.isEmpty() )
    {
        QJsonObject disconnectedMessage;
        disconnectedMessage[ QStringLiteral( "type" ) ] = QStringLiteral( "userdisconnected" );
        disconnectedMessage[ QStringLiteral( "username" ) ] = userName;
        broadcast( disconnectedMessage, nullptr );
        emit logMessage( userName + QLatin1String(" disconnected") );
    }

    sender->deleteLater();
}

void ChatServer::userError(ServerWorker *sender)
{
    Q_UNUSED( sender )
    emit logMessage( QLatin1String("Error from ") + sender->userName() );
}

void ChatServer::stopServer()
{
    emit stopAllClients();
    close();
}

void ChatServer::jsonFromLoggedOut( ServerWorker * sender, QJsonObject const & json )
{
    QString const & type = json.value( QStringLiteral( "type" ) ).toString();
    if( type.isEmpty() )
        return;

    if(type != "login")
        return;

    QString const & username = json.value( QStringLiteral( "username" ) ).toString();
    if( username.isEmpty() )
        return;

    for (ServerWorker * worker : qAsConst( m_clients ) )
    {
        if( worker == sender )
            continue;

        if( worker->userName() == username )
        {
            QJsonObject message;
            message[ QStringLiteral( "type") ] = QStringLiteral( "login" );
            message[ QStringLiteral( "success") ] = false;
            message[ QStringLiteral( "reason") ] = QStringLiteral( "duplicate username" );
            sendJson( sender, message );
            return;
        }
    }
    sender->setUserName( username );
    QJsonObject successMessage;
    QJsonArray array;

    for( ServerWorker * worker : m_clients )
    {
        if( worker == sender )
            continue;

        array.append( worker->userName() );
    }

    successMessage[ QStringLiteral( "userlists" ) ] = array;
    successMessage[ QStringLiteral( "type" ) ] = QStringLiteral( "login" );
    successMessage[ QStringLiteral( "success" ) ] = true;
    sendJson( sender, successMessage );

    QJsonObject connectedMessage;
    connectedMessage[ QStringLiteral( "type" ) ] = QStringLiteral( "newuser" );
    connectedMessage[ QStringLiteral( "username" ) ] = username;
    broadcast(connectedMessage, sender);
}
