#include "graphqlconnection.h"
#include <QtCore/QDebug>
#include <QAbstractSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include "queryrequestdto.h"

GraphQlConnection::GraphQlConnection() :
    m_url(QString()),
    m_websocketConnection(new GraphQlWebsocketConnection(this)),
    m_httpConnection(new GraphQlHttpConnection(this))
{
    connect(m_websocketConnection, &GraphQlWebsocketConnection::dataReceived, this, &GraphQlConnection::dataReceived);
    connect(m_websocketConnection, &GraphQlWebsocketConnection::stateChanged, this, &GraphQlConnection::onStateChanged);
    connect(m_websocketConnection, &GraphQlWebsocketConnection::error, this, &GraphQlConnection::error);

    connect(m_httpConnection, &GraphQlHttpConnection::dataReceived, this, &GraphQlConnection::dataReceived);
}

GraphQlConnection::~GraphQlConnection()
{
    delete m_websocketConnection;
    delete m_httpConnection;
}

void GraphQlConnection::query(const QString &query)
{
    if (websocketConnectionState() !=  WebSocketConnectionState::Acknowledged) {
        qDebug() << "connection is not acknowledged, doing http request";

        m_httpConnection->sendMessage(QueryRequestDto(query));
        return;
    }

    qDebug() << "query: " << query;
    m_websocketConnection->sendMessage(OperationMessage::ConnectionStartMessage(QueryRequestDto(query).toJsonObject()));
}

void GraphQlConnection::open()
{
    m_websocketConnection->open();
}

QString GraphQlConnection::url() const
{
    return m_url;
}

GraphQlConnection::WebSocketConnectionState GraphQlConnection::websocketConnectionState() const
{
    return static_cast<WebSocketConnectionState>(m_websocketConnection->connectionState());
}

bool GraphQlConnection::isConnected() const
{
    return websocketConnectionState() == WebSocketConnectionState::Acknowledged;
}

void GraphQlConnection::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "error" << error;
}

void GraphQlConnection::setUrl(const QString &url)
{
    if (m_url == url)
        return;

    m_url = url;
    m_websocketConnection->setUrl(url);
    m_httpConnection->setUrl(url);
    emit urlChanged(m_url);
}

void GraphQlConnection::onStateChanged(GraphQlWebsocketConnection::ConnectionState state)
{
    emit websocketConnectionStateChanged(static_cast<WebSocketConnectionState>(state));
}

