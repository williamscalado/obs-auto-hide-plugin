#pragma once

#include "presentation-client.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class ProPresentClient : public QObject, public IPresentationClient {
  Q_OBJECT

public:
  explicit ProPresentClient(QObject *parent = nullptr);
  ~ProPresentClient() override;

  void connect(const QString &url) override;
  void disconnect() override;
  bool is_connected() override;

  // Configuração
  void set_polling_interval(int ms);
  // Nota: Deixado compatível com HolyricsClient caso seja útil futuramente
  void set_disable_in_music(bool disable);

private slots:
  void check_view();

private:
  QNetworkAccessManager *network_manager;
  QTimer polling_timer;
  QString base_url;
  bool connected = false;
  bool verse_was_visible = false;
  int polling_interval_ms = 1000;
  bool disable_in_music = false;

  bool detect_verse(const QString &json_str);

  void log(const char *format, ...);
};
