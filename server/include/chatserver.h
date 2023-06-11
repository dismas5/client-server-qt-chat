#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QVector>

class QThread;
class ServerWorker;
class QJsonObject;

class ChatServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY( ChatServer )

public:

    explicit ChatServer( QObject *parent = nullptr );
    ~ChatServer();

protected:

    void incomingConnection( qintptr socketDescriptor ) override;

private:

    const int m_idealThreadCount;
    QVector< QThread * > m_availableThreads;
    QVector< int > m_threadsLoad;
    QVector< ServerWorker *> m_clients;

private slots:
    void jsonReceived( ServerWorker *sender, QJsonObject const & doc );
    void userDisconnected( ServerWorker *sender, int threadIdx );
    void userError( ServerWorker *sender );

public slots:
    void stopServer();

private:

    void broadcast( QJsonObject message, ServerWorker * exclude );
    void jsonFromLoggedOut( ServerWorker * sender, QJsonObject const & doc );
    void sendJson( ServerWorker * destination, QJsonObject const & message );
    ServerWorker * getWorkerByName( QString const & name );

signals:

    void logMessage( QString const & msg );
    void stopAllClients();

};

#endif // CHATSERVER_H
