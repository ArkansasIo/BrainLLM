#pragma once

#include <QString>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <memory>
#include "llm_engine.h"

namespace BrainLLM {

class RestServer : public QObject {
    Q_OBJECT

public:
    RestServer(int port = 8080);
    ~RestServer();
    
    bool start();
    void stop();
    bool is_running() const;
    
    void set_llm_engine(std::shared_ptr<LLMEngine> engine);
    
private slots:
    void on_new_connection();
    void on_read_ready();
    void on_disconnected();
    
private:
    QTcpServer* tcp_server_;
    int port_;
    std::shared_ptr<LLMEngine> engine_;
    
    struct HttpRequest {
        QString method;
        QString path;
        QString body;
        std::map<QString, QString> headers;
    };
    
    HttpRequest parse_http_request(const QString& raw_request);
    QByteArray build_http_response(const QByteArray& body, int status_code = 200,
                                   const QString& content_type = "application/json");
    QByteArray build_http_response(const QString& body, int status_code = 200,
                                   const QString& content_type = "application/json");
    bool is_static_asset_request(const HttpRequest& request) const;
    QByteArray serve_static_asset(const QString& path);
};

} // namespace BrainLLM
