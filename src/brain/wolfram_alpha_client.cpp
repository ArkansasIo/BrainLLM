#include "wolfram_alpha_client.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace BrainLLM {

namespace {

QString configured_app_id() {
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString app_id = env.value("WOLFRAM_APP_ID").trimmed();
    if (app_id.isEmpty()) {
        app_id = env.value("WOLFRAM_ALPHA_APPID").trimmed();
    }
    if (!app_id.isEmpty()) {
        return app_id;
    }

    QStringList config_paths;
    config_paths << QDir::current().absoluteFilePath("config.ini");

    if (QCoreApplication::instance()) {
        const QDir app_dir(QCoreApplication::applicationDirPath());
        config_paths << app_dir.absoluteFilePath("config.ini");
        config_paths << app_dir.absoluteFilePath("../config.ini");
        config_paths << app_dir.absoluteFilePath("../../config.ini");
    }

    for (const QString& path : config_paths) {
        QFile file(QDir::cleanPath(path));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            const QString line = stream.readLine().trimmed();
            if (line.startsWith('#') || line.startsWith(';') || !line.contains('=')) {
                continue;
            }

            const int separator = line.indexOf('=');
            const QString key = line.left(separator).trimmed().toLower();
            const QString value = line.mid(separator + 1).trimmed();
            if ((key == "wolfram_alpha_appid" || key == "wolfram_app_id") && !value.isEmpty()) {
                return value;
            }
        }
    }

    return app_id;
}

QString wolfram_input_url(const QString& query) {
    QUrl url("https://www.wolframalpha.com/input");
    QUrlQuery params;
    params.addQueryItem("i", query);
    url.setQuery(params);
    return url.toString();
}

} // namespace

WolframAlphaClient::WolframAlphaClient()
    : app_id_(configured_app_id()) {}

bool WolframAlphaClient::is_configured() const {
    return !app_id_.isEmpty();
}

QString WolframAlphaClient::app_id() const {
    return app_id_;
}

WolframAlphaResult WolframAlphaClient::short_answer(const QString& query, int timeout_ms) const {
    return request_text_result(
        "https://api.wolframalpha.com/v1/result",
        query,
        "i",
        timeout_ms);
}

WolframAlphaResult WolframAlphaClient::llm_answer(const QString& query, int timeout_ms) const {
    return request_text_result(
        "https://www.wolframalpha.com/api/v1/llm-api",
        query,
        "input",
        timeout_ms);
}

WolframAlphaResult WolframAlphaClient::request_text_result(
    const QString& endpoint,
    const QString& query,
    const QString& input_parameter,
    int timeout_ms) const {

    if (query.trimmed().isEmpty()) {
        return {false, "", "Wolfram Alpha query is empty.", ""};
    }

    if (!is_configured()) {
        return {
            false,
            "",
            "Wolfram Alpha AppID is not configured. Set WOLFRAM_APP_ID, WOLFRAM_ALPHA_APPID, or wolfram_alpha_appid in config.ini.",
            wolfram_input_url(query)
        };
    }

    if (!QCoreApplication::instance()) {
        return {false, "", "Qt application context is not available for network requests.", wolfram_input_url(query)};
    }

    QUrl url(endpoint);
    QUrlQuery params;
    params.addQueryItem("appid", app_id_);
    params.addQueryItem(input_parameter, query);
    url.setQuery(params);

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "BrainLLM/1.0");

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(timeout_ms);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        reply->deleteLater();
        return {false, "", "Wolfram Alpha request timed out.", wolfram_input_url(query)};
    }

    const int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray payload = reply->readAll();
    const QString text = QString::fromUtf8(payload).trimmed();
    const QNetworkReply::NetworkError network_error = reply->error();
    const QString network_error_text = reply->errorString();
    reply->deleteLater();

    if (network_error != QNetworkReply::NoError || status_code >= 400) {
        const QString detail = text.isEmpty() ? network_error_text : text;
        return {false, "", QString("Wolfram Alpha request failed: %1").arg(detail), wolfram_input_url(query)};
    }

    if (text.isEmpty()) {
        return {false, "", "Wolfram Alpha returned an empty answer.", wolfram_input_url(query)};
    }

    return {true, text, "", wolfram_input_url(query)};
}

} // namespace BrainLLM
