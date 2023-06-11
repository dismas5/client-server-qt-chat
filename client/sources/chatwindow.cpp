#include "chatwindow.h"

#include "ui_chatwindow.h"

#include "chatclient.h"

#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QJsonArray>

ChatWindow::ChatWindow( QWidget * parent ) :
    QWidget( parent ),
    m_ui( new Ui::ChatWindow ),
    m_chatClient( new ChatClient( this ) ),
//    m_chatModels( new QStandardItemModel( this ) ),
    m_chatModels()
{
    m_ui->setupUi( this );
    m_ui->stackedWidget->setCurrentIndex( 0 );

//    m_chatModel->insertColumn( 0 );

    m_ui->sendButton->setEnabled( false );

//    m_ui->chatView->setModel( m_chatModel );

    connect( m_ui->nameLineEdit, &QLineEdit::textChanged, this, &ChatWindow::checkNickname );
    connect( m_ui->messageEdit, &QLineEdit::textChanged, this, &ChatWindow::checkMessage );

    connect( m_chatClient, &ChatClient::connected, this, &ChatWindow::attemptLogin );
    connect( m_chatClient, &ChatClient::loggedIn, this, &ChatWindow::loggedIn );
    connect( m_chatClient, &ChatClient::loginError, this, &ChatWindow::loginFailed );
    connect( m_chatClient, &ChatClient::messageReceived, this, &ChatWindow::messageReceived );
    connect( m_chatClient, &ChatClient::disconnected, this, &ChatWindow::disconnectedFromServer );
    connect( m_chatClient, &ChatClient::error, this, &ChatWindow::error );
    connect( m_chatClient, &ChatClient::addNewUser, this, &ChatWindow::addNewUser );
    connect( m_chatClient, &ChatClient::userLeft, this, &ChatWindow::userLeft );

    connect( m_ui->connectButton, &QPushButton::clicked, this, &ChatWindow::attemptConnection );
    connect( m_ui->sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage );

    connect( m_ui->messageEdit, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage );
    connect( m_ui->nameLineEdit, &QLineEdit::returnPressed, this, &ChatWindow::attemptConnection );

    connect( m_ui->usersListWidget, &QListWidget::itemClicked, this, &ChatWindow::openChat );
}

ChatWindow::~ChatWindow()
{
    delete m_ui;
}

void ChatWindow::checkNickname()
{
    m_ui->connectButton->setEnabled( !m_ui->nameLineEdit->text().isEmpty() );
}

void ChatWindow::openChat( QListWidgetItem * item )
{
    int const itemIndex = m_ui->usersListWidget->row( item );
    m_ui->chatView->setModel( m_chatModels[ itemIndex ] );
    setItemFontBold( item, false );
}

void ChatWindow::setItemFontBold( QListWidgetItem * item, bool isBold )
{
    auto currentFont = item->font();
    currentFont.setBold( isBold );
    item->setFont( currentFont );
}

void ChatWindow::checkMessage()
{
    bool isOk = !m_ui->messageEdit->text().isEmpty() && !m_ui->usersListWidget->selectedItems().empty();
    m_ui->sendButton->setEnabled( isOk );
}

void ChatWindow::attemptConnection()
{
    m_ui->myName->setText( m_ui->nameLineEdit->text() );
    m_ui->connectButton->setEnabled( false );

    m_chatClient->connectToServer( QHostAddress( QStringLiteral( "127.0.0.1" ) ), 3333 );

    m_ui->connectButton->setEnabled( true );
}

void ChatWindow::attemptLogin()
{
    m_chatClient->login( m_ui->nameLineEdit->text() );
}

void ChatWindow::loggedIn( QJsonArray const & users )
{
    m_ui->stackedWidget->setCurrentIndex( 1 );
    m_ui->messageEdit->setEnabled( true );
    m_ui->chatView->setEnabled( true );

    for( auto const & user : users )
    {
        m_ui->usersListWidget->addItem( user.toString() );
        auto model = new QStandardItemModel( this );
        model->insertColumn( 0 );
        m_chatModels.append( model );
    }
}

void ChatWindow::loginFailed( QString const & reason )
{
    QMessageBox::critical( this, tr( "Error" ), reason );
}

void ChatWindow::messageReceived( QString const & sender, QString const & text )
{
    auto itemChatWithSender = m_ui->usersListWidget->findItems( sender, Qt::MatchExactly )[ 0 ];
    auto selectedItems = m_ui->usersListWidget->selectedItems();

    if( selectedItems.empty() )
        setItemFontBold( itemChatWithSender, true );
    else
        if( selectedItems[ 0 ] != itemChatWithSender )
            setItemFontBold( itemChatWithSender, true );

    int const itemIndex = m_ui->usersListWidget->row( itemChatWithSender );
    auto model = m_chatModels[ itemIndex ];
    int newRow = model->rowCount();


    model->insertRow( newRow );

    model->setData( model->index( newRow, 0 ), text );
    model->setData( model->index( newRow, 0 ), int( Qt::AlignLeft | Qt::AlignVCenter ), Qt::TextAlignmentRole );
    model->setData( model->index( newRow, 0 ), QColor( 0, 0, 0, 100 ), Qt::BackgroundColorRole );
    model->setData( model->index( newRow, 0 ), QColor( 255, 255, 255 ), Qt::TextColorRole );

    m_ui->chatView->scrollToBottom();
}

