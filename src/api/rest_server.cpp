#include "rest_server.h"
#include "request_handler.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrl>
#include <QUrlQuery>
#include <QTcpSocket>

namespace BrainLLM {

namespace {

QString status_text_for(int status_code) {
    if (status_code == 204) return "No Content";
    if (status_code == 400) return "Bad Request";
    if (status_code == 403) return "Forbidden";
    if (status_code == 404) return "Not Found";
    if (status_code >= 500) return "Server Error";
    return "OK";
}

QString clean_static_path(QString path) {
    if (path == "/" || path == "/client") {
        return "index.html";
    }
    if (path.startsWith("/client/")) {
        path = path.mid(QString("/client/").size());
    } else if (path.startsWith('/')) {
        path = path.mid(1);
    }
    return QDir::cleanPath(path);
}

QStringList static_roots() {
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    return {
        QDir(app_dir).filePath("client"),
        QDir(cwd).filePath("output/build/client"),
        QDir(cwd).filePath("web/client"),
        QDir(app_dir).filePath("../web/client")
    };
}

} // namespace

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
        socket->write(build_http_response(QString("{\"error\":\"Engine not initialized\"}"), 503));
        socket->flush();
        socket->close();
        return;
    }

    if (request.method == "OPTIONS") {
        socket->write(build_http_response(QString("{}"), 204));
        socket->flush();
        socket->close();
        return;
    }

    if (is_static_asset_request(request)) {
        socket->write(serve_static_asset(request.path));
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
    socket->write(build_http_response(payload, status));
    
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

QByteArray RestServer::build_http_response(const QByteArray& body, int status_code, const QString& content_type) {
    const QString status_text = status_text_for(status_code);
    QString response = QString("HTTP/1.1 %1 %2\r\n").arg(status_code).arg(status_text);
    response += QString("Content-Type: %1\r\n").arg(content_type);
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Connection: close\r\n";
    response += QString("Content-Length: %1\r\n\r\n").arg(body.size());
    return response.toUtf8() + body;
}

QByteArray RestServer::build_http_response(const QString& body, int status_code, const QString& content_type) {
    return build_http_response(body.toUtf8(), status_code, content_type);
}

bool RestServer::is_static_asset_request(const HttpRequest& request) const {
    if (request.method != "GET") {
        return false;
    }
    return request.path == "/" ||
           request.path == "/client" ||
           request.path.startsWith("/client/") ||
           request.path == "/app.js" ||
           request.path == "/styles.css";
}

QByteArray RestServer::serve_static_asset(const QString& path) {
    const QString relative = clean_static_path(path);
    if (relative.startsWith("..") || relative.contains("/../")) {
        return build_http_response(QString("{\"error\":\"Invalid static asset path\"}"), 403);
    }

    for (const QString& root : static_roots()) {
        QFileInfo candidate(QDir(root).filePath(relative));
        if (!candidate.exists() || !candidate.isFile()) {
            continue;
        }

        QFile file(candidate.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            return build_http_response(QString("{\"error\":\"Could not read static asset\"}"), 500);
        }

        QMimeDatabase mime_db;
        QString content_type = mime_db.mimeTypeForFile(candidate).name();
        if (candidate.suffix() == "js") content_type = "text/javascript";
        if (candidate.suffix() == "css") content_type = "text/css";
        if (candidate.suffix() == "html") content_type = "text/html; charset=utf-8";
        return build_http_response(file.readAll(), 200, content_type);
    }

    return build_http_response(QString("{\"error\":\"Static asset not found\"}"), 404);
}

} // namespace BrainLLM
