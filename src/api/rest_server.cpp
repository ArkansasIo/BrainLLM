#include "rest_server.h"
#include <QTcpSocket>

namespace BrainLLM {

RestServer::RestServer(int port)
    : tcp_server_(new QTcpServer(this)), port_(port), engine_(nullptr) {
    connect(tcp_server_, &QTcpServer::newConnection, this, &RestServer::on_new_connection);
}

RestServer::~RestServer() {
    stop();
}

bool RestServer::start() {
    return tcp_server_->listen(QHostAddress::Any, port_);
}

void RestServer::stop() {
    tcp_server_->close();
}

bool RestServer::is_running() const {
    return tcp_server_->isListening();
}

void RestServer::set_llm_engine(std::shared_ptr<LLMEngine> engine) {
    engine_ = engine;
}

void RestServer::on_new_connection() {
    while (QTcpSocket* socket = tcp_server_->nextPendingConnection()) {
        connect(socket, &QTcpSocket::readyRead, this, &RestServer::on_read_ready);
        connect(socket, &QTcpSocket::disconnected, this, &RestServer::on_disconnected);
    }
}

void RestServer::on_read_ready() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString raw_data = QString::fromUtf8(socket->readAll());
    auto request = parse_http_request(raw_data);
    
    QString response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    
    if (!engine_) {
        socket->write(response.toUtf8());
    }
    
    socket->flush();
    socket->close();
}

void RestServer::on_disconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

RestServer::HttpRequest RestServer::parse_http_request(const QString& raw_request) {
    HttpRequest request;
    QStringList lines = raw_request.split("\r\n");
    
    if (lines.size() > 0) {
        QStringList first_line = lines[0].split(" ");
        if (first_line.size() >= 2) {
            request.method = first_line[0];
            request.path = first_line[1];
        }
    }
    
    // Parse headers
    int body_start = raw_request.indexOf("\r\n\r\n");
    if (body_start != -1) {
        request.body = raw_request.mid(body_start + 4);
    }
    
    return request;
}

QString RestServer::build_http_response(const QString& body, int status_code) {
    QString status_text = (status_code == 200) ? "OK" : "Error";
    QString response = QString("HTTP/1.1 %1 %2\r\n").arg(status_code).arg(status_text);
    response += "Content-Type: application/json\r\n";
    response += QString("Content-Length: %1\r\n\r\n").arg(body.length());
    response += body;
    return response;
}

} // namespace BrainLLM
