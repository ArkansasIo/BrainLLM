#include "rest_server.h"
#include "request_handler.h"
#include <QUrl>
#include <QUrlQuery>
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
    
    QByteArray raw_bytes = socket->readAll();
    const int header_end = raw_bytes.indexOf("\r\n\r\n");
    if (header_end >= 0) {
        const QByteArray headers = raw_bytes.left(header_end);
        int content_length = 0;
        const QList<QByteArray> header_lines = headers.split('\n');
        for (QByteArray line : header_lines) {
            line = line.trimmed();
            if (line.toLower().startsWith("content-length:")) {
                content_length = line.mid(line.indexOf(':') + 1).trimmed().toInt();
                break;
            }
        }

        const int expected_size = header_end + 4 + content_length;
        while (content_length > 0 && raw_bytes.size() < expected_size && socket->waitForReadyRead(250)) {
            raw_bytes += socket->readAll();
        }
    }

    QString raw_data = QString::fromUtf8(raw_bytes);
    auto request = parse_http_request(raw_data);
    
    if (!engine_) {
        socket->write(build_http_response("{\"error\":\"Engine not initialized\"}", 503).toUtf8());
        socket->flush();
        socket->close();
        return;
    }

    if (request.method == "OPTIONS") {
        socket->write(build_http_response("{}", 204).toUtf8());
        socket->flush();
        socket->close();
        return;
    }

    RequestHandler handler(engine_);
    QString body = request.body;
    QString path = request.path;

    const int query_start = path.indexOf('?');
    if (query_start >= 0) {
        QUrlQuery query(path.mid(query_start + 1));
        path = path.left(query_start);
        if (body.isEmpty() && query.hasQueryItem("query")) {
            body = query.queryItemValue("query");
        }
    }

    QString payload = handler.handle_request(request.method, path, body);
    int status = payload.contains("\"error\"") ? 404 : 200;
    socket->write(build_http_response(payload, status).toUtf8());
    
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
    QString status_text = "OK";
    if (status_code == 404) status_text = "Not Found";
    else if (status_code >= 500) status_text = "Server Error";
    else if (status_code >= 400) status_text = "Bad Request";

    QString response = QString("HTTP/1.1 %1 %2\r\n").arg(status_code).arg(status_text);
    response += "Content-Type: application/json\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Connection: close\r\n";
    response += QString("Content-Length: %1\r\n\r\n").arg(body.toUtf8().size());
    response += body;
    return response;
}

} // namespace BrainLLM
