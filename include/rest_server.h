#pragma once

#include <QString>
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
    QString build_http_response(const QString& body, int status_code = 200);
};

} // namespace BrainLLM
