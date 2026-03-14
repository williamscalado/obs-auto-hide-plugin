#include "auto-hide-dock.hpp"
#include "settings-dialog.hpp"
#include <QDateTime>
#include <QMessageBox>
#include <obs-module.h>
#include <util/bmem.h>

AutoHideDockWidget::AutoHideDockWidget(PluginConfig &config,
                                       IPresentationClient **client_ptr,
                                       SceneController *controller,
                                       QWidget *parent)
    : QWidget(parent), config(config), active_client_ptr(client_ptr),
      scene_controller(controller) {
  setup_ui();
  update_ui_state();
}

AutoHideDockWidget::~AutoHideDockWidget() {}

void AutoHideDockWidget::setup_ui() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  // Header Status
  QString softwareName = config.client_type;
  connection_status_label = new QLabel("● " + softwareName + ": Desconectado", this);
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

  // Client Info
  client_info_label = new QLabel("Cliente Atual: " + config.client_type, this);
  client_info_label->setAlignment(Qt::AlignCenter);
  client_info_label->setMinimumHeight(20); // Forçar altura mínima
  client_info_label->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 10px; font-weight: bold; }");
  main_layout->addWidget(client_info_label);
}

void AutoHideDockWidget::update_ui_state() {
  client_info_label->setText("Cliente Atual: " + config.client_type);

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
    connection_status_label->setText("🟢 " + config.client_type + ": Conectado");
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
    connection_status_label->setText("● " + config.client_type + ": Desconectado");
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

void AutoHideDockWidget::set_active(bool active, bool restore_state) {
  if (active) {
    // Validações
    if (config.sources_to_hide.isEmpty()) {
      QMessageBox::warning(this, "Atenção",
                           "Configure as fontes para esconder primeiro!");
      open_settings();
      return;
    }

    plugin_active = true;
    if (*active_client_ptr) {
        (*active_client_ptr)->connect(config.holyrics_url);
    }
  } else {
    plugin_active = false;
    if (*active_client_ptr) {
        (*active_client_ptr)->disconnect();
    }
    if (restore_state) {
        scene_controller->restore_previous_state();
    }
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
    // Notificar o plugin principal para recriar o cliente se necessário
    if (on_settings_changed) {
        on_settings_changed();
    }
    
    // Atualizar a UI com os nomes e status mais recentes
    update_ui_state();
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
    connection_status_label->setText("🟢 " + config.client_type + ": Conectado");
  } else {
    connection_status_label->setText("🔴 " + config.client_type + ": Sem conexão");
  }
}