void ChatWindow::sendMessage()
{
    auto selectedItem = m_ui->usersListWidget->selectedItems()[ 0 ];
    int const itemIndex = m_ui->usersListWidget->row( selectedItem );
    auto model = m_chatModels[ itemIndex ];
    int const newRow = model->rowCount();

    m_chatClient->sendMessage( m_ui->messageEdit->text(), m_ui->myName->text(), selectedItem->text() );

    model->insertRow( newRow );
    model->setData( model->index( newRow, 0 ), m_ui->messageEdit->text() );
    model->setData( model->index( newRow, 0 ), int( Qt::AlignRight | Qt::AlignVCenter ), Qt::TextAlignmentRole );
    model->setData( model->index( newRow, 0 ), QColor( 0, 0, 255, 100 ), Qt::BackgroundColorRole );
    model->setData( model->index( newRow, 0 ), QColor( 255, 255, 255 ), Qt::TextColorRole );

    m_ui->messageEdit->clear();

    m_ui->chatView->scrollToBottom();
}

void ChatWindow::disconnectedFromServer()
{
    QMessageBox::warning( this, tr( "Disconnected" ), tr( "The host terminated the connection" ) );

    m_ui->sendButton->setEnabled( false );
    m_ui->messageEdit->setEnabled( false );
    m_ui->chatView->setEnabled( false );
    m_ui->connectButton->setEnabled( true );
//    m_chatModel->clear();
    m_ui->stackedWidget->setCurrentIndex( 0 );;
}

void ChatWindow::addNewUser( QString const & username )
{
    m_ui->usersListWidget->addItem( username );
    auto model = new QStandardItemModel( this );
    model->insertColumn( 0 );
    m_chatModels.append( model );

    m_ui->chatView->scrollToBottom();
}

void ChatWindow::userLeft( QString const & username )
{
    auto itemToDelete = m_ui->usersListWidget->findItems( username, Qt::MatchExactly );
    int const indexToDelete = m_ui->usersListWidget->row( itemToDelete[ 0 ] );

    m_ui->usersListWidget->takeItem( indexToDelete );
    m_chatModels.removeAt( indexToDelete );
    m_ui->chatView->scrollToBottom();
}

void ChatWindow::error( QAbstractSocket::SocketError socketError )
{
    switch( socketError )
    {
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::ProxyConnectionClosedError:
        return; // handled by disconnectedFromServer
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The host refused the connection"));
        break;
    case QAbstractSocket::ProxyConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The proxy refused the connection"));
        break;
    case QAbstractSocket::ProxyNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the proxy"));
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the server"));
        break;
    case QAbstractSocket::SocketAccessError:
        QMessageBox::critical(this, tr("Error"), tr("You don't have permissions to execute this operation"));
        break;
    case QAbstractSocket::SocketResourceError:
        QMessageBox::critical(this, tr("Error"), tr("Too many connections opened"));
        break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::warning(this, tr("Error"), tr("Operation timed out"));
        return;
    case QAbstractSocket::ProxyConnectionTimeoutError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy timed out"));
        break;
    case QAbstractSocket::NetworkError:
        QMessageBox::critical(this, tr("Error"), tr("Unable to reach the network"));
        break;
    case QAbstractSocket::UnknownSocketError:
        QMessageBox::critical(this, tr("Error"), tr("An unknown error occured"));
        break;
    case QAbstractSocket::UnsupportedSocketOperationError:
        QMessageBox::critical(this, tr("Error"), tr("Operation not supported"));
        break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        QMessageBox::critical(this, tr("Error"), tr("Your proxy requires authentication"));
        break;
    case QAbstractSocket::ProxyProtocolError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy comunication failed"));
        break;
    case QAbstractSocket::TemporaryError:
    case QAbstractSocket::OperationError:
        QMessageBox::warning(this, tr("Error"), tr("Operation failed, please try again"));
        return;
    default:
        Q_UNREACHABLE();
    }

    m_ui->connectButton->setEnabled( true );
    m_ui->stackedWidget->setCurrentIndex( 0 );
    m_ui->sendButton->setEnabled( false );
    m_ui->messageEdit->setEnabled( false );
    m_ui->chatView->setEnabled( false );
}
