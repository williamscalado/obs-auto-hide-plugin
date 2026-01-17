#pragma once

#include "presentation-client.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class HolyricsClient : public QObject, public IPresentationClient {
  Q_OBJECT

public:
  explicit HolyricsClient(QObject *parent = nullptr);
  ~HolyricsClient() override;

  void connect(const QString &url) override;
  void disconnect() override;
  bool is_connected() override;

  // Configuração
  void set_polling_interval(int ms);
  void set_disable_in_music(bool disable);

private slots:
  void check_view();
  void on_network_reply_finished();

private:
  QNetworkAccessManager *network_manager;
  QTimer polling_timer;
  QString base_url;
  bool connected = false;
  bool verse_was_visible = false;
  int polling_interval_ms = 1000;
  bool disable_in_music = false; // Novo: Configuração para música

  bool detect_verse(const QString &html);

  // Logging helper
  void log(const char *format, ...);
};
