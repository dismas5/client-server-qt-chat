#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>

namespace Ui
{
class ServerWindow;
}

class ChatServer;

class ServerWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerWindow)

public:

    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:

    Ui::ServerWindow *m_ui;
    ChatServer *m_chatServer;

private slots:

    void toggleStartServer();
    void logMessage( QString const & msg );

};

#endif // SERVERWINDOW_H
