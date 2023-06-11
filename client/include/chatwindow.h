#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "qlineedit.h"
#include <QWidget>
#include <QAbstractSocket>
#include <QListWidgetItem>

class ChatClient;
class QStandardItemModel;

namespace Ui
{
class ChatWindow;
}

class ChatWindow : public QWidget
{

    Q_OBJECT
    Q_DISABLE_COPY( ChatWindow )

public:

    explicit ChatWindow( QWidget * parent = nullptr );

    ~ChatWindow();

private:

    Ui::ChatWindow * m_ui;

    ChatClient * m_chatClient;

    QList< QStandardItemModel * > m_chatModels;

private slots:

    void setItemFontBold( QListWidgetItem * item, bool isBold );
    void openChat( QListWidgetItem * item );
    void checkNickname();
    void checkMessage();
    void attemptConnection();
    void attemptLogin();
    void loggedIn( QJsonArray const & users );
    void loginFailed( QString const & reason );
    void messageReceived( QString const & sender, QString const & text );
    void sendMessage();
    void disconnectedFromServer();
    void addNewUser( QString const & username );
    void userLeft( QString const & username );
    void error( QAbstractSocket::SocketError socketError );
};

#endif // CHATWINDOW_H
