#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>

class QHostAddress;
class QJsonDocument;

class ChatClient : public QObject
{

    Q_OBJECT
    Q_DISABLE_COPY( ChatClient )

public:

    explicit ChatClient( QObject *parent = nullptr );

public slots:

    void connectToServer( QHostAddress const & address, quint16 port );
    void login( QString const & userName );
    void sendMessage( QString const & text, QString const & sender, QString const & reciever );
    void disconnectFromHost();

private slots:

    void onReadyRead();

signals:

    void connected();
    void loggedIn( QJsonArray const & users );
    void loginError( QString const &reason);
    void disconnected();
    void messageReceived( QString const &sender, QString const & text );
    void error( QAbstractSocket::SocketError socketError );
    void addNewUser( QString const & username );
    void userLeft( QString const & username );

private:

    QTcpSocket * m_clientSocket;
    bool m_loggedIn;

    void jsonReceived( QJsonObject const & json );
};

#endif // CHATCLIENT_H
