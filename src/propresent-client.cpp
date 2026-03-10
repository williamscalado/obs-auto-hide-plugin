#include "propresent-client.hpp"
#include <obs-module.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

ProPresentClient::ProPresentClient(QObject *parent) : QObject(parent) {
  network_manager = new QNetworkAccessManager(this);

  // Configurar timer
  polling_timer.setTimerType(Qt::PreciseTimer);
  QObject::connect(&polling_timer, &QTimer::timeout, this,
                   &ProPresentClient::check_view);
}

ProPresentClient::~ProPresentClient() { disconnect(); }

void ProPresentClient::connect(const QString &url) {
  base_url = url;
  if (base_url.endsWith("/")) {
    base_url.chop(1);
  }

  polling_timer.setInterval(polling_interval_ms);
  polling_timer.start();
  connected = true;

  blog(LOG_INFO, "[Auto Hide] Conectando ao ProPresent em: %s",
       base_url.toUtf8().constData());

  // Primeira verificação imediata
  check_view();
}

void ProPresentClient::disconnect() {
  polling_timer.stop();
  connected = false;
  verse_was_visible = false;
  blog(LOG_INFO, "[Auto Hide] Desconectado do ProPresent");
}

bool ProPresentClient::is_connected() { return connected; }

void ProPresentClient::set_polling_interval(int ms) {
  polling_interval_ms = ms;
  if (connected) {
    polling_timer.setInterval(ms);
  }
}

void ProPresentClient::set_disable_in_music(bool disable) {
    disable_in_music = disable; // Mantido para consistência da Interface de Configuração se for adicionar grupos futuramente.
}

void ProPresentClient::check_view() {
  if (!connected)
    return;

  QString url = base_url + "/v1/presentation/active";

  QNetworkRequest request(url);
  request.setTransferTimeout(2000); // 2 segundos timeout
  request.setHeader(QNetworkRequest::UserAgentHeader, "OBS Auto Hide Plugin");

  QNetworkReply *reply = network_manager->get(request);

  QObject::connect(reply, &QNetworkReply::finished, [this, reply, url]() {
    if (reply->error() != QNetworkReply::NoError) {
      if (polling_timer.isActive()) {
        blog(LOG_WARNING, "[Auto Hide] Erro de conexão com ProPresent: %s",
             reply->errorString().toUtf8().constData());
      }
    } else {
      QByteArray data = reply->readAll();
      QString json_str = QString::fromUtf8(data);

      bool verse_visible = detect_verse(json_str);

      if (verse_visible != verse_was_visible) {
        verse_was_visible = verse_visible;

        blog(LOG_INFO, "[Auto Hide] ProPresent Estado mudou: Apresentação ativa %s",
             verse_visible ? "SIM" : "NÃO");

        if (on_verse_changed) {
          on_verse_changed(verse_visible);
        }
      }
    }

    reply->deleteLater();
  });
}

bool ProPresentClient::detect_verse(const QString &raw_data) {
  QJsonDocument doc = QJsonDocument::fromJson(raw_data.toUtf8());

  if (doc.isNull() || !doc.isObject()) {
      return false;
  }

  QJsonObject root = doc.object();

  if (root.contains("presentation")) {
      QJsonValue presentationVal = root.value("presentation");
      
      if (!presentationVal.isNull()) {
          // TODO: Se quisermos ignorar músicas, provavelmente verificaríamos se há algo nos groups/slides
          // Mas pela spec solicitada, null = esconde letras, não null = mostra letras.
          return true; 
      }
  }

  return false;
}

void ProPresentClient::log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  blog(LOG_INFO, format, args);
  va_end(args);
}
