#include "chatclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

ChatClient::ChatClient( QObject * parent )
    : QObject( parent )
    , m_clientSocket( new QTcpSocket( this ) )
    , m_loggedIn( false )
{
    connect( m_clientSocket, &QTcpSocket::connected, this, &ChatClient::connected );

    connect( m_clientSocket, &QTcpSocket::disconnected, this, &ChatClient::disconnected );

    connect( m_clientSocket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead );

    connect( m_clientSocket, &QAbstractSocket::errorOccurred, this, &ChatClient::error );

    connect( m_clientSocket, &QTcpSocket::disconnected, this, [ this ](){ m_loggedIn = false; } );
}

void ChatClient::login( QString const & userName )
{
    if( m_clientSocket->state() != QAbstractSocket::ConnectedState )
        return;

    QDataStream clientStream( m_clientSocket );
    clientStream.setVersion( QDataStream::Qt_5_7 );

    QJsonObject message;
    message[ QStringLiteral( "type" ) ] = QStringLiteral( "login" );
    message[ QStringLiteral( "username" ) ] = userName;

    clientStream << QJsonDocument( message ).toJson( QJsonDocument::Compact );
}

void ChatClient::sendMessage( QString const & text, QString const & sender, QString const & reciever )
{
    if( text.isEmpty() || reciever.isEmpty() )
        return;

    QDataStream clientStream( m_clientSocket );
    clientStream.setVersion( QDataStream::Qt_5_7 );

    QJsonObject message;
    message[ QStringLiteral( "type" ) ] = QStringLiteral( "message" );
    message[ QStringLiteral( "text" ) ] = text;
    message[ QStringLiteral( "sender" ) ] = sender;
    message[ QStringLiteral( "reciever" ) ] = reciever;

    clientStream << QJsonDocument( message ).toJson();
}

void ChatClient::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}

void ChatClient::jsonReceived( QJsonObject const & json )
{
    QJsonValue const & typeVal = json.value( QLatin1String( "type" ) );
    if( typeVal.isNull() || !typeVal.isString() )
        return;

    auto const & typeValue = typeVal.toString().toLower();
    if( typeValue == "login" )
    {
        if( m_loggedIn )
            return;

        const QJsonValue resultVal = json.value(QLatin1String("success"));
        if (resultVal.isNull() || !resultVal.isBool())
            return;

        if( resultVal.toBool() )
        {
            emit loggedIn( json[ "userlists" ].toArray() );
            return;
        }

        QString const reason = json.value( QStringLiteral( "reason" ) ).toString();
        emit loginError( reason );
    }
    else if( typeValue == QStringLiteral( "message" ) )
    {
        QJsonValue const & textVal = json.value( QStringLiteral( "text" ) );
        QJsonValue const & senderVal = json.value( QStringLiteral( "sender" ) );

        if( textVal.isNull() || !textVal.isString() )
            return;

        if( senderVal.isNull() || !senderVal.isString() )
            return;

        emit messageReceived( senderVal.toString(), textVal.toString() );
    }
    else if( typeValue == QStringLiteral( "newuser" ) )
    {
        auto const & username = json[ "username" ].toString();
        if( username.isEmpty() )
            return;

        emit addNewUser( username );
    } else if( typeValue == QStringLiteral( "userdisconnected") )
    {
        auto const & username = json.value( QStringLiteral( "username" ) ).toString();
        if( username.isEmpty() )
            return;

        emit userLeft( username );
    }
}

void ChatClient::connectToServer( QHostAddress const & address, quint16 port )
{
    m_clientSocket->connectToHost( address, port );
}

void ChatClient::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream( m_clientSocket );

    socketStream.setVersion( QDataStream::Qt_5_7 );

    while( true )
    {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if( socketStream.commitTransaction() )
        {
            QJsonParseError parseError;
            QJsonDocument const jsonDoc = QJsonDocument::fromJson( jsonData, &parseError );
            if( parseError.error == QJsonParseError::NoError )
            {
                if( jsonDoc.isObject() )
                    jsonReceived( jsonDoc.object() );
            }
        }
        else
            break;
    }
}
