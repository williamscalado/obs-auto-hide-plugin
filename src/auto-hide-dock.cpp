#include "auto-hide-dock.hpp"
#include "settings-dialog.hpp"
#include <QDateTime>
#include <QMessageBox>
#include <obs-module.h>
#include <util/bmem.h>

AutoHideDockWidget::AutoHideDockWidget(PluginConfig &config,
                                       HolyricsClient *client,
                                       SceneController *controller,
                                       QWidget *parent)
    : QWidget(parent), config(config), holyrics_client(client),
      scene_controller(controller) {
  setup_ui();
  update_ui_state();
}

AutoHideDockWidget::~AutoHideDockWidget() {}

void AutoHideDockWidget::setup_ui() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  // Header Status
  connection_status_label = new QLabel("● Holyrics: Desconectado", this);
  main_layout->addWidget(connection_status_label);

  // Main Toggle Button
  toggle_button = new QPushButton("ATIVAR PLUGIN", this);
  toggle_button->setMinimumHeight(40);
  toggle_button->setCursor(Qt::PointingHandCursor);
  main_layout->addWidget(toggle_button);

  connect(toggle_button, &QPushButton::clicked, this,
          &AutoHideDockWidget::on_toggle_clicked);

  // Info Area
  status_label = new QLabel("Status: ⚪ Desativado", this);
  main_layout->addWidget(status_label);

  last_event_label = new QLabel("", this);
  main_layout->addWidget(last_event_label);

  // Sources Info
  sources_group = new QGroupBox("Escondendo quando versículo:", this);
  QVBoxLayout *sources_layout = new QVBoxLayout(sources_group);
  sources_list_label = new QLabel("", this);
  sources_list_label->setWordWrap(true);
  sources_layout->addWidget(sources_list_label);
  sources_group->setVisible(false);
  main_layout->addWidget(sources_group);

  main_layout->addStretch();

  // Settings Button
  settings_button = new QPushButton("⚙️ Configurar", this);
  main_layout->addWidget(settings_button);

  connect(settings_button, &QPushButton::clicked, this,
          &AutoHideDockWidget::open_settings);

  // Credits
  QLabel *credits_label = new QLabel("Desenvolvido por: Williams Calado", this);
  credits_label->setAlignment(Qt::AlignCenter);
  credits_label->setMinimumHeight(20); // Forçar altura mínima
  credits_label->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 10px; font-weight: bold; }");
  main_layout->addWidget(credits_label);
}

void AutoHideDockWidget::update_ui_state() {
  if (plugin_active) {
    // Estilo ATIVO (Vermelho para parar)
    toggle_button->setText("DESATIVAR PLUGIN");
    toggle_button->setStyleSheet("QPushButton {"
                                 "  background-color: #f44336;"
                                 "  color: white;"
                                 "  font-weight: bold;"
                                 "  font-size: 14px;"
                                 "  border-radius: 5px;"
                                 "}"
                                 "QPushButton:hover {"
                                 "  background-color: #da190b;"
                                 "}");
    status_label->setText("Status: ✅ Ativo - Monitorando");
    sources_group->setVisible(true);
    update_sources_list();
    connection_status_label->setText("🟢 Holyrics: Conectado");
  } else {
    // Estilo INATIVO (Verde para iniciar)
    toggle_button->setText("ATIVAR PLUGIN");
    toggle_button->setStyleSheet("QPushButton {"
                                 "  background-color: #4CAF50;"
                                 "  color: white;"
                                 "  font-weight: bold;"
                                 "  font-size: 14px;"
                                 "  border-radius: 5px;"
                                 "}"
                                 "QPushButton:hover {"
                                 "  background-color: #45a049;"
                                 "}");
    status_label->setText("Status: ⚪ Desativado");
    sources_group->setVisible(false);
    last_event_label->setText("");
    connection_status_label->setText("● Holyrics: Desconectado");
  }
}

void AutoHideDockWidget::update_sources_list() {
  if (config.sources_to_hide.isEmpty()) {
    sources_list_label->setText("⚠️ Nenhuma fonte configurada!");
  } else {
    QString text = "";
    for (const QString &source : config.sources_to_hide) {
      text += "• " + source + "\n";
    }
    sources_list_label->setText(text);
  }
}

void AutoHideDockWidget::on_toggle_clicked() { set_active(!plugin_active); }

void AutoHideDockWidget::set_active(bool active) {
  if (active) {
    // Validações
    if (config.sources_to_hide.isEmpty()) {
      QMessageBox::warning(this, "Atenção",
                           "Configure as fontes para esconder primeiro!");
      open_settings();
      return;
    }

    plugin_active = true;
    holyrics_client->connect(config.holyrics_url);
  } else {
    plugin_active = false;
    holyrics_client->disconnect();
    scene_controller->restore_previous_state();
  }
  update_ui_state();
}

void AutoHideDockWidget::open_settings() {
  SettingsDialog dialog(config, scene_controller, this);
  if (dialog.exec() == QDialog::Accepted) {
    // Salvar configurações imediatamente
    char *config_path = obs_module_config_path("config.json");
    if (config_path) {
      config.save_to_file(QString(config_path));
      bfree(config_path);
    }

    // Se estava ativo, reinicia conexão para aplicar nova URL/intervalo
    if (plugin_active) {
      holyrics_client->disconnect();
      holyrics_client->set_polling_interval(config.polling_interval_ms);
      holyrics_client->set_disable_in_music(config.disable_in_music);
      holyrics_client->connect(config.holyrics_url);

      scene_controller->set_action_delay(config.action_delay_ms);
      update_sources_list();
    } else {
        // Mesmo se não estiver ativo, atualiza a config para quando ativar
        holyrics_client->set_disable_in_music(config.disable_in_music);
    }
  }
}

void AutoHideDockWidget::update_last_event(bool verse_visible) {
  QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
  if (verse_visible) {
    last_event_label->setText("Último: " + time + " - 📖 Versículo ativo");
  } else {
    last_event_label->setText("Último: " + time + " - ✓ Normal");
  }
}

void AutoHideDockWidget::update_connection_status(bool connected) {
  if (connected) {
    connection_status_label->setText("🟢 Holyrics: Conectado");
  } else {
    connection_status_label->setText("🔴 Holyrics: Sem conexão");
  }
}
