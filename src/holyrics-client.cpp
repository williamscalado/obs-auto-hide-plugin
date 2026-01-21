#include "holyrics-client.hpp"
#include <obs-module.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

HolyricsClient::HolyricsClient(QObject *parent) : QObject(parent) {
  network_manager = new QNetworkAccessManager(this);

  // Configurar timer
  polling_timer.setTimerType(Qt::PreciseTimer);
  QObject::connect(&polling_timer, &QTimer::timeout, this,
                   &HolyricsClient::check_view);
}

HolyricsClient::~HolyricsClient() { disconnect(); }

void HolyricsClient::connect(const QString &url) {
  base_url = url;
  if (base_url.endsWith("/")) {
    base_url.chop(1);
  }

  polling_timer.setInterval(polling_interval_ms);
  polling_timer.start();
  connected = true;

  blog(LOG_INFO, "[Auto Hide] Conectando ao Holyrics em: %s",
       base_url.toUtf8().constData());

  // Primeira verificação imediata
  check_view();
}

void HolyricsClient::disconnect() {
  polling_timer.stop();
  connected = false;
  verse_was_visible = false;
  blog(LOG_INFO, "[Auto Hide] Desconectado do Holyrics");
}

bool HolyricsClient::is_connected() { return connected; }

void HolyricsClient::set_polling_interval(int ms) {
  polling_interval_ms = ms;
  if (connected) {
    polling_timer.setInterval(ms);
  }
}

void HolyricsClient::set_disable_in_music(bool disable) {
    disable_in_music = disable;
}

void HolyricsClient::check_view() {
  if (!connected)
    return;

  QString url = base_url + "/view/text.json";

  QNetworkRequest request(url);
  request.setTransferTimeout(2000); // 2 segundos timeout
  request.setHeader(QNetworkRequest::UserAgentHeader, "OBS Auto Hide Plugin");

  QNetworkReply *reply = network_manager->get(request);

  // Usar lambda para capturar contexto com segurança
  QObject::connect(reply, &QNetworkReply::finished, [this, reply, url]() {
    if (reply->error() != QNetworkReply::NoError) {
      // Apenas logar aviso periodicamente seria ideal para não floodar,
      // mas por enquanto logamos erro
      if (polling_timer.isActive()) {
        blog(LOG_WARNING, "[Auto Hide] Erro de conexão: %s",
             reply->errorString().toUtf8().constData());
      }
    } else {
      QByteArray data = reply->readAll();
      QString json_str = QString::fromUtf8(data);

      bool verse_visible = detect_verse(json_str);

      // Estado mudou?
      if (verse_visible != verse_was_visible) {
        verse_was_visible = verse_visible;

        blog(LOG_INFO, "[Auto Hide] Estado mudou: Versículo %s (Type detectado via JSON)",
             verse_visible ? "VISÍVEL" : "OCULTO");

        // Notificar callback
        if (on_verse_changed) {
          on_verse_changed(verse_visible);
        }
      }
    }

    reply->deleteLater();
  });
}

void HolyricsClient::on_network_reply_finished() {
  // Implementado via lambda no check_view
}

bool HolyricsClient::detect_verse(const QString &raw_data) {

  QJsonDocument doc = QJsonDocument::fromJson(raw_data.toUtf8());

  if (doc.isNull()) {

      blog(LOG_WARNING, "[Auto Hide DEBUG] JSON inválido ou vazio.");

      return false;

  }

  if (!doc.isObject()) {

      blog(LOG_WARNING, "[Auto Hide DEBUG] JSON não é um objeto raiz.");

      return false;

  }



  QJsonObject root = doc.object();



  // Logar o JSON inteiro (limitado para não explodir o log)

  // blog(LOG_INFO, "[Auto Hide DEBUG] JSON Recebido: %s", raw_data.left(500).toUtf8().constData());



  if (root.contains("map")) {

      QJsonObject map = root.value("map").toObject();

            if (map.contains("type")) {

                QString type = map.value("type").toString();

                blog(LOG_INFO, "[Auto Hide DEBUG] Tipo detectado: '%s'", type.toUtf8().constData());

                if (type.compare("MUSIC", Qt::CaseInsensitive) == 0) {
                    if (on_deactivation_requested && disable_in_music) {
                        on_deactivation_requested();
                        return true;
                    }
                    return true;
                }
                return type.compare("BIBLE", Qt::CaseInsensitive) == 0;

            } else {

          blog(LOG_WARNING, "[Auto Hide DEBUG] Objeto 'map' não contém chave 'type'.");

      }

  } else {

      blog(LOG_WARNING, "[Auto Hide DEBUG] JSON não contém chave 'map'.");

  }



  return false;

}

void HolyricsClient::log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  blog(LOG_INFO, format, args);
  va_end(args);
}
